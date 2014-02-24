/*
  Copyright (c) 2013 Ronald de Man
  This file may be redistributed and/or modified without restrictions.

  tbprobe.cpp contains the Stockfish-specific routines of the
  tablebase probing code. It should be relatively easy to adapt
  this code to other chess engines.
*/

// The probing code currently expects a little-endian architecture (e.g. x86).

#include "movegen.h"
#include "bitboard.h"
#include "search.h"
#include "hash.h"
#include "tbprobe.h"
#include "tbcore.h"
#include "eval.h"

// Given a position with 6 or fewer pieces, produce a text string
// of the form KQPvKRP, where "KQP" represents the white pieces if
// mirror == 0 and the black pieces if mirror == 1.
// mikko:done
static void prt_str(char *str, int mirror)
{
  bool color = !mirror ? White : Black;
  int i;

  if (color)
  {
	  *str++ = pchr[0];
	  for (i = 0; i < popcnt(blackQueens); i++)
	  {
		*str++ = pchr[1];
	  }
	  for (i = 0; i < popcnt(blackRooks); i++)
	  {
		*str++ = pchr[2];
	  }
	  for (i = 0; i < popcnt(blackBishops); i++)
	  {
		*str++ = pchr[3];
	  }
	  for (i = 0; i < popcnt(blackKnights); i++)
	  {
		*str++ = pchr[4];
	  }
	  for (i = 0; i < popcnt(blackPawns); i++)
	  {
		*str++ = pchr[5];
	  }

	  *str++ = 'v';

	  *str++ = pchr[0];
	  for (i = 0; i < popcnt(whiteQueens); i++)
	  {
		*str++ = pchr[1];
	  }
	  for (i = 0; i < popcnt(whiteRooks); i++)
	  {
		*str++ = pchr[2];
	  }
	  for (i = 0; i < popcnt(whiteBishops); i++)
	  {
		*str++ = pchr[3];
	  }
	  for (i = 0; i < popcnt(whiteKnights); i++)
	  {
		*str++ = pchr[4];
	  }
	  for (i = 0; i < popcnt(whitePawns); i++)
	  {
		*str++ = pchr[5];
	  }
  }
  else
  {
	  *str++ = pchr[0];
	  for (i = 0; i < popcnt(whiteQueens); i++)
	  {
		*str++ = pchr[1];
	  }
	  for (i = 0; i < popcnt(whiteRooks); i++)
	  {
		*str++ = pchr[2];
	  }
	  for (i = 0; i < popcnt(whiteBishops); i++)
	  {
		*str++ = pchr[3];
	  }
	  for (i = 0; i < popcnt(whiteKnights); i++)
	  {
		*str++ = pchr[4];
	  }
	  for (i = 0; i < popcnt(whitePawns); i++)
	  {
		*str++ = pchr[5];
	  }
	  
	  *str++ = 'v';

	  *str++ = pchr[0];
	  for (i = 0; i < popcnt(blackQueens); i++)
	  {
		*str++ = pchr[1];
	  }
	  for (i = 0; i < popcnt(blackRooks); i++)
	  {
		*str++ = pchr[2];
	  }
	  for (i = 0; i < popcnt(blackBishops); i++)
	  {
		*str++ = pchr[3];
	  }
	  for (i = 0; i < popcnt(blackKnights); i++)
	  {
		*str++ = pchr[4];
	  }
	  for (i = 0; i < popcnt(blackPawns); i++)
	  {
		*str++ = pchr[5];
	  }
  }
  *str++ = 0;
}

