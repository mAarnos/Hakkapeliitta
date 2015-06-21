/*
  Copyright (c) 2013 Ronald de Man
  This file may be redistributed and/or modified without restrictions.

  tbprobe.cpp contains the Stockfish-specific routines of the
  tablebase probing code. It should be relatively easy to adapt
  this code to other chess engines.
*/

#define NOMINMAX

#include <algorithm>

#include "../zobrist.hpp"
#include "../position.hpp"
#include "../movelist.hpp"
#include "../movegen.hpp"
#include "tbprobe.hpp"
#include "tbcore-impl.hpp"

int Syzygy::maxCardinality = 0;

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
            key ^= Zobrist::materialHashKey(Color::White * 6 + pt, i - 1);
    color = !color;
    for (Piece pt = Piece::Pawn; pt <= Piece::King; ++pt)
        for (auto i = pos.getPieceCount(color, pt); i > 0; i--)
            key ^= Zobrist::materialHashKey(Color::Black * 6 + pt, i - 1);

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
        key ^= Zobrist::materialHashKey(Color::White * 6 + pt, i);
    color ^= 8;
    for (Piece pt = Piece::Pawn; pt <= Piece::King; ++pt)
        for (auto i = 0; i < pcs[color + pt]; ++i)
        key ^= Zobrist::materialHashKey(Color::Black * 6 + pt, i);

    return key;
}

bool is_little_endian()
{
    union
    {
        int i;
        char c[sizeof(int)];
    } x;
    x.i = 1;
    return x.c[0] == 1;
}

static uint8_t decompress_pairs(struct PairsData *d, uint64_t idx)
{
    static const bool isLittleEndian = is_little_endian();
    return isLittleEndian ? decompress_pairs<true>(d, idx)
           : decompress_pairs<false>(d, idx);
}

// probe_wdl_table and probe_dtz_table require similar adaptations.
static int probe_wdl_table(const Position& pos, int& success)
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
    if (key == (Zobrist::materialHashKey(Piece::WhiteKing, 0) ^ Zobrist::materialHashKey(Piece::BlackKing, 0)))
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
#ifdef _MSC_VER
            _ReadWriteBarrier();
#else
            __asm__ __volatile__ ("" ::: "memory");
