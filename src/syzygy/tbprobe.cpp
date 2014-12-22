/*
  Copyright (c) 2013 Ronald de Man
  This file may be redistributed and/or modified without restrictions.

  tbprobe.cpp contains the Stockfish-specific routines of the
  tablebase probing code. It should be relatively easy to adapt
  this code to other chess engines.
*/

// The probing code currently expects a little-endian architecture (e.g. x86).

// Define DECOMP64 when compiling for a 64-bit platform.
// 32-bit is only supported for 5-piece tables, because tables are mmap()ed
// into memory.
#ifdef IS_64BIT
#define DECOMP64
#endif

#include "..\color.hpp"
#include "..\piece.hpp"
#include "..\zobrist.hpp"
#include "..\position.hpp"
#include "..\movelist.hpp"
#include "..\movegen.hpp"
#include "..\search.hpp"
#include "tbprobe.hpp"
#include "tbcore.hpp"
#include "tbcore-impl.hpp"

int Syzygy::tbLargest = 0;

// Given a position with 6 or fewer pieces, produce a text string
// of the form KQPvKRP, where "KQP" represents the white pieces if
// mirror == 0 and the black pieces if mirror == 1.
static void prt_str(const Position& pos, char* str, int mirror)
{
    Color color = !mirror ? Color::White : Color::Black;

    for (Piece pt = Piece::King; pt >= Piece::Pawn; --pt)
        for (auto i = pos.getPieceCount(color, pt); i > 0; i--)
            *str++ = pchr[5 - pt];
    *str++ = 'v';
    color = !color;
    for (Piece pt = Piece::King; pt >= Piece::Pawn; --pt)
        for (auto i = pos.getPieceCount(color, pt); i > 0; i--)
            *str++ = pchr[5 - pt];
    *str++ = 0;
}

// Given a position, produce a 64-bit material signature key.
// If the engine supports such a key, it should equal the engine's key.
static uint64_t calc_key(const Position& pos, int mirror)
{
    uint64_t key = 0;
    Color color = !mirror ? Color::White : Color::Black;

    for (Piece pt = Piece::Pawn; pt <= Piece::King; ++pt)
        for (auto i = pos.getPieceCount(color, pt); i > 0; i--)
            key ^= Zobrist::materialHash[Color::White * 6 + pt][i - 1];
    color = !color;
    for (Piece pt = Piece::Pawn; pt <= Piece::King; ++pt)
        for (auto i = pos.getPieceCount(color, pt); i > 0; i--)
            key ^= Zobrist::materialHash[Color::Black * 6 + pt][i - 1];

    return key;
}

// Produce a 64-bit material key corresponding to the material combination
// defined by pcs[16], where pcs[1], ..., pcs[6] is the number of white
// pawns, ..., kings and pcs[9], ..., pcs[14] is the number of black
// pawns, ..., kings.
static uint64_t calc_key_from_pcs(int* pcs, int mirror)
{
    uint64_t key = 0;
    auto color = !mirror ? 1 : 9;

    for (Piece pt = Piece::Pawn; pt <= Piece::King; ++pt)
        for (auto i = 0; i < pcs[color + pt]; ++i)
            key ^= Zobrist::materialHash[Color::White * 6 + pt][i];
    color ^= 8;
    for (Piece pt = Piece::Pawn; pt <= Piece::King; ++pt)
        for (auto i = 0; i < pcs[color + pt]; ++i)
            key ^= Zobrist::materialHash[Color::Black * 6 + pt][i];

    return key;
}

