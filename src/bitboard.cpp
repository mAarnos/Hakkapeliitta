#ifndef BITBOARD_CPP
#define BITBOARD_CPP

#include "bitboard.h"

array<uint64_t, Squares + 1> above;
array<uint64_t, Squares + 1> below;
array<uint64_t, Squares> kingAttacks;
array<uint64_t, Squares> knightAttacks;
array<uint64_t, Squares> pawnAttacks[2];
array<uint64_t, Squares> pawnSingleMoves[2];
array<uint64_t, Squares> pawnDoubleMoves[2];

void initializeBitMasks();
void initializeKingAttacks();
void initializeKnightAttacks();
void initializePawnAttacks();
void initializePawnMoves();

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
		kingSet |= (((uint64_t)1 << sq) << 1) & notAFile;
		kingSet |= (((uint64_t)1 << sq) >> 1) & notHFile;

		kingAttacks[sq] |= kingSet << 8;
		kingAttacks[sq] |= kingSet >> 8;
		kingAttacks[sq] |= kingSet;
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

		knightSet |= (((uint64_t)1 << sq) << 17) & notAFile;
		knightSet |= (((uint64_t)1 << sq) << 10) & notABFile;
		knightSet |= (((uint64_t)1 << sq) << 15) & notHFile;
		knightSet |= (((uint64_t)1 << sq) << 6) & notGHFile;
		knightSet |= (((uint64_t)1 << sq) >> 17) & notHFile;
		knightSet |= (((uint64_t)1 << sq) >> 10) & notGHFile;
		knightSet |= (((uint64_t)1 << sq) >> 15) & notAFile;
		knightSet |= (((uint64_t)1 << sq) >> 6) & notABFile;

		knightAttacks[sq] = knightSet;
	}
}
	
void initializePawnAttacks()
{
	for (int sq = A1; sq <= H8; sq++)
	{
		pawnAttacks[White][sq] |= (((uint64_t)1 << sq) << 9) & notAFile;
		pawnAttacks[White][sq] |= (((uint64_t)1 << sq) << 7) & notHFile;

		pawnAttacks[Black][sq] |= (((uint64_t)1 << sq) >> 9) & notHFile;
		pawnAttacks[Black][sq] |= (((uint64_t)1 << sq) >> 7) & notAFile;
	}
}
   
void initializePawnMoves()
{
	for (int sq = A1; sq <= H8; sq++) 
	{
		pawnSingleMoves[White][sq] = pawnSingleMoves[Black][sq] = 0;
		pawnDoubleMoves[White][sq] = pawnDoubleMoves[Black][sq] = 0;

		setBit(pawnSingleMoves[White][sq], sq + 8);
		setBit(pawnSingleMoves[Black][sq], sq - 8);
		if (sq <= H2)
		{
			setBit(pawnDoubleMoves[White][sq], sq + 16);
		}
	    if (sq >= A7)
	    {
			setBit(pawnDoubleMoves[Black][sq], sq - 16);
	    }
    }	
}	


#endif

