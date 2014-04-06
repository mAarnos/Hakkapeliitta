#ifndef BITBOARD_H
#define BITBOARD_H

#include "defs.h"
#include <intrin.h>

void initializeBitboards();	

extern array<uint64_t, Squares> bit;
extern array<uint64_t, Squares> kingAttacks;
extern array<uint64_t, Squares> knightAttacks;
extern array<uint64_t, Squares> pawnAttacks[Colours];
extern array<uint64_t, Squares> pawnSingleMoves[Colours];
extern array<uint64_t, Squares> pawnDoubleMoves[Colours];
extern array<uint64_t, Squares> rays[8];

extern array<int, Squares> heading[Squares];

extern array<uint64_t, Squares> passed[Colours];
extern array<uint64_t, Squares> backward[Colours];
extern array<uint64_t, Squares> isolated;

const array<uint64_t, 8> ranks = {
	0x00000000000000FF,
	0x000000000000FF00,
	0x0000000000FF0000,
	0x00000000FF000000,
	0x000000FF00000000,
	0x0000FF0000000000,
	0x00FF000000000000,
	0x00FF000000000000
};

const array<uint64_t, 8> files = {
	0x0101010101010101,
	0x0202020202020202,
	0x0404040404040404,
	0x0808080808080808,
	0x1010101010101010,
	0x2020202020202020,
	0x4040404040404040,
	0x8080808080808080
};

const uint64_t kingSide = 0xE0E0E0E0E0E0E0E0;
const uint64_t queenSide = 0x0707070707070707;
const uint64_t center = 0x1818181818181818;

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