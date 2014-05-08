/*
  Copyright (c) 2013 Ronald de Man
  This file may be redistributed and/or modified without restrictions.

  tbprobe.cpp contains the Stockfish-specific routines of the
  tablebase probing code. It should be relatively easy to adapt
  this code to other chess engines.
*/

// The probing code currently expects a little-endian architecture (e.g. x86).

// Define __WIN32__ when compiling for a windows platform.
#define __WIN32__
// Define IS_64BIT when compiling for a 64-bit platform.
// 32-bit is only supported for 5-piece tables, because tables are mmap()ed
// into memory.
#define IS_64BIT

#include "tbprobe.hpp"
#include "tbcore.hpp"
#include "eval.hpp"
#include "bitboard.hpp"
#include "movegen.hpp"

// Please forgive me.
#include "tbcore.cpp"

int TBLargest = 0;

// Given a position with 6 or fewer pieces, produce a text string
// of the form KQPvKRP, where "KQP" represents the white pieces if
// mirror == 0 and the black pieces if mirror == 1.
static void prt_str(Position& pos, char *str, int mirror)
{
    bool color;
    int i, pt;

    color = !mirror ? White : Black;
    for (pt = King; pt >= Pawn; --pt)
        for (i = popcnt(pos.getBitboard(color, pt)); i > 0; i--)
            *str++ = pchr[5 - pt];
    *str++ = 'v';
    color = !color;
    for (pt = King; pt >= Pawn; --pt)
        for (i = popcnt(pos.getBitboard(color, pt)); i > 0; i--)
            *str++ = pchr[5 - pt];
    *str++ = 0;
}

// Given a position, produce a 64-bit material signature key.
// If the engine supports such a key, it should equal the engine's key.
static uint64_t calc_key(Position& pos, int mirror)
{
    bool color;
    int i, pt;
    uint64_t key = 0;

    color = !mirror ? White : Black;
    for (pt = Pawn; pt <= King; ++pt)
        for (i = popcnt(pos.getBitboard(color, pt)); i > 0; i--)
            key ^= materialHash[White * 6 + pt][i - 1];
    color = !color;
    for (pt = Pawn; pt <= King; ++pt)
        for (i = popcnt(pos.getBitboard(color, pt)); i > 0; i--)
            key ^= materialHash[Black * 6 + pt][i - 1];

    return key;
}

// Produce a 64-bit material key corresponding to the material combination
// defined by pcs[16], where pcs[1], ..., pcs[6] is the number of white
// pawns, ..., kings and pcs[9], ..., pcs[14] is the number of black
// pawns, ..., kings.
static uint64_t calc_key_from_pcs(int *pcs, int mirror)
{
	int color;
	int i, pt;
	uint64_t key = 0;

	color = !mirror ? 1 : 9;
	for (pt = Pawn; pt <= King; ++pt)
	for (i = 0; i < pcs[color + pt]; i++)
		key ^= materialHash[White * 6 + pt][i];
	color ^= 8;
	for (pt = Pawn; pt <= King; ++pt)
	for (i = 0; i < pcs[color + pt]; i++)
		key ^= materialHash[Black * 6 + pt][i];

	return key;
}