// Given a position, produce a 64-bit material signature key.
// If the engine supports such a key, it should equal the engine's key.
// mikko:done
static U64 calc_key(int mirror)
{
  bool color = !mirror ? White : Black;
  int i;
  U64 key = 0;

  if (color)
  {
	  for (i = 0; i < popcnt(blackQueens); i++)
	  {
		key ^= materialHash[White][4][i];
	  }
	  for (i = 0; i < popcnt(blackRooks); i++)
	  {
		key ^= materialHash[White][3][i];
	  }
	  for (i = 0; i < popcnt(blackBishops); i++)
	  {
		key ^= materialHash[White][2][i];
	  }
	  for (i = 0; i < popcnt(blackKnights); i++)
	  {
		key ^= materialHash[White][1][i];
	  }
	  for (i = 0; i < popcnt(blackPawns); i++)
	  {
		key ^= materialHash[White][0][i];
	  }

	  for (i = 0; i < popcnt(whiteQueens); i++)
	  {
		key ^= materialHash[Black][4][i];
	  }
	  for (i = 0; i < popcnt(whiteRooks); i++)
	  {
		key ^= materialHash[Black][3][i];
	  }
	  for (i = 0; i < popcnt(whiteBishops); i++)
	  {
		key ^= materialHash[Black][2][i];
	  }
	  for (i = 0; i < popcnt(whiteKnights); i++)
	  {
		key ^= materialHash[Black][1][i];
	  }
	  for (i = 0; i < popcnt(whitePawns); i++)
	  {
		key ^= materialHash[Black][0][i];
	  }
  }
  else
  {
	  for (i = 0; i < popcnt(whiteQueens); i++)
	  {
		key ^= materialHash[White][4][i];
	  }
	  for (i = 0; i < popcnt(whiteRooks); i++)
	  {
		key ^= materialHash[White][3][i];
	  }
	  for (i = 0; i < popcnt(whiteBishops); i++)
	  {
		key ^= materialHash[White][2][i];
	  }
	  for (i = 0; i < popcnt(whiteKnights); i++)
	  {
		key ^= materialHash[White][1][i];
	  }
	  for (i = 0; i < popcnt(whitePawns); i++)
	  {
		key ^= materialHash[White][0][i];
	  }

	  for (i = 0; i < popcnt(blackQueens); i++)
	  {
		key ^= materialHash[Black][4][i];
	  }
	  for (i = 0; i < popcnt(blackRooks); i++)
	  {
		key ^= materialHash[Black][3][i];
	  }
	  for (i = 0; i < popcnt(blackBishops); i++)
	  {
		key ^= materialHash[Black][2][i];
	  }
	  for (i = 0; i < popcnt(blackKnights); i++)
	  {
		key ^= materialHash[Black][1][i];
	  }
	  for (i = 0; i < popcnt(blackPawns); i++)
	  {
		key ^= materialHash[Black][0][i];
	  }
  }

  return key;
}

// Produce a 64-bit material key corresponding to the material combination
// defined by pcs[16], where pcs[1], ..., pcs[6] is the number of white
// pawns, ..., kings and pcs[9], ..., pcs[14] is the number of black
// pawns, ..., kings.
U64 calc_key_from_pcs(int *pcs, int mirror)
{
  int i;
  U64 key = 0;

  if (mirror)
  {
	  for (i = 0; i < pcs[8 + 5]; i++)
	  {
		key ^= materialHash[White][4][i];
	  }
	  for (i = 0; i < pcs[8 + 4]; i++)
	  {
		key ^= materialHash[White][3][i];
	  }
	  for (i = 0; i < pcs[8 + 3]; i++)
	  {
		key ^= materialHash[White][2][i];
	  }
	  for (i = 0; i < pcs[8 + 2]; i++)
	  {
		key ^= materialHash[White][1][i];
	  }
	  for (i = 0; i < pcs[8 + 1]; i++)
	  {
		key ^= materialHash[White][0][i];
	  }

	  for (i = 0; i < pcs[5]; i++)
	  {
		key ^= materialHash[Black][4][i];
	  }
	  for (i = 0; i < pcs[4]; i++)
	  {
		key ^= materialHash[Black][3][i];
	  }
	  for (i = 0; i < pcs[3]; i++)
	  {
		key ^= materialHash[Black][2][i];
	  }
	  for (i = 0; i < pcs[2]; i++)
	  {
		key ^= materialHash[Black][1][i];
	  }
	  for (i = 0; i < pcs[1]; i++)
	  {
		key ^= materialHash[Black][0][i];
	  }
  }
  else
  {
	  for (i = 0; i < pcs[5]; i++)
	  {
		key ^= materialHash[White][4][i];
	  }
	  for (i = 0; i < pcs[4]; i++)
	  {
		key ^= materialHash[White][3][i];
	  }
	  for (i = 0; i < pcs[3]; i++)
	  {
		key ^= materialHash[White][2][i];
	  }
	  for (i = 0; i < pcs[2]; i++)
	  {
		key ^= materialHash[White][1][i];
	  }
	  for (i = 0; i < pcs[1]; i++)
	  {
		key ^= materialHash[White][0][i];
	  }

	  for (i = 0; i < pcs[8 + 5]; i++)
	  {
		key ^= materialHash[Black][4][i];
	  }
	  for (i = 0; i < pcs[8 + 4]; i++)
	  {
		key ^= materialHash[Black][3][i];
	  }
	  for (i = 0; i < pcs[8 + 3]; i++)
	  {
		key ^= materialHash[Black][2][i];
	  }
	  for (i = 0; i < pcs[8 + 2]; i++)
	  {
		key ^= materialHash[Black][1][i];
	  }
	  for (i = 0; i < pcs[8 + 1]; i++)
	  {
		key ^= materialHash[Black][0][i];
	  }
  }

  return key;
}

