#include "hash.h"
#include "random.h"
#include "position.h"

array<uint64_t, Squares> pieceHash[12];
array<uint64_t, 8> materialHash[12];
array<uint64_t, 16> castlingRightsHash;
array<uint64_t, Squares> enPassantHash;
uint64_t turnHash;

void initializeHash()
{
	WELL512 rng(123456789);

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

	turnHash = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());

	// Do something about this mess.
	castlingRightsHash[1] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
	castlingRightsHash[2] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
	castlingRightsHash[3] = castlingRightsHash[1] ^ castlingRightsHash[2];
	castlingRightsHash[4] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
	castlingRightsHash[5] = castlingRightsHash[1] ^ castlingRightsHash[4];
	castlingRightsHash[6] = castlingRightsHash[2] ^ castlingRightsHash[4];
	castlingRightsHash[7] = castlingRightsHash[1] ^ castlingRightsHash[2] ^ castlingRightsHash[4];
	castlingRightsHash[8] = (uint64_t(rng.rand()) << 32) | uint64_t(rng.rand());
	castlingRightsHash[9] = castlingRightsHash[1] ^ castlingRightsHash[8];
	castlingRightsHash[10] = castlingRightsHash[2] ^ castlingRightsHash[8];
	castlingRightsHash[11] = castlingRightsHash[1] ^ castlingRightsHash[2] ^ castlingRightsHash[8];
	castlingRightsHash[12] = castlingRightsHash[4] ^ castlingRightsHash[8];
	castlingRightsHash[13] = castlingRightsHash[1] ^ castlingRightsHash[4] ^ castlingRightsHash[8];
	castlingRightsHash[14] = castlingRightsHash[2] ^ castlingRightsHash[4] ^ castlingRightsHash[8];
	castlingRightsHash[15] = castlingRightsHash[1] ^ castlingRightsHash[2] ^ castlingRightsHash[4] ^ castlingRightsHash[8];

}

uint64_t Position::calculateHash()
{
	uint64_t h = 0;
	for (int i = A1; i <= H8; i++)
	{
		if (board[i] != Empty)
		{
			h ^= pieceHash[board[i]][i];
		}
	}
	if (enPassantSquare != 64)
	{
		h ^= enPassantHash[enPassantSquare];
	}
	if (sideToMove)
	{
		h ^= turnHash;
	}
	if (castlingRights)
	{
		h ^= castlingRightsHash[castlingRights];
	}

	return h;
}

uint64_t Position::calculatePawnHash()
{
	uint64_t p = 0;
	for (int i = A1; i <= H8; i++)
	{
		if (board[i] == WhitePawn || board[i] == BlackPawn)
		{
			p ^= pieceHash[board[i]][i];
		}
	}

	return p;
}

uint64_t Position::calculateMaterialHash()
{
	uint64_t material = 0;
	for (int i = WhitePawn; i <= BlackKing; i++)
	{
		for (int j = 0; j < popcnt(bitboards[i]); j++)
		{
			material ^= materialHash[i][j];
		}
	}
	return material;
}