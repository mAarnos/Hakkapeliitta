#ifndef BITBOARD_H
#define BITBOARD_H

#include "defs.h"
#include <intrin.h>

void initializeBitboards();	

const uint64_t notAFile = 0xFEFEFEFEFEFEFEFE;
const uint64_t notHFile = 0x7F7F7F7F7F7F7F7F;

extern array<uint64_t, Squares> bit;
extern array<uint64_t, Squares> kingAttacks;
extern array<uint64_t, Squares> knightAttacks;
extern array<uint64_t, Squares> pawnAttacks[2];
extern array<uint64_t, Squares> pawnSingleMoves[2];
extern array<uint64_t, Squares> pawnDoubleMoves[2];

inline int bitScanForward(uint64_t mask)
{
    unsigned long index; 
    return _BitScanForward64(&index, mask) ? (int)index : 64;
}

inline int bitScanReverse(uint64_t mask)
{
    unsigned long index; 
    return _BitScanReverse64(&index, mask) ? (int)index : 64;
}

inline int popcnt(uint64_t mask)
{
	return (int)_mm_popcnt_u64(mask);
}

#endif