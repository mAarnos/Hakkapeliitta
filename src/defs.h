#ifndef DEFS_H
#define DEFS_H

#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <ctime>
#include <string>
#include <sstream>

using namespace std;

enum { 
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

enum {
	White = 0, Black = 1
};

enum {
	Pawn = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4, King = 5
};

enum {
	WhitePawn = 0, WhiteKnight = 1, WhiteBishop = 2, WhiteRook = 3, WhiteQueen = 4, WhiteKing = 5,
	BlackPawn = 6, BlackKnight = 7, BlackBishop = 8, BlackRook = 9, BlackQueen = 10, BlackKing = 11, Empty = 12
};

enum {
	whiteOO = 1, whiteOOO = 2, blackOO = 4, blackOOO = 8
};

const int Squares = NoSquare;


#endif
