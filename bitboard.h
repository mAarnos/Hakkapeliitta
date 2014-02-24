#ifndef BITBOARD_H
#define BITBOARD_H

#include "defs.h"
#include "board.h"
#include <intrin.h>

#pragma intrinsic(_BitScanForward64)

void initializeBitboards();	

extern U64 setMask[Squares+1];
extern U64 above[Squares+1];
extern U64 below[Squares+1];
extern U64 kingAttacks[Squares];
extern U64 knightAttacks[Squares];
extern U64 whitePawnAttacks[Squares];
extern U64 whitePawnMoves[Squares];
extern U64 blackPawnAttacks[Squares];
extern U64 blackPawnMoves[Squares];

extern U64 RaySW[Squares];
extern U64 RayS[Squares];
extern U64 RaySE[Squares];
extern U64 RayW[Squares];
extern U64 RayE[Squares];
extern U64 RayNW[Squares];
extern U64 RayN[Squares];
extern U64 RayNE[Squares];

extern U64 passedWhite[Squares];
extern U64 backwardWhite[Squares];
extern U64 passedBlack[Squares];
extern U64 backwardBlack[Squares];
extern U64 isolated[Squares];

extern int Headings[Squares][Squares];
extern U64 between[Squares][Squares]; // bitboard with all squares between starting and finishing points

// Checks if the bitboard "bb" has a value of
// true at the bit "square" bits from the right.
inline bool isBitSet(U64 bb, int square)
{
	return (bb & ((U64)1 << square)) != 0;
}

// Returns the type of piece, if any, on a particular square.
inline int getPiece(int square) 
{
	if (isBitSet(whitePieces, square)) 
	{
		if (isBitSet(whitePawns, square))
		{
			return WhitePawn;
		}
		if (isBitSet(whiteKnights, square))
		{
			return WhiteKnight;
		}
		if (isBitSet(whiteBishops, square))
		{
			return WhiteBishop;
		}
		if (isBitSet(whiteRooks, square))
		{
			return WhiteRook;
		}
		if (isBitSet(whiteQueens, square))
		{
			return WhiteQueen;
		}
		return WhiteKing;
	}
	if (isBitSet(blackPieces, square))
	{
		if (isBitSet(blackPawns, square))
		{
			return BlackPawn;
		}
		if (isBitSet(blackKnights, square))
		{
			return BlackKnight;
		}
		if (isBitSet(blackBishops, square))
		{
			return BlackBishop;
		}
		if (isBitSet(blackRooks, square))
		{
			return BlackRook;
		}
		if (isBitSet(blackQueens, square))
		{
			return BlackQueen;
		}
		return BlackKing;
	}
	return Empty;
}

// ONLY WORKS AND IS ONLY USED WITH SYZYGY TABLEBASES
// IS JUST A HACK, FIX SOMEDAY
inline U64 getBitboard(bool color, int piecetype)
{
	if (color)
	{
		if (piecetype == 1)
			return blackPawns;
		if (piecetype == 2)
			return blackKnights;
		if (piecetype == 3)
			return blackBishops;
		if (piecetype == 4)
			return blackRooks;
		if (piecetype == 5)
			return blackQueens;
		if (piecetype == 6)
			return blackKing;
	}
	else
	{
		if (piecetype == 1)
			return whitePawns;
		if (piecetype == 2)
			return whiteKnights;
		if (piecetype == 3)
			return whiteBishops;
		if (piecetype == 4)
			return whiteRooks;
		if (piecetype == 5)
			return whiteQueens;
		if (piecetype == 6)
			return whiteKing;
	}
}

// bitScanForward/bitScanReverse, 64-bit, both return 64 if no bits are set
inline int bitScanForward(U64 mask)
{
    unsigned long index; 
    return _BitScanForward64(&index, mask) ? (int)index : 64;
}

inline int bitScanReverse(U64 mask)
{
    unsigned long index; 
    return _BitScanReverse64(&index, mask) ? (int)index : 64;
}
// 64-bit population count or popcnt for short. Returns the amount of bits set.
inline int popcnt(U64 mask)
{
	return (int)_mm_popcnt_u64(mask);
}

inline int lsb(U64 mask)
{
    unsigned long index; 
    _BitScanForward64(&index, mask);
	return index;
}

inline int pop_lsb(U64 * b) {

  U64 bb = *b;
  *b = bb & (bb - 1);
  return lsb(bb);
}

#endif