#ifndef DEFS_H
#define DEFS_H

#include <iostream>
#include <assert.h>
#include <cstring>
#include <stdio.h>

typedef unsigned long long U64;
typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;

enum { // A1 = 0, B1 = 1 and so on.
   A1, B1, C1, D1, E1, F1, G1, H1,
   A2, B2, C2, D2, E2, F2, G2, H2,
   A3, B3, C3, D3, E3, F3, G3, H3,
   A4, B4, C4, D4, E4, F4, G4, H4,
   A5, B5, C5, D5, E5, F5, G5, H5,
   A6, B6, C6, D6, E6, F6, G6, H6,
   A7, B7, C7, D7, E7, F7, G7, H7,
   A8, B8, C8, D8, E8, F8, G8, H8,
   NoSquare
};
const int Squares = NoSquare;

enum { Rank1, Rank2, Rank3, Rank4, Rank5, Rank6, Rank7, Rank8, NoRank }; // Rank1 = 0, Rank2 = 1 and so on.
const int Ranks = NoRank;

enum { FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH, NoFile }; // FileA = 0, FileB = 1 and so on.
const int Files = NoFile;

enum { White, Black }; // White = 0, Black = 1.
const int Colour = Black;

const int Empty = 0;               //  0000
const int WhitePawn = 1;           //  0001
const int WhiteKing = 2;           //  0010
const int WhiteKnight = 3;         //  0011
const int WhiteBishop =  5;        //  0101
const int WhiteRook = 6;           //  0110
const int WhiteQueen = 7;          //  0111
const int BlackPawn = 9;           //  1001
const int BlackKing = 10;          //  1010
const int BlackKnight = 11;        //  1011
const int BlackBishop = 13;        //  1101
const int BlackRook = 14;          //  1110
const int BlackQueen = 15;         //  1111

const int Infinity = 2147483647;
const int HashMove = Infinity >> 1;
const int CaptureMove = Infinity >> 2;
const int KillerMove1 = Infinity >> 3;
const int KillerMove2 = Infinity >> 4;
const int KillerMove3 = Infinity >> 5;
const int KillerMove4 = Infinity >> 6;

// checks if the intersection of r and f is on board
inline bool isPositionOnBoard(int r, int f) { 
   return Rank1 <= r && r < NoRank && FileA <= f && f < NoFile;
}

// returns the number of the square when given the rank and file
inline int Square(int r, int f) { 
   return 8 * r + f;
}

// Returns the rank when given the square.
inline int Rank(int sq) { 
   return sq / 8;
}

// Returns the file when given the square.
inline int File(int sq) { 
   return sq % 8;
}

const int onePly = 2;

#endif