// probe_wdl_table and probe_dtz_table require similar adaptations.
static int probe_wdl_table(Position& pos, int *success)
{
    struct TBEntry *ptr;
    struct TBHashEntry *ptr2;
    uint64_t idx;
    uint64_t key;
    int i;
    uint8_t res;
    int p[TBPIECES];

    // Obtain the position's material signature key.
    key = pos.getMaterialHash();

    // Test for KvK.
    if (key == (materialHash[WhiteKing][0] ^ materialHash[BlackKing][0]))
        return 0;

    ptr2 = TB_hash[key >> (64 - TBHASHBITS)];
    for (i = 0; i < HSHMAX; i++)
        if (ptr2[i].key == key) break;
    if (i == HSHMAX) {
        *success = 0;
        return 0;
    }

    ptr = ptr2[i].ptr;
    if (!ptr->ready) {
        LOCK(TB_mutex);
        if (!ptr->ready) {
            char str[16];
            prt_str(pos, str, ptr->key != key);
            if (!init_table_wdl(ptr, str)) {
                ptr2[i].key = 0ULL;
                *success = 0;
                UNLOCK(TB_mutex);
                return 0;
            }
            // Memory barrier to ensure ptr->ready = 1 is not reordered.
            // __asm__ __volatile__ ("" ::: "memory");
            ptr->ready = 1;
        }
        UNLOCK(TB_mutex);
    }

    int bside, mirror, cmirror;
    if (!ptr->symmetric) {
        if (key != ptr->key) {
            cmirror = 8;
            mirror = 0x38;
            bside = (pos.getSideToMove() == White);
        } else {
            cmirror = mirror = 0;
            bside = !(pos.getSideToMove() == White);
        }
    } else {
        cmirror = pos.getSideToMove() == White ? 0 : 8;
        mirror = pos.getSideToMove() == White ? 0 : 0x38;
        bside = 0;
    }

    // p[i] is to contain the square 0-63 (A1-H8) for a piece of type
    // pc[i] ^ cmirror, where 1 = white pawn, ..., 14 = black king.
    // Pieces of the same type are guaranteed to be consecutive.
    if (!ptr->has_pawns) {
        struct TBEntry_piece *entry = (struct TBEntry_piece *)ptr;
        uint8_t *pc = entry->pieces[bside];
        for (i = 0; i < entry->num;) {
            uint64_t bb = pos.getBitboard(!!((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07)-1);
            do {
                p[i++] = bitScanForward(bb);
                bb &= (bb - 1);
            } while (bb);
        }
        idx = encode_piece(entry, entry->norm[bside], p, entry->factor[bside]);
        res = decompress_pairs(entry->precomp[bside], idx);
    } else {
        struct TBEntry_pawn *entry = (struct TBEntry_pawn *)ptr;
        int k = entry->file[0].pieces[0][0] ^ cmirror;
        uint64_t bb = pos.getBitboard(!!(k >> 3), (k & 0x07)-1);
        i = 0;
        do {
            p[i++] = bitScanForward(bb) ^ mirror;
            bb &= (bb - 1);
        } while (bb);
        int f = pawn_file(entry, p);
        uint8_t *pc = entry->file[f].pieces[bside];
        for (; i < entry->num;) {
            // check this
            bb = pos.getBitboard(!!((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07)-1);
            do {
                p[i++] = bitScanForward(bb) ^ mirror;
                bb &= (bb - 1);
            } while (bb);
        }
        idx = encode_pawn(entry, entry->file[f].norm[bside], p, entry->file[f].factor[bside]);
        res = decompress_pairs(entry->file[f].precomp[bside], idx);
    }

    return ((int)res) - 2;
}

static int probe_dtz_table(Position& pos, int wdl, int *success)
{
    struct TBEntry *ptr;
    uint64_t idx;
    int i, res;
    int p[TBPIECES];

    // Obtain the position's material signature key.
    uint64_t key = pos.getMaterialHash();

    if (DTZ_table[0].key1 != key && DTZ_table[0].key2 != key) {
        for (i = 1; i < DTZ_ENTRIES; i++)
            if (DTZ_table[i].key1 == key) break;
        if (i < DTZ_ENTRIES) {
            struct DTZTableEntry table_entry = DTZ_table[i];
            for (; i > 0; i--)
                DTZ_table[i] = DTZ_table[i - 1];
            DTZ_table[0] = table_entry;
        } else {
            struct TBHashEntry *ptr2 = TB_hash[key >> (64 - TBHASHBITS)];
            for (i = 0; i < HSHMAX; i++)
                if (ptr2[i].key == key) break;
            if (i == HSHMAX) {
                *success = 0;
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
    if (!ptr) {
        *success = 0;
        return 0;
    }

    int bside, mirror, cmirror;
    if (!ptr->symmetric) {
        if (key != ptr->key) {
            cmirror = 8;
            mirror = 0x38;
            bside = (pos.getSideToMove() == White);
        } else {
            cmirror = mirror = 0;
            bside = !(pos.getSideToMove() == White);
        }
    } else {
        cmirror = pos.getSideToMove() == White ? 0 : 8;
        mirror = pos.getSideToMove() == White ? 0 : 0x38;
        bside = 0;
    }

    if (!ptr->has_pawns) {
        struct DTZEntry_piece *entry = (struct DTZEntry_piece *)ptr;
        if ((entry->flags & 1) != bside && !entry->symmetric) {
            *success = -1;
            return 0;
        }
        uint8_t *pc = entry->pieces;
        for (i = 0; i < entry->num;) {
            // check this
            uint64_t bb = pos.getBitboard(!!((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07)-1);
            do {
                p[i++] = bitScanForward(bb);
                bb &= (bb - 1);
            } while (bb);
        }
        idx = encode_piece((struct TBEntry_piece *)entry, entry->norm, p, entry->factor);
        res = decompress_pairs(entry->precomp, idx);

        if (entry->flags & 2)
            res = entry->map[entry->map_idx[wdl_to_map[wdl + 2]] + res];

        if (!(entry->flags & pa_flags[wdl + 2]) || (wdl & 1))
            res *= 2;
    } else {
        struct DTZEntry_pawn *entry = (struct DTZEntry_pawn *)ptr;
        int k = entry->file[0].pieces[0] ^ cmirror;
        // check this
        uint64_t bb = pos.getBitboard(!!(k >> 3), (k & 0x07)-1);
        i = 0;
        do {
            p[i++] = bitScanForward(bb) ^ mirror;
            bb &= (bb - 1);
        } while (bb);
        int f = pawn_file((struct TBEntry_pawn *)entry, p);
        if ((entry->flags[f] & 1) != bside) {
            *success = -1;
            return 0;
        }
        uint8_t *pc = entry->file[f].pieces;
        for (; i < entry->num;) {
            bb = pos.getBitboard(!!((pc[i] ^ cmirror) >> 3), (pc[i] & 0x07)-1);
            do {
                p[i++] = bitScanForward(bb) ^ mirror;
                bb &= (bb - 1);
            } while (bb);
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

static int probe_ab(Position& pos, int alpha, int beta, int *success)
{
    int v, generatedMoves;
    Move moveStack[256];

    // Generate (at least) all legal non-ep captures including (under)promotions.
    // It is OK to generate more, as long as they are filtered out below.
	generatedMoves = generateMoves(pos, moveStack);
    for (int i = 0; i < generatedMoves; i++) {
        if (pos.getPiece(moveStack[i].getTo()) == Empty || moveStack[i].getPromotion() == Pawn)
            continue;
        if (!(pos.makeMove(moveStack[i]))) {
            continue;
        }
        v = -probe_ab(pos, -beta, -alpha, success);
        pos.unmakeMove(moveStack[i]);
        if (*success == 0) return 0;
        if (v > alpha) {
            if (v >= beta) {
                *success = 2;
                return v;
            }
            alpha = v;
        }
    }

    v = probe_wdl_table(pos, success);
    if (*success == 0) return 0;
    if (alpha >= v) {
        *success = 1 + (alpha > 0);
        return alpha;
    } else {
        *success = 1;
        return v;
    }
}

// Probe the WDL table for a particular position.
// If *success != 0, the probe was successful.
// The return value is from the point of view of the side to move:
// -2 : loss
// -1 : loss, but draw under 50-move rule
//  0 : draw
//  1 : win, but draw under 50-move rule
//  2 : win
int probe_wdl(Position& pos, int *success)
{
    int v, generatedMoves;

    *success = 1;
    v = probe_ab(pos, -2, 2, success);

    // If en passant is not possible, we are done.
    if (pos.getEnPassantSquare() == NoSquare)
        return v;
    if (!(*success)) return 0;

    // Now handle en passant.
    int v1 = -3;
    // Generate (at least) all legal en passant captures.
    Move moveStack[256];
	generatedMoves = generateMoves(pos, moveStack);
    for (int i = 0; i < generatedMoves; i++) {
        if (moveStack[i].getPromotion() != Pawn)
            continue;
        if (!(pos.makeMove(moveStack[i]))) {
            continue;
        }
        int v0 = -probe_ab(pos, -2, 2, success);
        pos.unmakeMove(moveStack[i]);
        if (*success == 0) return 0;
        if (v0 > v1) v1 = v0;
    }
    if (v1 > -3) {
        if (v1 >= v) v = v1;
        else if (v == 0) {
            int i;
            // Check whether there is at least one legal non-ep move.
            for (i = 0; i < generatedMoves; i++) {
                if (moveStack[i].getPromotion() == Pawn) continue;
                if (pos.makeMove(moveStack[i])) {
                    pos.unmakeMove(moveStack[i]);
                    break;
                }
            }
            // If not, then we are forced to play the losing ep capture.
            if (i == generatedMoves)
                v = v1;
        }
    }

    return v;
}

// This routine treats a position with en passant captures as one without.
static int probe_dtz_no_ep(Position& pos, int *success)
{
    int wdl, dtz, generatedMoves;

    wdl = probe_ab(pos, -2, 2, success);
    if (*success == 0) return 0;

    if (wdl == 0) return 0;

    if (*success == 2)
        return wdl == 2 ? 1 : 101;

    Move moveStack[256];
	// Generate at least all legal non-capturing pawn moves
	// including non-capturing promotions.
	generatedMoves = generateMoves(pos, moveStack);
    if (wdl > 0) {
        for (int i = 0; i < generatedMoves; i++) {
			if (pos.getPieceType(moveStack[i].getFrom()) != Pawn || pos.getPiece(moveStack[i].getTo()) != Empty)
                continue;
            if (!(pos.makeMove(moveStack[i]))) {
                continue;
            }
            int v = -probe_ab(pos, -2, -wdl + 1, success);
            pos.unmakeMove(moveStack[i]);
            if (*success == 0) return 0;
            if (v == wdl)
                return v == 2 ? 1 : 101;
        }
    }

    dtz = 1 + probe_dtz_table(pos, wdl, success);
    if (*success >= 0) {
        if (wdl & 1) dtz += 100;
        return wdl >= 0 ? dtz : -dtz;
    }

    if (wdl > 0) {
        int best = 0xffff;
        for (int i = 0; i < generatedMoves; i++) {
            if (pos.getPiece(moveStack[i].getTo()) != Empty || pos.getPieceType(moveStack[i].getFrom()) == Pawn)
                continue;
            if (!(pos.makeMove(moveStack[i]))) {
                continue;
            }
            int v = -probe_dtz(pos, success);
            pos.unmakeMove(moveStack[i]);
            if (*success == 0) return 0;
            if (v > 0 && v + 1 < best)
                best = v + 1;
        }
        return best;
    } else {
        int best = -1;
        for (int i = 0; i < generatedMoves; i++) {
            int v;
            if (!(pos.makeMove(moveStack[i]))) {
                continue;
            }
            // Check this part.
            if (pos.getFiftyMoveDistance() == 0) {
                if (wdl == -2) v = -1;
                else {
                    v = probe_ab(pos, 1, 2, success);
                    v = (v == 2) ? 0 : -101;
                }
            } else {
                v = -probe_dtz(pos, success) - 1;
            }
            pos.unmakeMove(moveStack[i]);
            if (*success == 0) return 0;
            if (v < best)
                best = v;
        }
        return best;
    }
}

static int wdl_to_dtz[] = {
    -1, -101, 0, 101, 1
};

// Probe the DTZ table for a particular position.
// If *success != 0, the probe was successful.
// The return value is from the point of view of the side to move:
//         n < -100 : loss, but draw under 50-move rule
// -100 <= n < -1   : loss in n ply (assuming 50-move counter == 0)
//         0	    : draw
//     1 < n <= 100 : win in n ply (assuming 50-move counter == 0)
//   100 < n        : win, but draw under 50-move rule
//
// The return value n can be off by 1: a return value -n can mean a loss
// in n+1 ply and a return value +n can mean a win in n+1 ply. This
// cannot happen for tables with positions exactly on the "edge" of
// the 50-move rule.
//
// This implies that if dtz > 0 is returned, the position is certainly
// a win if dtz + 50-move-counter <= 99. Care must be taken that the engine
// picks moves that preserve dtz + 50-move-counter <= 99.
//
// If n = 100 immediately after a capture or pawn move, then the position
// is also certainly a win, and during the whole phase until the next
// capture or pawn move, the inequality to be preserved is
// dtz + 50-movecounter <= 100.
//
// In short, if a move is available resulting in dtz + 50-move-counter <= 99,
// then do not accept moves leading to dtz + 50-move-counter == 100.
//
int probe_dtz(Position& pos, int *success)
{
    *success = 1;
    int v = probe_dtz_no_ep(pos, success);

    if (pos.getEnPassantSquare() == NoSquare)
        return v;
    if (*success == 0) return 0;

    // Now handle en passant.
    int v1 = -3, generatedMoves;

    Move moveStack[256];
	generatedMoves = generateMoves(pos, moveStack);
    for (int i = 0; i < generatedMoves; i++) {
        if (moveStack[i].getPromotion() != Pawn)
            continue;
        if (!(pos.makeMove(moveStack[i]))) {
            continue;
        }
        int v0 = -probe_ab(pos, -2, 2, success);
        pos.unmakeMove(moveStack[i]);
        if (*success == 0) return 0;
        if (v0 > v1) v1 = v0;
    }
    if (v1 > -3) {
        v1 = wdl_to_dtz[v1 + 2];
        if (v < -100) {
            if (v1 >= 0)
                v = v1;
        } else if (v < 0) {
                v = v1;
        } else if (v > 100) {
            if (v1 > 0)
                v = v1;
        } else if (v > 0) {
            if (v1 == 1)
                v = v1;
        } else if (v1 >= 0) {
            v = v1;
        } else {
            int i;
            for (i = 0; i < generatedMoves; i++) {
                if (moveStack[i].getPromotion() == Pawn) continue;
                if (pos.makeMove(moveStack[i])) {
                    pos.unmakeMove(moveStack[i]);
                    break;
                }
            }
            if (i == generatedMoves)
                v = v1;
        }
    }

    return v;
}

static int wdl_to_Value[5] = {
    -mateScore + 200 + 1,
    0 - 2,
    0,
    0 + 2,
    mateScore - 200 - 1
};

// Use the DTZ tables to filter out moves that don't preserve the win or draw.
// If the position is lost, but DTZ is fairly high, only keep moves that
// maximise DTZ.
//
// A return value false indicates that not all probes were successful and that
// no moves were filtered out.
bool root_probe(Position& pos, int & TBScore, Move * moveStack, int & generatedMoves)
{
    int success;

    int dtz = probe_dtz(pos, &success);
    if (!success) return false;

    // Probe each move.
	int j = 0;
    for (int i = 0; i < generatedMoves; i++) {
        if (!(pos.makeMove(moveStack[i]))) {
            continue;
        }
        int v = 0;
        if (pos.inCheck() && dtz > 0) {
            Move s[192];
            if (generateEvasions(pos, s) == 0)
                v = 1;
        }
        if (!v) {
            if (pos.getFiftyMoveDistance() != 0) {
                v = -probe_dtz(pos, &success);
                if (v > 0) v++;
                else if (v < 0) v--;
            } else {
                v = -probe_wdl(pos, &success);
                v = wdl_to_dtz[v + 2];
            }
        }
        pos.unmakeMove(moveStack[i]);
        if (!success) return false;
		moveStack[j].setScore(v);
		moveStack[j++].setMove(moveStack[i].getMove());
    }
	generatedMoves = j;

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
    TBScore = wdl_to_Value[wdl + 2];

	j = 0;
    // Now be a bit smart about filtering out moves.
    if (dtz > 0) { // winning (or 50-move rule draw)
        int best = 0xffff;
        for (int i = 0; i < generatedMoves; i++) {
            int v = moveStack[i].getScore();
            if (v > 0 && v < best)
                best = v;
        }
        int max = best;
        // If the current phase has not seen repetitions, then try all moves
        // that stay safely within the 50-move budget, if there are any.
        if (!pos.repetitionDraw() && best + cnt50 <= 99)
            max = 99 - cnt50;
        for (int i = 0; i < generatedMoves; i++) {
            int v = moveStack[i].getScore();
			if (v > 0 && v <= max) {
				moveStack[j++] = moveStack[i];
			}
        }
    } else if (dtz < 0) { // losing (or 50-move rule draw)
        int best = 0;
        for (int i = 0; i < generatedMoves; i++) {
            int v = moveStack[i].getScore();
            if (v < best)
                best = v;
        }
        // Try all moves, unless we approach or have a 50-move rule draw.
        if (-best * 2 + cnt50 < 100)
            return true;
        for (int i = 0; i < generatedMoves; i++) {
			if (moveStack[i].getScore() == best) {
				moveStack[j++] = moveStack[i];
			}

        }
    } else { // drawing
        // Try all moves that preserve the draw.
        for (int i = 0; i < generatedMoves; i++) {
			if (moveStack[i].getScore() == 0) {
				moveStack[j++] = moveStack[i];
			}
        }
    }
    // Update generatedMoves to reflect the new status.
    generatedMoves = j;

    return true;
}

// Use the WDL tables to filter out moves that don't preserve the win or draw.
// This is a fallback for the case that some or all DTZ tables are missing.
// A return value false indicates that not all probes were successful and that
// no moves were filtered out.
bool root_probe_wdl(Position& pos, int & TBScore, Move * moveStack, int & generatedMoves)
{
    int success;

    int wdl = probe_wdl(pos, &success);
    if (!success) return false;
    TBScore = wdl_to_Value[wdl + 2];

    int best = -2;
	int j = 0;
    // Probe each move.
    for (int i = 0; i < generatedMoves; i++) {
        if (!(pos.makeMove(moveStack[i]))) {
            continue;
        }
        int v = -probe_wdl(pos, &success);
        pos.unmakeMove(moveStack[i]);
        if (!success) return false;
		moveStack[j].setScore(v);
		moveStack[j++].setMove(moveStack[i].getMove());
        if (v > best)
            best = v;
    }
	generatedMoves = j;

    j = 0;
    for (int i = 0; i < generatedMoves; i++) {
		if (moveStack[i].getScore() == best) {
			moveStack[j].setScore(best);
			moveStack[j++].setMove(moveStack[i].getMove());
		}
    }
    // Update generatedMoves to reflect the new status.
    generatedMoves = j;

    return true;
}

