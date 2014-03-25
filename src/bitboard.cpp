#ifndef BITBOARD_CPP
#define BITBOARD_CPP

#include "bitboard.h"

array<uint64_t, Squares> bit;
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
		bit[sq] = (uint64_t)1 << sq;
	}
}

void initializeKingAttacks()
{
	for (int sq = A1; sq <= H8; sq++)
	{
		uint64_t kingSet = 0;

		kingSet |= bit[sq];
		kingSet |= (((uint64_t)1 << sq) << 1) & notAFile;
		kingSet |= (((uint64_t)1 << sq) >> 1) & notHFile;

		kingAttacks[sq] |= kingSet << 8;
		kingAttacks[sq] |= kingSet >> 8;
		kingAttacks[sq] |= kingSet;
		kingAttacks[sq] ^= bit[sq];
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

		if (sq <= H7)
		{
			pawnSingleMoves[White][sq] = bit[sq + 8];
		}
		if (sq >= A2)
		{
			pawnSingleMoves[Black][sq] = bit[sq - 8];
		}
		if (sq <= H2)
		{
			pawnDoubleMoves[White][sq] = bit[sq + 16];
		}
	    if (sq >= A7)
	    {
			pawnDoubleMoves[Black][sq] = bit[sq - 16];
	    }
    }	
}	


#endif