// probe_wdl_table and probe_dtz_table require similar adaptations.
static int probe_wdl_table(Position& pos, int& success)
{
    struct TBEntry *ptr;
    struct TBHashEntry *ptr2;
    uint64_t idx;
    uint64_t key;
    int i;
    uint8_t res;
    int p[TBPIECES];

    // Obtain the position's material signature key.
    key = pos.getMaterialHashKey();

    // Test for KvK.
    if (key == (Zobrist::materialHash[Piece::WhiteKing][0] ^ Zobrist::materialHash[Piece::BlackKing][0]))
        return 0;

    ptr2 = TB_hash[key >> (64 - TBHASHBITS)];
    for (i = 0; i < HSHMAX; i++)
        if (ptr2[i].key == key) break;
    if (i == HSHMAX)
    {
        success = 0;
        return 0;
    }

    ptr = ptr2[i].ptr;
    if (!ptr->ready)
    {
        TB_mutex.lock();
        if (!ptr->ready)
        {
            char str[16];
            prt_str(pos, str, ptr->key != key);
            if (!init_table_wdl(ptr, str))
            {
                ptr2[i].key = 0ULL;
                success = 0;
                TB_mutex.unlock();
                return 0;
            }
            // Memory barrier to ensure ptr->ready = 1 is not reordered.
            // TODO: do something about this
            // __asm__ __volatile__ ("" ::: "memory");
            ptr->ready = 1;
        }
        TB_mutex.unlock();
    }

    int bside, mirror, cmirror;
    if (!ptr->symmetric)
    {
        if (key != ptr->key)
        {
            cmirror = 8;
            mirror = 0x38;
            bside = (pos.getSideToMove() == Color::White);
        }
        else
        {
            cmirror = mirror = 0;
            bside = !(pos.getSideToMove() == Color::White);
        }
    }
    else
    {
        cmirror = pos.getSideToMove() == Color::White ? 0 : 8;
        mirror = pos.getSideToMove() == Color::White ? 0 : 0x38;
        bside = 0;
    }

    // p[i] is to contain the square 0-63 (A1-H8) for a piece of type
    // pc[i] ^ cmirror, where 1 = white pawn, ..., 14 = black king.
    // Pieces of the same type are guaranteed to be consecutive.
    if (!ptr->has_pawns)
    {
        struct TBEntry_piece *entry = (struct TBEntry_piece *)ptr;
        uint8_t *pc = entry->pieces[bside];
        for (i = 0; i < entry->num;)
        {
            Bitboard bb = pos.getBitboard(!!((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07) - 1);
            do
            {
                p[i++] = Bitboards::popLsb(bb);
            }
            while (bb);
        }
        idx = encode_piece(entry, entry->norm[bside], p, entry->factor[bside]);
        res = decompress_pairs(entry->precomp[bside], idx);
    }
    else
    {
        struct TBEntry_pawn *entry = (struct TBEntry_pawn *)ptr;
        int k = entry->file[0].pieces[0][0] ^ cmirror;
        Bitboard bb = pos.getBitboard(!!(k >> 3), (k & 0x07) - 1);
        i = 0;
        do
        {
            p[i++] = Bitboards::popLsb(bb) ^ mirror;
        }
        while (bb);
        int f = pawn_file(entry, p);
        uint8_t *pc = entry->file[f].pieces[bside];
        for (; i < entry->num;)
        {
            bb = pos.getBitboard(!!((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07) - 1);
            do
            {
                p[i++] = Bitboards::popLsb(bb) ^ mirror;
            }
            while (bb);
        }
        idx = encode_pawn(entry, entry->file[f].norm[bside], p, entry->file[f].factor[bside]);
        res = decompress_pairs(entry->file[f].precomp[bside], idx);
    }

    return ((int)res) - 2;
}

static int probe_dtz_table(Position& pos, int wdl, int& success)
{
    struct TBEntry *ptr;
    uint64_t idx;
    int i, res;
    int p[TBPIECES];

    // Obtain the position's material signature key.
    uint64_t key = pos.getMaterialHashKey();

    if (DTZ_table[0].key1 != key && DTZ_table[0].key2 != key)
    {
        for (i = 1; i < DTZ_ENTRIES; i++)
            if (DTZ_table[i].key1 == key) break;
        if (i < DTZ_ENTRIES)
        {
            struct DTZTableEntry table_entry = DTZ_table[i];
            for (; i > 0; i--)
                DTZ_table[i] = DTZ_table[i - 1];
            DTZ_table[0] = table_entry;
        }
        else
        {
            struct TBHashEntry *ptr2 = TB_hash[key >> (64 - TBHASHBITS)];
            for (i = 0; i < HSHMAX; i++)
                if (ptr2[i].key == key) break;
            if (i == HSHMAX)
            {
                success = 0;
                return 0;
            }
            ptr = ptr2[i].ptr;
            char str[16];
            int mirror = (ptr->key != key);
            prt_str(pos, str, mirror);
            if (DTZ_table[DTZ_ENTRIES - 1].entry)
                free_dtz_entry(DTZ_table[DTZ_ENTRIES-1].entry);
            for (i = DTZ_ENTRIES - 1; i > 0; i--)
                DTZ_table[i] = DTZ_table[i - 1];
            load_dtz_table(str, calc_key(pos, mirror), calc_key(pos, !mirror));
        }
    }

    ptr = DTZ_table[0].entry;
    if (!ptr)
    {
        success = 0;
        return 0;
    }

    int bside, mirror, cmirror;
    if (!ptr->symmetric)
    {
        if (key != ptr->key)
        {
            cmirror = 8;
            mirror = 0x38;
            bside = (pos.getSideToMove() == Color::White);
        }
        else
        {
            cmirror = mirror = 0;
            bside = !(pos.getSideToMove() == Color::White);
        }
    }
    else
    {
        cmirror = pos.getSideToMove() == Color::White ? 0 : 8;
        mirror = pos.getSideToMove() == Color::White ? 0 : 0x38;
        bside = 0;
    }

    if (!ptr->has_pawns)
    {
        struct DTZEntry_piece *entry = (struct DTZEntry_piece *)ptr;
        if ((entry->flags & 1) != bside && !entry->symmetric)
        {
            success = -1;
            return 0;
        }
        uint8_t *pc = entry->pieces;
        for (i = 0; i < entry->num;)
        {
            Bitboard bb = pos.getBitboard(!!((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07) - 1);
            do
            {
                p[i++] = Bitboards::popLsb(bb);
            }
            while (bb);
        }
        idx = encode_piece((struct TBEntry_piece *)entry, entry->norm, p, entry->factor);
        res = decompress_pairs(entry->precomp, idx);

        if (entry->flags & 2)
            res = entry->map[entry->map_idx[wdl_to_map[wdl + 2]] + res];

        if (!(entry->flags & pa_flags[wdl + 2]) || (wdl & 1))
            res *= 2;
    }
    else
    {
        struct DTZEntry_pawn *entry = (struct DTZEntry_pawn *)ptr;
        int k = entry->file[0].pieces[0] ^ cmirror;
        Bitboard bb = pos.getBitboard(!!(k >> 3), (k & 0x07) - 1);
        i = 0;
        do
        {
            p[i++] = Bitboards::popLsb(bb) ^ mirror;
        }
        while (bb);
        int f = pawn_file((struct TBEntry_pawn *)entry, p);
        if ((entry->flags[f] & 1) != bside)
        {
            success = -1;
            return 0;
        }
        uint8_t *pc = entry->file[f].pieces;
        for (; i < entry->num;)
        {
            bb = pos.getBitboard(!!((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07) - 1);
            do
            {
                p[i++] = Bitboards::popLsb(bb) ^ mirror;
            }
            while (bb);
        }
        idx = encode_pawn((struct TBEntry_pawn *)entry, entry->file[f].norm, p, entry->file[f].factor);
        res = decompress_pairs(entry->file[f].precomp, idx);

        if (entry->flags[f] & 2)
            res = entry->map[entry->map_idx[f][wdl_to_map[wdl + 2]] + res];

        if (!(entry->flags[f] & pa_flags[wdl + 2]) || (wdl & 1))
            res *= 2;
    }

    return res;
}


// Add underpromotion captures to list of captures.
static void add_underprom_caps(const Position& pos, MoveList& stack)
{
    for (auto i = 0; i < stack.size(); ++i)
    {
        const auto& move = stack[i];
        if (move.getPromotion() == Piece::Queen && pos.getBoard(move.getTo()) != Piece::Empty)
        {
            stack.push_back(Move(move.getFrom(), move.getTo(), Piece::Rook, 0));
            stack.push_back(Move(move.getFrom(), move.getTo(), Piece::Bishop, 0));
            stack.push_back(Move(move.getFrom(), move.getTo(), Piece::Knight, 0));
            break;
        }
    }
}

static int probe_ab(Position& pos, int alpha, int beta, int& success)
{
    int v;
    MoveList moveList;
    History history;

    // Generate (at least) all legal non-ep captures including (under)promotions.
    // It is OK to generate more, as long as they are filtered out below.
    if (!pos.inCheck())
    {
        MoveGen::generatePseudoLegalCaptureMoves(pos, moveList);
        // Since underpromotion captures are not included, we need to add them.
        add_underprom_caps(pos, moveList);
    }
    else
    {
        MoveGen::generateLegalEvasions(pos, moveList);
    }

    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto& move = moveList[i];
        if (pos.getBoard(move.getTo()) == Piece::Empty || move.getPromotion() == Piece::Pawn)
            continue;
        if (!pos.makeMove(move, history))
        {
            continue;
        }
        v = -probe_ab(pos, -beta, -alpha, success);
        pos.unmakeMove(move, history);
        if (success == 0) return 0;
        if (v > alpha)
        {
            if (v >= beta)
            {
                success = 2;
                return v;
            }
            alpha = v;
        }
    }

    v = probe_wdl_table(pos, success);
    if (success == 0) return 0;
    if (alpha >= v)
    {
        success = 1 + (alpha > 0);
        return alpha;
    }
    else
    {
        success = 1;
        return v;
    }
}

int Syzygy::probeWdl(Position& pos, int& success)
{
    int v;

    success = 1;
    v = probe_ab(pos, -2, 2, success);

    // If en passant is not possible, we are done.
    if (pos.getEnPassantSquare() == Square::NoSquare)
        return v;
    if (!success) return 0;

    // Now handle en passant.
    int v1 = -3;
    // Generate (at least) all legal en passant captures.
    MoveList moveList;
    History history;

    if (!pos.inCheck())
        MoveGen::generatePseudoLegalCaptureMoves(pos, moveList);
    else
        MoveGen::generateLegalEvasions(pos, moveList);

    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto& move = moveList[i];
        if (move.getPromotion() != Piece::Pawn)
            continue;
        if (!pos.makeMove(move, history))
        {
            continue;
        }
        int v0 = -probe_ab(pos, -2, 2, success);
        pos.unmakeMove(move, history);
        if (success == 0) return 0;
        if (v0 > v1) v1 = v0;
    }
    if (v1 > -3)
    {
        if (v1 >= v) v = v1;
        else if (v == 0)
        {
            int i;
            // Check whether there is at least one legal non-ep move.
            for (i = 0; i < moveList.size(); ++i)
            {
                const auto& move = moveList[i];
                if (move.getPromotion() == Piece::Pawn) continue;
                if (pos.makeMove(move, history))
                {
                    pos.unmakeMove(move, history);
                    break;
                }
            }
            if (i == moveList.size() && !pos.inCheck())
            {
                moveList.clear();
                MoveGen::generatePseudoLegalMoves(pos, moveList);
                for (i = 0; i < moveList.size(); ++i)
                {
                    const auto& move = moveList[i];
                    if (pos.getBoard(move.getTo()) != Piece::Empty || (move.getPromotion() != Piece::Empty && move.getPromotion() != Piece::King))
                    {
                        continue;
                    }
                    if (pos.makeMove(move, history))
                    {
                        pos.unmakeMove(move, history);
                        break;
                    }
                }
            }
            // If not, then we are forced to play the losing ep capture.
            if (i == moveList.size())
                v = v1;
        }
    }

    return v;
}

// This routine treats a position with en passant captures as one without.
static int probe_dtz_no_ep(Position& pos, int& success)
{
    int wdl, dtz;

    wdl = probe_ab(pos, -2, 2, success);
    if (success == 0) return 0;

    if (wdl == 0) return 0;

    if (success == 2)
        return wdl == 2 ? 1 : 101;

    MoveList moveList;
    History history;

    if (wdl > 0)
    {
        // Generate at least all legal non-capturing pawn moves
        // including non-capturing promotions.
        if (!pos.inCheck())
            MoveGen::generatePseudoLegalMoves(pos, moveList);
        else
            MoveGen::generateLegalEvasions(pos, moveList);

        for (auto i = 0; i < moveList.size(); ++i)
        {
            const auto& move = moveList[i];
            if ((pos.getBoard(move.getFrom()) % 6) != Piece::Pawn || pos.getBoard(move.getTo()) != Piece::Empty)
                continue;
            if (!pos.makeMove(move, history))
            {
                continue;
            }
            int v = -probe_ab(pos, -2, -wdl + 1, success);
            pos.unmakeMove(move, history);
            if (success == 0) return 0;
            if (v == wdl)
                return v == 2 ? 1 : 101;
        }
    }

    dtz = 1 + probe_dtz_table(pos, wdl, success);
    if (success >= 0)
    {
        if (wdl & 1) dtz += 100;
        return wdl >= 0 ? dtz : -dtz;
    }

    if (wdl > 0)
    {
        int best = 0xffff;
        for (auto i = 0; i < moveList.size(); ++i)
        {
            const auto& move = moveList[i];
            if (pos.getBoard(move.getTo()) != Piece::Empty || (pos.getBoard(move.getFrom()) % 6) == Piece::Pawn)
                continue;
            if (!pos.makeMove(move, history))
            {
                continue;
            }
            int v = -Syzygy::probeDtz(pos, success);
            pos.unmakeMove(move, history);
            if (success == 0) return 0;
            if (v > 0 && v + 1 < best)
                best = v + 1;
        }
        return best;
    }
    else
    {
        int best = -1;
        if (!pos.inCheck())
            MoveGen::generatePseudoLegalMoves(pos, moveList);
        else
            MoveGen::generateLegalEvasions(pos, moveList);
        for (auto i = 0; i < moveList.size(); ++i)
        {
            int v;
            const auto& move = moveList[i];
            if (!pos.makeMove(move, history))
            {
                continue;
            }
            if (pos.getFiftyMoveDistance() == 0)
            {
                if (wdl == -2) v = -1;
                else
                {
                    v = probe_ab(pos, 1, 2, success);
                    v = (v == 2) ? 0 : -101;
                }
            }
            else
            {
                v = -Syzygy::probeDtz(pos, success) - 1;
            }
            pos.unmakeMove(move, history);
            if (success == 0) return 0;
            if (v < best)
                best = v;
        }
        return best;
    }
}

static int wdl_to_dtz[] =
{
    -1, -101, 0, 101, 1
};

int Syzygy::probeDtz(Position& pos, int& success)
{
    success = 1;
    int v = probe_dtz_no_ep(pos, success);

    if (pos.getEnPassantSquare() == Square::NoSquare)
        return v;
    if (success == 0) return 0;

    // Now handle en passant.
    int v1 = -3;

    MoveList moveList;
    History history;

    if (!pos.inCheck())
        MoveGen::generatePseudoLegalCaptureMoves(pos, moveList);
    else
        MoveGen::generateLegalEvasions(pos, moveList);

    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto& move = moveList[i];
        if (move.getPromotion() != Piece::Pawn)
            continue;
        if (!pos.makeMove(move, history))
        {
            continue;
        }
        int v0 = -probe_ab(pos, -2, 2, success);
        pos.unmakeMove(move, history);
        if (success == 0) return 0;
        if (v0 > v1) v1 = v0;
    }
    if (v1 > -3)
    {
        v1 = wdl_to_dtz[v1 + 2];
        if (v < -100)
        {
            if (v1 >= 0)
                v = v1;
        }
        else if (v < 0)
        {
            if (v1 >= 0 || v1 < 100)
                v = v1;
        }
        else if (v > 100)
        {
            if (v1 > 0)
                v = v1;
        }
        else if (v > 0)
        {
            if (v1 == 1)
                v = v1;
        }
        else if (v1 >= 0)
        {
            v = v1;
        }
        else
        {
            int i;
            for (i = 0; i < moveList.size(); ++i)
            {
                const auto& move = moveList[i];
                if (move.getPromotion() == Piece::Pawn) continue;
                if (pos.makeMove(move, history))
                {
                    pos.unmakeMove(move, history);
                    break;
                }
            }
            if (i == moveList.size() && !pos.inCheck())
            {
                moveList.clear();
                MoveGen::generatePseudoLegalMoves(pos, moveList);
                for (i = 0; i < moveList.size(); ++i)
                {
                    const auto& move = moveList[i];
                    if (pos.getBoard(move.getTo()) != Piece::Empty || (move.getPromotion() != Piece::Empty && move.getPromotion() != Piece::King))
                        continue;
                    if (pos.makeMove(move, history))
                    {
                        pos.unmakeMove(move, history);
                        break;
                    }
                }
            }
            if (i == moveList.size())
                v = v1;
        }
    }

    return v;
}

static int wdl_to_Value[5] =
{
    -mateScore + 200 + 1,
    -2,
    0,
    2,
    mateScore - 200 - 1
};


bool Syzygy::rootProbe(Position& pos, MoveList& rootMoveList, int& tbScore)
{
    int success;

    int dtz = probeDtz(pos, success);
    if (!success) return false;

    History history;

    // Probe each move.
    for (auto i = 0; i < rootMoveList.size(); ++i)
    {
        auto& move = rootMoveList[i];
        pos.makeMove(move, history); // Can't fail due to the rootMoveList only containing legal moves.
        int v = 0;
        if (pos.inCheck() && dtz > 0)
        {
            MoveList moveList;
            MoveGen::generateLegalEvasions(pos, moveList);
            if (moveList.size() == 0)
                v = 1;
        }
        if (!v)
        {
            if (pos.getFiftyMoveDistance() != 0)
            {
                v = -Syzygy::probeDtz(pos, success);
                if (v > 0) v++;
                else if (v < 0) v--;
            }
            else
            {
                v = -Syzygy::probeWdl(pos, success);
                v = wdl_to_dtz[v + 2];
            }
        }
        pos.unmakeMove(move, history);
        if (!success) return false;
        move.setScore(v);
    }

    auto cnt50 = pos.getFiftyMoveDistance();
    // Use 50-move counter to determine whether the root position is
    // won, lost or drawn.
    int wdl = 0;
    if (dtz > 0)
        wdl = (dtz + cnt50 <= 100) ? 2 : 1;
    else if (dtz < 0)
        wdl = (-dtz + cnt50 <= 100) ? -2 : -1;

    // Determine the score to report to the user.
    tbScore = wdl_to_Value[wdl + 2];
    // If the position is winning or losing, but too few moves left, adjust the
    // score to show how close it is to winning or losing.
    if (wdl == 1 && dtz <= 100)
        tbScore = 200 - dtz - cnt50;
    else if (wdl == -1 && dtz >= -100)
        tbScore = -200 + dtz - cnt50;

    // Now be a bit smart about filtering out moves.
    auto j = 0;
    if (dtz > 0)   // winning (or 50-move rule draw)
    {
        int best = 0xffff;
        for (auto i = 0; i < rootMoveList.size(); ++i)
        {
            int v = rootMoveList[i].getScore();
            if (v > 0 && v < best)
                best = v;
        }
        int max = best;
        // If the current phase has not seen repetitions, then try all moves
        // that stay safely within the 50-move budget, if there are any.
        // if (!has_repeated(st.previous) && best + cnt50 <= 99)
        // TODO: fix this
        if (best + cnt50 <= 99)
            max = 99 - cnt50;
        for (auto i = 0; i < rootMoveList.size(); ++i)
        {
            int v = rootMoveList[i].getScore();
            if (v > 0 && v <= max)
                rootMoveList[j++] = rootMoveList[i];
        }
    }
    else if (dtz < 0)     // losing (or 50-move rule draw)
    {
        int best = 0;
        for (auto i = 0; i < rootMoveList.size(); ++i)
        {
            int v = rootMoveList[i].getScore();
            if (v < best)
                best = v;
        }
        // Try all moves, unless we approach or have a 50-move rule draw.
        if (-best * 2 + cnt50 < 100)
            return true;
        for (auto i = 0; i < rootMoveList.size(); ++i)
        {
            if (rootMoveList[i].getScore() == best)
                rootMoveList[j++] = rootMoveList[i];
        }
    }
    else     // drawing
    {
        // Try all moves that preserve the draw.
        for (auto i = 0; i < rootMoveList.size(); ++i)
        {
            if (rootMoveList[i].getScore() == 0)
                rootMoveList[j++] = rootMoveList[i];
        }
    }
    rootMoveList.resize(j);

    return true;
}

bool Syzygy::rootProbeWdl(Position& pos, MoveList& rootMoveList, int& tbScore)
{
    int success;

    int wdl = Syzygy::probeWdl(pos, success);
    if (!success) return false;
    tbScore = wdl_to_Value[wdl + 2];

    History history;

    int best = -2;

    // Probe each move.
    for (auto i = 0; i < rootMoveList.size(); ++i)
    {
        auto& move = rootMoveList[i];
        pos.makeMove(move, history); // Can't fail due to the rootMoveList only containing legal moves.
        int v = -Syzygy::probeWdl(pos, success);
        pos.unmakeMove(move, history);
        if (!success) return false;
        move.setScore(v);
        if (v > best)
            best = v;
    }

    auto j = 0;
    for (auto i = 0; i < rootMoveList.size(); ++i)
    {
        if (rootMoveList[i].getScore() == best)
            rootMoveList[j++] = rootMoveList[i];
    }
    rootMoveList.resize(j);

    return true;
}



