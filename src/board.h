#ifndef BOARD_H
#define BOARD_H

#include "defs.h"
#include <string>

using namespace std;

extern U64 whiteKing;
extern U64 whiteQueens;
extern U64 whiteRooks;
extern U64 whiteBishops;
extern U64 whiteKnights;
extern U64 whitePawns;
extern U64 blackKing;
extern U64 blackQueens; 
extern U64 blackRooks;
extern U64 blackBishops;
extern U64 blackKnights;
extern U64 blackPawns;
extern U64 whitePieces;
extern U64 blackPieces;
extern U64 occupiedSquares;

// amounts of white and black pieces on the board, updated incrementally
extern int wp; 
extern int bp;
extern int wn;
extern int bn;
extern int wb;
extern int bb;
extern int wr;
extern int br;
extern int wq;
extern int bq;
 
extern bool sideToMove; // white = false, black = true 
extern int castling; // castling status; if 1 is set white can castle kingside, if 2 is set white can castle queenside, if 4 is set black can castle kingside, if 8 is set black can castle queenside
extern int enPassant; // en passant target square after double pawn move
extern int fiftyMoveDistance; // moves since the last pawn move or capture, if this reaches 100 game is drawn

extern int ply;
extern int hply;

extern U64 Hash;
extern U64 pHash;

void initializeBoard();
void initializeFromSquares(int input[64], bool side, int castle, int ep, int fifty);
int initializeBoardFromFen(string aFEN);

int algebraicMoves(char * a);

#endif