#ifndef HASH_CPP
#define HASH_CPP

#include "hash.h"
#include "random.h"

array<uint64_t, Squares> pieceHash[12];
array<uint64_t, 8> materialHash[12];
array<uint64_t, 16> castlingRightsHash;
array<uint64_t, Squares> enPassantHash;
uint64_t turnHash;

void initializeHash()
{
	WELL512 rng(119582769);

	for (int i = WhitePawn; i <= BlackKing; i++)
	{
		for (int j = A1; j <= H8; j++)
		{
			pieceHash[i][j] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
		}
		for (int j = 0; j < 8; j++)
		{
			materialHash[i][j] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
		}
	}

	for (int i = A1; i <= H8; i++)
	{
		enPassantHash[i] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
	}

	for (int i = 0; i < 16; i++)
	{
		castlingRightsHash[i] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
	}

	turnHash = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
}

#endif