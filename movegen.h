#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"
#include "Move.h"
#include "board.h"

typedef struct 
{
	Move m;
	unsigned int castle;
	unsigned int ep;
	unsigned int fifty;
	U64 Hash;
	U64 pHash;
} history;	

extern Move moveStack[10000];
extern int moveList[600];

extern history historyStack[600];

extern U64 inCheck(bool s);
extern bool make(int mov);
extern void generateMoves();
extern void generateCaptures();
extern void generateEvasions();
extern void unmake(int mov);
extern void initializeCastlingMoveInts();

extern int whiteShortCastle, whiteLongCastle, blackShortCastle, blackLongCastle;

#endif
