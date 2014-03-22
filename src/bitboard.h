#ifndef BITBOARD_H
#define BITBOARD_H

#include "defs.h"
#include <intrin.h>

void initializeBitboards();	

extern array<uint64_t, 64 + 1> above;
extern array<uint64_t, 64 + 1> below;
extern array<uint64_t, 64> kingAttacks;
extern array<uint64_t, 64> knightAttacks;
extern array<uint64_t, 64> pawnAttacks[2];
extern array<uint64_t, 64> pawnMoves[2];

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

inline void setBit(uint64_t& bb, int bit)
{
	bb |= (uint64_t)1 << bit;
}

inline void clearBit(uint64_t& bb, int bit)
{
	bb ^= (uint64_t)1 << bit;
}

inline void orBits(uint64_t& bb, uint64_t bits)
{
	bb |= bits;
}

inline void andBits(uint64_t& bb, uint64_t bits)
{
	bb &= bits;
}

inline void xorBits(uint64_t& bb, uint64_t bits)
{
	bb ^= bits;
}

#endif