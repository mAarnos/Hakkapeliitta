#include "bitboard.hpp"

array<uint64_t, Squares> bit;
array<uint64_t, Squares> kingAttacks;
array<uint64_t, Squares> knightAttacks;
array<uint64_t, Squares> pawnAttacks[2];
array<uint64_t, Squares> pawnSingleMoves[2];
array<uint64_t, Squares> pawnDoubleMoves[2];
array<uint64_t, Squares> rays[8];

array<uint64_t, Squares> kingZone[2];

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
	uint64_t kingSet;
	kingAttacks.fill(0);

	for (int sq = A1; sq <= H8; sq++)
	{
		kingSet = bit[sq];
		kingSet |= (((uint64_t)1 << sq) << 1) & 0xFEFEFEFEFEFEFEFE;
		kingSet |= (((uint64_t)1 << sq) >> 1) & 0x7F7F7F7F7F7F7F7F;

		kingAttacks[sq] |= kingSet << 8;
		kingAttacks[sq] |= kingSet >> 8;
		kingAttacks[sq] |= kingSet;
		kingAttacks[sq] ^= bit[sq];
	}

    for (int sq = A1; sq <= H8; sq++)
    {
        if (sq < A8)
            kingZone[White][sq] = kingAttacks[sq] | kingAttacks[sq + 8];
        if (sq > H1)
            kingZone[Black][sq] = kingAttacks[sq] | kingAttacks[sq - 8];
    }
}

void initializeKnightAttacks()
{
	uint64_t knightSet;
	knightAttacks.fill(0);

	for (int sq = A1; sq <= H8; sq++)
	{
		knightSet = (((uint64_t)1 << sq) << 17) & 0xFEFEFEFEFEFEFEFE;
		knightSet |= (((uint64_t)1 << sq) << 10) & 0xFCFCFCFCFCFCFCFC;
		knightSet |= (((uint64_t)1 << sq) << 15) & 0x7F7F7F7F7F7F7F7F;
		knightSet |= (((uint64_t)1 << sq) << 6) & 0x3F3F3F3F3F3F3F3F;
		knightSet |= (((uint64_t)1 << sq) >> 17) & 0x7F7F7F7F7F7F7F7F;
		knightSet |= (((uint64_t)1 << sq) >> 10) & 0x3F3F3F3F3F3F3F3F;
		knightSet |= (((uint64_t)1 << sq) >> 15) & 0xFEFEFEFEFEFEFEFE;
		knightSet |= (((uint64_t)1 << sq) >> 6) & 0xFCFCFCFCFCFCFCFC;

		knightAttacks[sq] = knightSet;
	}
}
	
void initializePawnAttacks()
{
	memset(pawnAttacks, 0, sizeof(pawnAttacks));

	for (int sq = A1; sq <= H8; sq++)
	{
		pawnAttacks[White][sq] |= (((uint64_t)1 << sq) << 9) & 0xFEFEFEFEFEFEFEFE;
		pawnAttacks[White][sq] |= (((uint64_t)1 << sq) << 7) & 0x7F7F7F7F7F7F7F7F;

		pawnAttacks[Black][sq] |= (((uint64_t)1 << sq) >> 9) & 0x7F7F7F7F7F7F7F7F;
		pawnAttacks[Black][sq] |= (((uint64_t)1 << sq) >> 7) & 0xFEFEFEFEFEFEFEFE;
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
		int rank = Rank(sq);
		int file = File(sq);

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
		int file = File(sq);
		
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