// probe_wdl_table and probe_dtz_table require similar adaptations.
static int probe_wdl_table(int *success)
{
  struct TBEntry *ptr;
  struct TBHashEntry *ptr2;
  U64 idx;
  U64 key;
  int i;
  U8 res;
  int p[TBPIECES];

  // Obtain the position's material signature key.
  key = calc_key(false);

  // Test for KvK.
  if (!key) return 0;

  ptr2 = TB_hash[key >> (64 - TBHASHBITS)];
  for (i = 0; i < HSHMAX; i++)
    if (ptr2[i].key == key) break;
  if (i == HSHMAX) {
    *success = 0;
    return 0;
  }

  ptr = ptr2[i].ptr;
  if (!ptr->ready) {
    if (!ptr->ready) {
      char str[16];
      prt_str(str, ptr->key != key);
      if (!init_table_wdl(ptr, str)) {
	ptr->data = NULL;
	ptr2[i].key = 0ULL;
	*success = 0;
	return 0;
      }
      ptr->ready = 1;
    }
  }
 
  int bside, mirror, cmirror;
  if (!ptr->symmetric) {
    if (key != ptr->key) {
      cmirror = 8;
      mirror = 0x38;
      bside = (sideToMove == White);
    } else {
      cmirror = mirror = 0;
      bside = !(sideToMove == White);
    }
  } else {
    cmirror = sideToMove == White ? 0 : 8;
    mirror = sideToMove == White ? 0 : 0x38;
    bside = 0;
  }

  // p[i] is to contain the square 0-63 (A1-H8) for a piece of type
  // pc[i] ^ cmirror, where 1 = white pawn, ..., 14 = black king.
  // Pieces of the same type are guaranteed to be consecutive.
  if (!ptr->has_pawns) {
    struct TBEntry_piece *entry = (struct TBEntry_piece *)ptr;
    U8 *pc = entry->pieces[bside];
    for (i = 0; i < entry->num;) {
      U64 bb = getBitboard((bool)((pc[i] ^ cmirror) >> 3),
				      (int)(pc[i] & 0x07));
      do {
	p[i++] = pop_lsb(&bb);
      } while (bb);
    }
    idx = encode_piece(entry, entry->norm[bside], p, entry->factor[bside]);
    res = decompress_pairs(entry->precomp[bside], idx);
  } else {
    struct TBEntry_pawn *entry = (struct TBEntry_pawn *)ptr;
    int k = entry->file[0].pieces[0][0] ^ cmirror;
	// bb is zero, which causes an exception
    U64 bb = getBitboard((bool)(k >> 3), (int)(k & 0x07));
    i = 0;
    do {
      p[i++] = pop_lsb(&bb) ^ mirror;
    } while (bb);
    int f = pawn_file(entry, p);
    U8 *pc = entry->file[f].pieces[bside];
    for (; i < entry->num;) {
      bb = getBitboard((bool)((pc[i] ^ cmirror) >> 3),
				    (int)(pc[i] & 0x07));
      do {
	p[i++] = pop_lsb(&bb) ^ mirror;
      } while (bb);
    }
    idx = encode_pawn(entry, entry->file[f].norm[bside], p, entry->file[f].factor[bside]);
    res = decompress_pairs(entry->file[f].precomp[bside], idx);
  }

  return ((int)res) - 2;
}

