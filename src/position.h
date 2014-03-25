#ifndef POSITION_H
#define POSITION_H

#include "defs.h"
#include "bitboard.h"
#include "move.h"

class History
{
	public:
		int castle;
		int ep;
		int fifty;
		int captured;
		uint64_t hash;
		uint64_t pHash;
};

class Position
{
	public:
		void initializeBoardFromFEN(string FEN);

		bool inCheck(bool side);

		inline int getPieceType(int sq) { return board[sq]; }
		inline uint64_t getBitboard(bool colour, int piece) { return bitboards[piece + colour * 6]; }
		inline uint64_t getPieces(bool colour) { return bitboards[12 + colour]; }
		inline uint64_t getOccupiedSquares() { return bitboards[14]; }
		inline uint64_t getFreeSquares() { return bitboards[15]; }
		
		inline bool getSideToMove() { return sideToMove; }
		inline int getEnPassantSquare() { return enPassantSquare; }
		inline int getCastlingRights() { return castlingRights; }

		bool makeMove(Move m);
		void unmakeMove(Move m);

		// Displays the board. Used for debugging.
		void displayBoard();

	private:
		// All bitboards needed to represent the position.
		// 6 bitboards for different white pieces + 1 for all white pieces.
		// 6 bitboards for different black pieces + 1 for all black pieces.
		// 1 for all occupied squares.
		// 1 for all not-occupied squares.
		array<uint64_t, 16> bitboards;
		// The board as a one-dimensional array.
		// We have it because often we want to know what piece is on which square or something like that.
		array<int, Squares> board;

		History historyStack[600];

		// Miscellaneous, everything is pretty self explanatory.
		bool sideToMove;
		int castlingRights;
		int enPassantSquare;
		int fiftyMoveDistance;
		int hply;
		uint64_t hash, pawnHash;

		// Miscellaneous functions used by the program.
		bool attack(int sq, bool side);
		void makeCapture(int captured, int to);
		void unmakeCapture(int captured, int to);
		void makePromotion(int promotion, int to);
		void unmakePromotion(int promotion, int to);
};

#endif
