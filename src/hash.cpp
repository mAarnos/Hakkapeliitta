#ifndef HASH_CPP
#define HASH_CPP

#include "hash.h"
#include "random.h"

array<uint64_t, 6> pieceHash[64][2];
array<uint64_t, 8> materialHash[2][6];
array<uint64_t, 16> castlingRightsHash;
array<uint64_t, 64> enPassantHash;
uint64_t turnHash;

void initializeHash()
{
	WELL512 rng(119582769);

	for (int i = A1; i <= H8; i++)
	{
		enPassantHash[i] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
		for (int j = White; j <= Black; j++)
		{
			for (int k = Pawn; k <= King; k++)
			{
				pieceHash[i][j][k] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
			}
		}
	}

	for (int i = 0; i < 16; i++)
	{
		castlingRightsHash[i] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
	}

	for (int i = White; i <= Black; i++)
	{
		for (int j = Pawn; j <= King; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				materialHash[i][j][k] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
			}
		}
	}

	turnHash = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
}

#endif