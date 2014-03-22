#ifndef POSITION_H
#define POSITION_H

#include "defs.h"

class Position
{
	public:
		void initializeBoardFromFEN(string FEN);
	private:
		// All bitboards needed to represent the position.
		// 6 bitboards for different white pieces + 1 for all white pieces.
		// 6 bitboards for different black pieces + 1 for all black pieces.
		// 1 for all occupied squares.
		// 1 for all not-occupied squares.
		array<uint64_t, 16> bitboards;
		// Miscellaneous, everything is pretty self explanatory.
		bool sideToMove;
		int castlingRights;
		int enPassantSquare;
		int fiftyMoveDistance;
		int ply, hply;
		uint64_t hash, pawnHash;

		int initBoardFromFEN(string FEN);
};

#endif
