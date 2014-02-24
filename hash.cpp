#ifndef HASH_CPP
#define HASH_CPP

#include "hash.h"
#include "board.h"
#include "bitboard.h"

boost::random::mt19937_64 rng;

U64 pieceHash[16][64];
U64 materialHash[2][6][8];
U64 side;
U64 ep[64];
U64 castle[16];

void initializeHash()
{
	int i, j;

	rng.seed(0);

	for (i = 0; i < 64; i++)
    {
		ep[i] = random64(rng); // hash for en passant on square i
		for (j = 0; j < 16; j++) 
		{
			pieceHash[j][i] = random64(rng); // hash for every possible piece on square i
		}
    }

	castle[1] = random64(rng); // hash for white O-O
	castle[2] = random64(rng); // hash for white O-O-O
	castle[3] = castle[1] ^ castle[2];
	castle[4] = random64(rng); // hash for black O-O
	castle[5] = castle[1] ^ castle[4];
	castle[6] = castle[2] ^ castle[4];
	castle[7] = castle[1] ^ castle[2] ^ castle[4]; 
	castle[8] = random64(rng); // hash for black O-O-O
	castle[9] = castle[1] ^ castle[8];
	castle[10] = castle[2] ^ castle[8];
	castle[11] = castle[1] ^ castle[2] ^ castle[8];
	castle[12] = castle[4] ^ castle[8];
	castle[13] = castle[1] ^ castle[4] ^ castle[8];
	castle[14] = castle[2] ^ castle[4] ^ castle[8];
	castle[15] = castle[1] ^ castle[2] ^ castle[4] ^ castle[8];

	side = random64(rng); // hash for sideToMove = Black

	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 6; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				materialHash[i][j][k] = random64(rng);
			}
		}
	}
	
}

// used when setting up the board, turns the initial position into a nearly-unique hash
// we could use this function every time we want to update the hash, but that is slow so we update it incrementally in make/unmake
U64 setHash() 
{
    int i;
	U64 Hash;
	U64 wPawns     = whitePawns;
	U64	wKnights   = whiteKnights;
	U64	wBishops   = whiteBishops;
	U64	wRooks     = whiteRooks;
	U64	wQueens    = whiteQueens;
	U64 bPawns     = blackPawns;
	U64	bKnights   = blackKnights;
	U64	bBishops   = blackBishops;
	U64	bRooks     = blackRooks;
	U64	bQueens    = blackQueens;
    
	Hash = 0;
	
    while(wPawns) 
	{ 
		i = bitScanForward(wPawns);
        wPawns ^= setMask[i];
		Hash ^= pieceHash[WhitePawn][i];
	}
	while(wKnights) 
	{ 
		i = bitScanForward(wKnights);
        wKnights ^= setMask[i];
		Hash ^= pieceHash[WhiteKnight][i];
	}
	while(wBishops) 
	{ 
		i = bitScanForward(wBishops);
        wBishops ^= setMask[i];
		Hash ^= pieceHash[WhiteBishop][i];
	}
	while(wRooks) 
	{ 
		i = bitScanForward(wRooks);
        wRooks ^= setMask[i];
		Hash ^= pieceHash[WhiteRook][i];
	}
	while(wQueens) 
	{ 
		i = bitScanForward(wQueens);
        wQueens ^= setMask[i];
		Hash ^= pieceHash[WhiteQueen][i];
	}
	i = bitScanForward(whiteKing);
	Hash ^= pieceHash[WhiteKing][i];

	while(bPawns) 
	{ 
		i = bitScanForward(bPawns);
        bPawns ^= setMask[i];
		Hash ^= pieceHash[BlackPawn][i];
	}
	while(bKnights) 
	{ 
		i = bitScanForward(bKnights);
        bKnights ^= setMask[i];
		Hash ^= pieceHash[BlackKnight][i];
	}
	while(bBishops) 
	{ 
		i = bitScanForward(bBishops);
        bBishops ^= setMask[i];
		Hash ^= pieceHash[BlackBishop][i];
	}
	while(bRooks) 
	{ 
		i = bitScanForward(bRooks);
        bRooks ^= setMask[i];
		Hash ^= pieceHash[BlackRook][i];
	}
	while(bQueens) 
	{ 
		i = bitScanForward(bQueens);
        bQueens ^= setMask[i];
		Hash ^= pieceHash[BlackQueen][i];
	}
	
	i = bitScanForward(blackKing);
	Hash ^= pieceHash[BlackKing][i];
	
	if (sideToMove == Black)
	{
		Hash ^= side;
	}
	if (castling)
	{
		Hash ^= castle[castling];
	}
	if (enPassant != 64)
	{
		Hash ^= ep[enPassant];
	}
	return Hash;
}

U64 setPHash()
{
	int i;
	U64 Hash = 0;
	U64 wPawns = whitePawns;
	U64 bPawns = blackPawns;
	
    while(wPawns) 
	{ 
		i = bitScanForward(wPawns);
		Hash ^= pieceHash[WhitePawn][i];
		wPawns ^= setMask[i];
	}
	while(bPawns) 
	{ 
		i = bitScanForward(bPawns);
		Hash ^= pieceHash[BlackPawn][i];
		bPawns ^= setMask[i];
	}

	return Hash;
}

#endif