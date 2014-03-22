#ifndef BITBOARD_CPP
#define BITBOARD_CPP

#include "bitboard.h"

array<uint64_t, 64 + 1> above;
array<uint64_t, 64 + 1> below;
array<uint64_t, 64> kingAttacks;
array<uint64_t, 64> knightAttacks;
array<uint64_t, 64> pawnAttacks[2];
array<uint64_t, 64> pawnMoves[2];

void initializeBitMasks();
void initializeKingAttacks();
void initializeKnightAttacks();
void initializePawnAttacks();
void initializePawnMoves();

uint64_t notAFile = 0xFEFEFEFEFEFEFEFE;
uint64_t notHFile = 0x7F7F7F7F7F7F7F7F;

void initializeBitboards() 
{	
   initializeBitMasks();
   initializeKingAttacks();
   initializeKnightAttacks();
   initializePawnAttacks();
   initializePawnMoves();
}

void initializeBitMasks() 
{
	for (int sq = A1; sq <= H8; sq++) 
	{
		above[sq] = 0;
		for (int i = sq + 1; i <= 63; i++) 
		{
			setBit(above[sq], i);
		}

		below[sq] = 0;
		for (int i = 0; i < sq; i++) 
		{
			setBit(below[sq], i);
		}
	}
	above[NoSquare] = (uint64_t)0xffffffffffffffff;
	below[NoSquare] = (uint64_t)0xffffffffffffffff;
}

void initializeKingAttacks()
{
	for (int sq = A1; sq <= H8; sq++)
	{
		uint64_t kingSet = 0;

		setBit(kingSet, sq);
		orBits(kingSet, (((uint64_t)1 << sq) << 1) & notAFile);
		orBits(kingSet, (((uint64_t)1 << sq) >> 1) & notHFile);

		xorBits(kingAttacks[sq], kingSet << 8);
		xorBits(kingAttacks[sq], kingSet >> 8);
		orBits(kingAttacks[sq], kingSet);
		clearBit(kingAttacks[sq], sq);
	}
}

void initializeKnightAttacks()
{
	uint64_t notABFile = 0xFCFCFCFCFCFCFCFC;
	uint64_t notGHFile = 0x3F3F3F3F3F3F3F3F;
	for (int sq = A1; sq <= H8; sq++)
	{
		uint64_t knightSet = 0;

		orBits(knightSet, (((uint64_t)1 << sq) << 17) & notAFile);
		orBits(knightSet, (((uint64_t)1 << sq) << 10) & notABFile);
		orBits(knightSet, (((uint64_t)1 << sq) << 15) & notHFile);
		orBits(knightSet, (((uint64_t)1 << sq) << 6) & notGHFile);
		orBits(knightSet, (((uint64_t)1 << sq) >> 17) & notHFile);
		orBits(knightSet, (((uint64_t)1 << sq) >> 10) & notGHFile);
		orBits(knightSet, (((uint64_t)1 << sq) >> 15) & notAFile);
		orBits(knightSet, (((uint64_t)1 << sq) >> 6) & notABFile);

		knightAttacks[sq] = knightSet;
	}
}
	
void initializePawnAttacks()
{
	for (int sq = A2; sq <= H7; sq++)
	{
		orBits(pawnAttacks[White][sq], (((uint64_t)1 << sq) << 9) & notAFile);
		orBits(pawnAttacks[White][sq], (((uint64_t)1 << sq) << 7) & notHFile);

		orBits(pawnAttacks[Black][sq], (((uint64_t)1 << sq) >> 9) & notHFile);
		orBits(pawnAttacks[Black][sq], (((uint64_t)1 << sq) >> 7) & notAFile);
	}
}
   
void initializePawnMoves()
{
	for (int sq = A2; sq <= H7; sq++) 
	{
		pawnMoves[White][sq] = pawnMoves[Black][sq] = 0;
		setBit(pawnMoves[White][sq], sq + 8);
		setBit(pawnMoves[Black][sq], sq - 8);
		if (sq <= H2)
		{
			setBit(pawnMoves[White][sq], sq + 16);
		}
	    if (sq >= A7)
	    {
			setBit(pawnMoves[Black][sq], sq - 16);
	    }
    }	
}	


#endif