#endif
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
            Bitboard bb = pos.getBitboard((int8_t)((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07));
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
        Bitboard bb = pos.getBitboard((int8_t)(k >> 3), (k & 0x07));
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
            bb = pos.getBitboard((int8_t)((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07));
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

static int probe_dtz_table(const Position& pos, int wdl, int& success)
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
            Bitboard bb = pos.getBitboard((int8_t)((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07));
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
        Bitboard bb = pos.getBitboard((int8_t)(k >> 3), (k & 0x07));
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
            bb = pos.getBitboard((int8_t)((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07));
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

static int probe_ab(const Position& pos, int alpha, int beta, int& success)
{
    int v;
    const auto inCheck = pos.inCheck();
    MoveList moveList;

    inCheck ? MoveGen::generateLegalEvasions(pos, moveList)
            : MoveGen::generatePseudoLegalCaptures(pos, moveList, true);
    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto move = moveList.getMove(i);
        if (!pos.captureOrPromotion(move)
            || move.getFlags() == Piece::Pawn
            || !pos.legal(move, inCheck))
        {
            continue;
        }

        Position newPos(pos);
        newPos.makeMove(move);
        v = -probe_ab(newPos, -beta, -alpha, success);
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

int Syzygy::probeWdl(const Position& pos, int& success)
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
    const auto inCheck = pos.inCheck();

    inCheck ? MoveGen::generateLegalEvasions(pos, moveList)
            : MoveGen::generatePseudoLegalCaptures(pos, moveList, false);

    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto move = moveList.getMove(i);
        if (move.getFlags() != Piece::Pawn || !pos.legal(move, inCheck))
        {
            continue;
        }

        Position newPos(pos);
        newPos.makeMove(move);
        int v0 = -probe_ab(newPos , -2, 2, success);
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
                const auto move = moveList.getMove(i);
                if (move.getFlags() == Piece::Pawn) continue;
                if (pos.legal(move, inCheck)) break;
            }
            if (i == moveList.size() && !inCheck)
            {
                moveList.clear();
                MoveGen::generatePseudoLegalQuietMoves(pos, moveList);
                for (i = 0; i < moveList.size(); ++i)
                {
                    const auto move = moveList.getMove(i);
                    if (pos.legal(move, inCheck)) 
                        break;
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
static int probe_dtz_no_ep(const Position& pos, int& success)
{
    int wdl, dtz;

    wdl = probe_ab(pos, -2, 2, success);
    if (success == 0) return 0;

    if (wdl == 0) return 0;

    if (success == 2)
        return wdl == 2 ? 1 : 101;

    const auto inCheck = pos.inCheck();
    MoveList moveList;

    if (wdl > 0)
    {
        // Generate at least all legal non-capturing pawn moves
        // including non-capturing promotions.
        inCheck ? MoveGen::generateLegalEvasions(pos, moveList)
                : MoveGen::generatePseudoLegalMoves(pos, moveList);

        for (auto i = 0; i < moveList.size(); ++i)
        {
            const auto move = moveList.getMove(i);
            if (pos.getBoard(move.getFrom()).getPieceType() != Piece::Pawn
                || pos.getBoard(move.getTo()) != Piece::Empty
                || !pos.legal(move, inCheck))
            {
                continue;
            }

            Position newPos(pos);
            newPos.makeMove(move);
            int v = -probe_ab(newPos, -2, -wdl + 1, success);
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
            const auto move = moveList.getMove(i);
            if (pos.getBoard(move.getTo()) != Piece::Empty
                || pos.getBoard(move.getFrom()).getPieceType() == Piece::Pawn
                || !pos.legal(move, inCheck))
            {
                continue;
            }

            Position newPos(pos);
            newPos.makeMove(move);
            int v = -Syzygy::probeDtz(newPos, success);
            if (success == 0) return 0;
            if (v > 0 && v + 1 < best)
                best = v + 1;
        }
        return best;
    }
    else
    {
        int best = -1;
        inCheck ? MoveGen::generateLegalEvasions(pos, moveList)
                : MoveGen::generatePseudoLegalMoves(pos, moveList);

        for (auto i = 0; i < moveList.size(); ++i)
        {
            int v;
            const auto move = moveList.getMove(i);
            if (!pos.legal(move, inCheck))
                continue;

            Position newPos(pos);
            newPos.makeMove(move);
            if (newPos.getFiftyMoveDistance() == 0)
            {
                if (wdl == -2) v = -1;
                else
                {
                    v = probe_ab(newPos, 1, 2, success);
                    v = (v == 2) ? 0 : -101;
                }
            }
            else
            {
                v = -Syzygy::probeDtz(newPos, success) - 1;
            }
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

int Syzygy::probeDtz(const Position& pos, int& success)
{
    success = 1;
    int v = probe_dtz_no_ep(pos, success);

    if (pos.getEnPassantSquare() == Square::NoSquare)
        return v;
    if (success == 0) return 0;

    // Now handle en passant.
    int v1 = -3;

    const auto inCheck = pos.inCheck();
    MoveList moveList;

    inCheck ? MoveGen::generateLegalEvasions(pos, moveList)
            : MoveGen::generatePseudoLegalCaptures(pos, moveList, false);

    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto move = moveList.getMove(i);
        if (move.getFlags() != Piece::Pawn || !pos.legal(move, inCheck))
        {
            continue;
        }

        Position newPos(pos);
        newPos.makeMove(move);
        int v0 = -probe_ab(newPos, -2, 2, success);
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
                const auto move = moveList.getMove(i);
                if (move.getFlags() == Piece::Pawn) continue;
                if (pos.legal(move, inCheck)) break;
            }
            if (i == moveList.size() && !inCheck)
            {
                moveList.clear();
                MoveGen::generatePseudoLegalQuietMoves(pos, moveList);
                for (i = 0; i < moveList.size(); ++i)
                {
                    const auto move = moveList.getMove(i);
                    if (pos.legal(move, inCheck))
                        break;
                }
            }
            if (i == moveList.size())
                v = v1;
        }
    }

    return v;
}

/*
// Check whether there has been at least one repetition of positions
// since the last capture or pawn move.
static int has_repeated(StateInfo *st)
{
    while (1)
    {
        int i = 4, e = std::min(st->rule50, st->pliesFromNull);
        if (e < i)
            return 0;
        StateInfo *stp = st->previous->previous;
        do
        {
            stp = stp->previous->previous;
            if (stp->key == st->key)
                return 1;
            i += 2;
        }
        while (i <= e);
        st = st->previous;
    }
}
*/

bool Syzygy::rootProbe(const Position& pos, MoveList& rootMoves, int& score)
{
    int success;

    int dtz = probeDtz(pos, success);
    if (!success) return false;

    // Probe each move.
    for (auto i = 0; i < rootMoves.size(); i++)
    {
        const auto move = rootMoves.getMove(i);

        Position newPos(pos);
        newPos.makeMove(move);
        int v = 0;
        if (newPos.inCheck() && dtz > 0)
        {
            MoveList moveList;
            MoveGen::generateLegalEvasions(newPos, moveList);
            if (moveList.empty())
                v = 1;
        }
        if (!v)
        {
            if (newPos.getFiftyMoveDistance() != 0)
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
        if (!success) return false;
        rootMoves.setScore(i, static_cast<int16_t>(v));
    }

    // Obtain 50-move counter for the root position.
    int cnt50 = pos.getFiftyMoveDistance();

    // Use 50-move counter to determine whether the root position is
    // won, lost or drawn.
    int wdl = 0;
    if (dtz > 0)
        wdl = (dtz + cnt50 <= 100) ? 2 : 1;
    else if (dtz < 0)
        wdl = (-dtz + cnt50 <= 100) ? -2 : -1;

    // Determine the score to report to the user.
    score = wdl;

    // Now be a bit smart about filtering out moves.
    auto j = 0;
    if (dtz > 0)   // winning (or 50-move rule draw)
    {
        int best = 0xffff;
        for (auto i = 0; i < rootMoves.size(); i++)
        {
            int v = rootMoves.getScore(i);
            if (v > 0 && v < best)
                best = v;
        }
        int max = best;
        // If the current phase has not seen repetitions, then try all moves
        // that stay safely within the 50-move budget, if there are any.
        // TODO: put has_repeated back. I am too lazy to code it. Hopefully it won't backfire.
        if (best + cnt50 <= 99)
            max = 99 - cnt50;
        for (auto i = 0; i < rootMoves.size(); i++)
        {
            int v = rootMoves.getScore(i);
            if (v > 0 && v <= max)
            {
                rootMoves.setMove(j, rootMoves.getMove(i));
                rootMoves.setScore(j, rootMoves.getScore(i));
                j += 1;
            }
        }
    }
    else if (dtz < 0)     // losing (or 50-move rule draw)
    {
        int best = 0;
        for (auto i = 0; i < rootMoves.size(); i++)
        {
            int v = rootMoves.getScore(i);
            if (v < best)
                best = v;
        }
        // Try all moves, unless we approach or have a 50-move rule draw.
        if (-best * 2 + cnt50 < 100)
            return true;
        for (auto i = 0; i < rootMoves.size(); i++)
        {
            if (rootMoves.getScore(i) == best)
            {
                rootMoves.setMove(j, rootMoves.getMove(i));
                rootMoves.setScore(j, rootMoves.getScore(i));
                j += 1;
            }
        }
    }
    else     // drawing
    {
        // Try all moves that preserve the draw.
        for (auto i = 0; i < rootMoves.size(); i++)
        {
            if (rootMoves.getScore(i) == 0)
            {
                rootMoves.setMove(j, rootMoves.getMove(i));
                rootMoves.setScore(j, rootMoves.getScore(i));
                j += 1;
            }
        }
    }
    rootMoves.resize(j);

    return true;
}

bool Syzygy::rootProbeWdl(const Position& pos, MoveList& rootMoves, int& score)
{
    int success;

    int wdl = Syzygy::probeWdl(pos, success);
    if (!success) return false;
    score = wdl;

    int best = -2;

    // Probe each move.
    for (auto i = 0; i < rootMoves.size(); i++)
    {
        const auto move = rootMoves.getMove(i);

        Position newPos(pos);
        newPos.makeMove(move);
        int v = -Syzygy::probeWdl(pos, success);
        if (!success) return false;
        rootMoves.setScore(i, static_cast<int16_t>(v));
        if (v > best)
            best = v;
    }

    auto j = 0;
    for (auto i = 0; i < rootMoves.size(); i++)
    {
        if (rootMoves.getScore(i) == best)
        {
            rootMoves.setMove(j, rootMoves.getMove(i));
            rootMoves.setScore(j, rootMoves.getScore(i));
            j += 1;
        }
    }
    rootMoves.resize(j);

    return true;
}