static int probe_dtz_table(int wdl, int *success)
{
  struct TBEntry *ptr;
  U64 idx;
  int i, res;
  int p[TBPIECES];

  // Obtain the position's material signature key.
  U64 key = calc_key(false);

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
      prt_str(str, mirror);
      if (DTZ_table[DTZ_ENTRIES - 1].entry)
	free_dtz_entry(DTZ_table[DTZ_ENTRIES-1].entry);
      for (i = DTZ_ENTRIES - 1; i > 0; i--)
	DTZ_table[i] = DTZ_table[i - 1];
      load_dtz_table(str, calc_key(mirror), calc_key(!mirror));
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
      bside = (sideToMove == White);
    } else {
      cmirror = mirror = 0;
      bside = !(sideToMove == White);
    }
  } else {
    cmirror = sideToMove == White ? 0 : 8;
    mirror = sideToMove == White ? 0 : 0x38;
    bside = 0;
  }

  if (!ptr->has_pawns) {
    struct DTZEntry_piece *entry = (struct DTZEntry_piece *)ptr;
    if ((entry->flags & 1) != bside && !entry->symmetric) {
      *success = -1;
      return 0;
    }
    U8 *pc = entry->pieces;
    for (i = 0; i < entry->num;) {
      U64 bb = getBitboard((bool)((pc[i] ^ cmirror) >> 3),
				    (int)(pc[i] & 0x07));
      do {
	p[i++] = pop_lsb(&bb);
      } while (bb);
    }
    idx = encode_piece((struct TBEntry_piece *)entry, entry->norm, p, entry->factor);
    res = decompress_pairs(entry->precomp, idx);

    if (entry->flags & 2)
      res = entry->map[entry->map_idx[wdl_to_map[wdl + 2]] + res];

    if (!(entry->flags & pa_flags[wdl + 2]) && !(wdl & 1))
      res *= 2;
  } else {
    struct DTZEntry_pawn *entry = (struct DTZEntry_pawn *)ptr;
    int k = entry->file[0].pieces[0] ^ cmirror;
    U64 bb = getBitboard((bool)(k >> 3), (int)(k & 0x07));
    i = 0;
    do {
      p[i++] = pop_lsb(&bb) ^ mirror;
    } while (bb);
    int f = pawn_file((struct TBEntry_pawn *)entry, p);
    if ((entry->flags[f] & 1) != bside) {
      *success = -1;
      return 0;
    }
    U8 *pc = entry->file[f].pieces;
    for (; i < entry->num;) {
      bb = getBitboard((bool)((pc[i] ^ cmirror) >> 3),
			    (int)(pc[i] & 0x07));
      do {
	p[i++] = pop_lsb(&bb) ^ mirror;
      } while (bb);
    }
    idx = encode_pawn((struct TBEntry_pawn *)entry, entry->file[f].norm, p, entry->file[f].factor);
    res = decompress_pairs(entry->file[f].precomp, idx);

    if (entry->flags[f] & 2)
      res = entry->map[entry->map_idx[f][wdl_to_map[wdl + 2]] + res];

    if (!(entry->flags[f] & pa_flags[wdl + 2]) && !(wdl & 1))
      res *= 2;
  }

  return res;
}

