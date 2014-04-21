#ifndef BITBOARD_CPP
#define BITBOARD_CPP

#include "bitboard.h"

array<uint64_t, Squares> bit;
array<uint64_t, Squares> kingAttacks;
array<uint64_t, Squares> knightAttacks;
array<uint64_t, Squares> pawnAttacks[2];
array<uint64_t, Squares> pawnSingleMoves[2];
array<uint64_t, Squares> pawnDoubleMoves[2];
array<uint64_t, Squares> rays[8];

// Not really a bitboard but it is easiest to initialize here.
array<int, Squares> heading[Squares];

array<uint64_t, Squares> between[Squares];

array<uint64_t, Squares> passed[Colours];
array<uint64_t, Squares> backward[Colours];
array<uint64_t, Squares> isolated;

void initializeBitMasks();
void initializeKingAttacks();
void initializeKnightAttacks();
void initializePawnAttacks();
void initializePawnMoves();
void initializeRays();
void initializePawnEvaluationBitboards();
void initializeBetween();

void initializeBitboards() 
{	
   initializeBitMasks();
   initializeKingAttacks();
   initializeKnightAttacks();
   initializePawnAttacks();
   initializePawnMoves();
   initializeRays();
   initializePawnEvaluationBitboards();
   initializeBetween();
}

void initializeBitMasks() 
{
	bit.fill(0);

	for (int sq = A1; sq <= H8; sq++) 
	{
		bit[sq] = (uint64_t)1 << sq;
	}
}

void initializeKingAttacks()
{
	uint64_t notAFile = 0xFEFEFEFEFEFEFEFE;
	uint64_t notHFile = 0x7F7F7F7F7F7F7F7F;

	kingAttacks.fill(0);

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
	uint64_t notAFile = 0xFEFEFEFEFEFEFEFE;
	uint64_t notHFile = 0x7F7F7F7F7F7F7F7F;
	uint64_t notABFile = 0xFCFCFCFCFCFCFCFC;
	uint64_t notGHFile = 0x3F3F3F3F3F3F3F3F;

	knightAttacks.fill(0);

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
	uint64_t notAFile = 0xFEFEFEFEFEFEFEFE;
	uint64_t notHFile = 0x7F7F7F7F7F7F7F7F;

	memset(pawnAttacks, 0, sizeof(pawnAttacks));

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
	memset(pawnSingleMoves, 0, sizeof(pawnSingleMoves));
	memset(pawnDoubleMoves, 0, sizeof(pawnDoubleMoves));

	for (int sq = A1; sq <= H8; sq++) 
	{
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

void initializeRays()
{
	array<int, 8> rankDirection = { -1, -1, -1, 0, 0, 1, 1, 1 };
	array<int, 8> fileDirection = { -1, 0, 1, -1, 1, -1, 0, 1 };

	memset(heading, -1, sizeof(heading));

	for (int sq = A1; sq <= H8; sq++)
	{
		int rank = sq / 8;
		int file = sq % 8;

		for (int i = SW; i <= NE; i++)
		{
			for (int j = 1; j < 8; j++)
			{
				int toRank = rankDirection[i] * j + rank;
				int toFile = fileDirection[i] * j + file;
				// Check if we went over the side of the board.
				if (toRank < 0 || toRank > 7 || toFile < 0 || toFile > 7)
				{
					break;
				}
				heading[sq][toRank * 8 + toFile] = i;
				rays[i][sq] |= bit[toRank * 8 + toFile];
			}
		}
	}
}

void initializePawnEvaluationBitboards()
{
	memset(passed, 0, sizeof(passed));
	memset(backward, 0, sizeof(backward));
	isolated.fill(0);

	for (int sq = A2; sq <= H7; sq++)
	{
		int file = sq % 8;
		
		passed[White][sq] = rays[N][sq];
		passed[Black][sq] = rays[S][sq];

		if (file != 7)
		{
			passed[White][sq] |= rays[N][sq + 1];
			passed[Black][sq] |= rays[S][sq + 1];
			backward[White][sq] |= rays[S][sq + 9];
			backward[Black][sq] |= rays[N][sq - 7];
			isolated[sq] |= files[file + 1];
		}
		if (file != 0)
		{
			passed[White][sq] |= rays[N][sq - 1];
			passed[Black][sq] |= rays[S][sq - 1];
			backward[White][sq] |= rays[S][sq + 7];
			backward[Black][sq] |= rays[N][sq - 9];
			isolated[sq] |= files[file - 1];
		}
	}
}

void initializeBetween()
{
	memset(between, 0, sizeof(between));

	for (int i = A1; i <= H8; i++)
	{
		for (int j = A1; j <= H8; j++)
		{
			int h = heading[i][j];
			if (h != -1)
			{
				between[i][j] = rays[h][i] & rays[7 - h][j];
			}
		}
	}
}

#endif

