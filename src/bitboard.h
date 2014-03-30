#ifndef BITBOARD_H
#define BITBOARD_H

#include "defs.h"
#include <intrin.h>

void initializeBitboards();	

extern array<uint64_t, Squares> bit;
extern array<uint64_t, Squares> kingAttacks;
extern array<uint64_t, Squares> knightAttacks;
extern array<uint64_t, Squares> pawnAttacks[2];
extern array<uint64_t, Squares> pawnSingleMoves[2];
extern array<uint64_t, Squares> pawnDoubleMoves[2];

// Returns the least significant set bit in the mask.
// Precondition: mask != 0
inline int bitScanForward(uint64_t mask)
{
    unsigned long index; 
	_BitScanForward64(&index, mask);
    return (int)index;
}

// If the machine you are compiling this engine for has no hardware bitScanForward(only w32 nowadays?), use the below bitScanForward instead of the above.
// The code for this software bitScanFotward is by Kim Walisch(2012)
/*
const array<int, Squares> index64 = {
	0, 47, 1, 56, 48, 27, 2, 60,
	57, 49, 41, 37, 28, 16, 3, 61,
	54, 58, 35, 52, 50, 42, 21, 44,
	38, 32, 29, 23, 17, 11, 4, 62,
	46, 55, 26, 59, 40, 36, 15, 53,
	34, 51, 20, 43, 31, 22, 10, 45,
	25, 39, 14, 33, 19, 30, 9, 24,
	13, 18, 8, 12, 7, 6, 5, 63
};

inline int bitScanForward(uint64_t bb) 
{
	return index64[((bb ^ (bb - 1)) * 0x03f79d71b4cb0a89) >> 58];
}
*/

// Returns the amount of set bits in the mask.
inline int popcnt(uint64_t mask)
{
	return (int)_mm_popcnt_u64(mask);
}

// If the machine you are compiling this engine for has no hardware popcnt(w32 and old x64 machines), use the below popnct instead of the above.
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