static int probe_ab(int alpha, int beta, int *success)
{
  int v;

  generateMoves();
  for (int i = moveList[ply]; i < moveList[ply + 1]; i++) 
  {
	 if (!moveStack[i].isCapture() || moveStack[i].isEnpassant())
		 continue;
    if (!(make(moveStack[i].moveInt))) 
	{
		continue;
	}
    v = -probe_ab(-beta, -alpha, success);
    unmake(moveStack[i].moveInt);
    if (*success == 0) return 0;
    if (v > alpha) {
      if (v >= beta) {
        *success = 2;
	return v;
      }
      alpha = v;
    }
  }

  v = probe_wdl_table(success);
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
int probe_wdl(int *success)
{
  int v, i;
  bool otherlegalfound = false;

  *success = 1;
  v = probe_ab(-2, 2, success);

  // If en passant is not possible, we are done.
  if (enPassant == 64)
    return v;
  if (!(*success)) return 0;

  // Now handle en passant.
  int v1 = -3;

  // Generate (at least) all legal en passant captures.
  generateMoves();
  for (i = moveList[ply]; i < moveList[ply + 1]; i++) 
  {
	if (!moveStack[i].isEnpassant())
		 continue;
    if (!(make(moveStack[i].moveInt))) 
	{
		continue;
	}
    int v0 = -probe_ab(-2, 2, success);
    unmake(moveStack[i].moveInt);
    if (*success == 0) return 0;
    if (v0 > v1) v1 = v0;
  }
  if (v1 > -3) 
  {
    if (v1 >= v) v = v1;
    else if (v == 0) 
	{
      // Check whether there is at least one legal non-ep move.
      for (i = moveList[ply]; i < moveList[ply + 1]; i++)  
	  {
		if (moveStack[i].isEnpassant()) continue;
		if (!(make(moveStack[i].moveInt))) 
		{
			continue;
		}
		unmake(moveStack[i].moveInt);
		otherlegalfound = true;
		break;
      }
	}
	  // If not, then we are forced to play the losing ep capture.
      if (!otherlegalfound)
		v = v1;
  }

  return v;
}

// This routine treats a position with en passant captures as one without.
static int probe_dtz_no_ep(int *success)
{
  int wdl, dtz;

  wdl = probe_ab(-2, 2, success);
  if (*success == 0) return 0;

  if (wdl == 0) return 0;

  if (*success == 2)
    return wdl == 2 ? 1 : 101;

  generateMoves();
  if (wdl > 0) {
    // Generate at least all legal non-capturing pawn moves
    // including non-capturing promotions.
    for (int i = moveList[ply]; i < moveList[ply + 1]; i++)
	{
      if (!moveStack[i].isPawnmove() || moveStack[i].isCapture())
          continue;
	  if (!(make(moveStack[i].moveInt))) 
		{
			continue;
		}
      int v = -probe_ab(-2, -wdl + 1, success);
      unmake(moveStack[i].moveInt);
      if (*success == 0) return 0;
      if (v == wdl)
	return v == 2 ? 1 : 101;
    }
  }

  dtz = 1 + probe_dtz_table(wdl, success);
  if (*success >= 0) {
    if (wdl & 1) dtz += 100;
    return wdl >= 0 ? dtz : -dtz;
  }

  if (wdl > 0) {
    int best = 0xffff;
    for (int i = moveList[ply]; i < moveList[ply + 1]; i++) {
      if (moveStack[i].isPawnmove() || moveStack[i].isCapture())
          continue;
	  if (!(make(moveStack[i].moveInt))) 
	  {
	      continue;
	  }
      int v = -probe_dtz(success);
       unmake(moveStack[i].moveInt);
      if (*success == 0) return 0;
      if (v > 0 && v + 1 < best)
	best = v + 1;
    }
    return best;
  } else {
    int best = -1;
    for (int i = moveList[ply]; i < moveList[ply + 1]; i++) {
      int v;
	  if (!(make(moveStack[i].moveInt))) 
	  {
	      continue;
	  }
	  // possible error here
      if (fiftyMoveDistance == 0) {
	if (wdl == -2) v = -1;
	else {
	  v = probe_ab(1, 2, success);
	  v = (v == 2) ? 0 : -101;
	}
      } else {
	v = -probe_dtz(success) - 1;
      }
      unmake(moveStack[i].moveInt);
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
//         n < -100 : 
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
int probe_dtz(int *success)
{
  *success = 1;
  int moves;
  int v = probe_dtz_no_ep(success);

  if (enPassant == 64)
    return v;
  if (*success == 0) return 0;

  // Now handle en passant.
  int v1 = -3;
  bool legalfound = false;

  generateMoves();
  for (int i = moveList[ply]; i < moveList[ply + 1]; i++) {
    if (!moveStack[i].isEnpassant())
		continue;
	if (!(make(moveStack[i].moveInt))) 
	  {
	      continue;
	  }
    int v0 = -probe_ab(-2, 2, success);
    unmake(moveStack[i].moveInt);
    if (*success == 0) return 0;
    if (v0 > v1) v1 = v0;
  }
  if (v1 > -3) {
    v1 = wdl_to_dtz[v1 + 2];
    if (v < -100) {
      if (v1 >= 0)
	v = v1;
    } else if (v < 0) {
      if (v1 >= 0 || v1 < 100)
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
      for (moves = moveList[ply]; moves < moveList[ply + 1]; moves++) 
	  {
		if (moveStack[moves].isEnpassant())
			continue;
		if (!(make(moveStack[moves].moveInt))) 
		 {
			 continue;
		 }
		unmake(moveStack[moves].moveInt);
		legalfound = true;
		break;
      }
      if (!legalfound) 
		v = v1;
    }
  }

  return v;
}

// Use the DTZ tables to filter out moves that don't preserve the win or draw.
// If the position is lost, but DTZ is fairly high, only keep moves that
// maximise DTZ.
//
// A return value of 0 indicates that not all probes were successful and that
// no moves were filtered out.
int root_probe()
{
	int success, v;

	int wdl = probe_wdl(&success);
	if (!success) return 0;

	// Probe each move.
	for (int i = moveList[ply]; i < moveList[ply + 1]; i++) 
	{
		if (!(make(moveStack[i].moveInt))) 
		{
			continue;
		}
		if (fiftyMoveDistance != 0)
		{
			v = -probe_dtz(&success);
			if (v > 0)
			{
				v++;
			}
			else if (v < 0)
			{
				v--;
			}
		}
		else 
		{
			v = -probe_wdl(&success);
			v = wdl_to_dtz[v + 2];
		}
		unmake(moveStack[i].moveInt);
		if (!success) return 0;
		moveStack[i].score = v;
	}
	if (wdl > 0)
	{
		int best = 65535;
		for (int i = moveList[ply]; i < moveList[ply + 1]; i++) 
		{
			int v = moveStack[i].score;
			if (v > 0 && v < best)
				best = v;
		}
		int max = best;
		// If the current phase has not seen repetitions, then try all moves
		// that stay safely within the 50-move budget.
		if (!repetitionCount() && best + fiftyMoveDistance <= 99)
			max = 99 - fiftyMoveDistance;
		for (int i = moveList[ply]; i < moveList[ply + 1]; i++) 
		{
			int v = moveStack[i].score;
			if (!(v > 0 && v <= max))
				moveStack[i].score = -mateScore;
		}
	}
	else if (wdl < 0)
	{
		int best = 0;
		for (int i = moveList[ply]; i < moveList[ply + 1]; i++) 
		{
			int v = moveStack[i].score;
			if (v < best)
				best = v;
		}
		// try all moves unless we are approaching or have a fifty-move draw
		if (-best * 2 + fiftyMoveDistance < 100)
			return 1;
		else 
		{
			for (int i = moveList[ply]; i < moveList[ply + 1]; i++) 
			{
				if (best != moveStack[i].score)
					moveStack[i].score = -mateScore;
			}
		}
	}
	else 
	{
		for (int i = moveList[ply]; i < moveList[ply + 1]; i++) 
		{
			// only try moves which preserve the draw
			if (moveStack[i].score != 0)
			{
				moveStack[i].score = -mateScore;
			}
		}
	}
	return 1;
}

