#ifndef BITBOARD_H
#define BITBOARD_H

#include "defs.h"
#include <intrin.h>

extern void initializeBitboards();	

extern array<uint64_t, Squares> bit;
extern array<uint64_t, Squares> kingAttacks;
extern array<uint64_t, Squares> knightAttacks;
extern array<uint64_t, Squares> pawnAttacks[2];
extern array<uint64_t, Squares> pawnSingleMoves[2];
extern array<uint64_t, Squares> pawnDoubleMoves[2];

inline int bitScanForward(uint64_t mask)
{
    unsigned long index; 
	_BitScanForward64(&index, mask);
    return (int)index;
}

inline int popcnt(uint64_t mask)
{
	return (int)_mm_popcnt_u64(mask);
}

// If the machine you are compiling this engine for has no hardware popcnt, comment out the above popcnt and comment in the below software popcnt.
/*
inline int popcnt(uint64_t mask)
{
	mask = mask - ((mask >> 1) & 0x5555555555555555ULL);
	mask = (mask & 0x3333333333333333ULL) +
		((mask >> 2) & 0x3333333333333333ULL);
	mask = (mask + (mask >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
	return (mask * 0x0101010101010101ull) >> 56;
}
*/

#endif