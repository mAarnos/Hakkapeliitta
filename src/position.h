#ifndef POSITION_H
#define POSITION_H

#include "defs.h"
#include "bitboard.h"
#include "move.h"
#include "hash.h"

const int maxGameLength = 600;

class History
{
	public:
		int castle;
		int ep;
		int fifty;
		int captured;
		uint64_t hash;
		uint64_t pHash;
		uint64_t mHash;
};

class Position
{
	public:
		void initializeBoardFromFEN(string FEN);
		void displayBoard();

		inline bool inCheck(bool side) { return attack(bitScanForward(bitboards[King + side * 6]), !side); }
		inline bool isAttacked(int sq, bool side) { return attack(sq, side); }

		inline int getBoard(int sq) { return board[sq]; }
		inline int getBoardPieceType(int sq) { return (board[sq] % Pieces); }

		inline uint64_t getBitboard(bool colour, int piece) { return bitboards[piece + colour * 6]; }
		inline uint64_t getPieces(bool colour) { return bitboards[12 + colour]; }
		inline uint64_t getOccupiedSquares() { return bitboards[14]; }
		inline uint64_t getFreeSquares() { return bitboards[15]; }

		inline uint64_t getHash() { return hash; }
		inline uint64_t getPawnHash() { return pawnHash; }
		inline uint64_t getMaterialHash() { return matHash; }
		
		inline bool getSideToMove() { return sideToMove; }
		inline int getTurn() { return hply; }
		inline int getEnPassantSquare() { return enPassantSquare; }
		inline int getCastlingRights() { return castlingRights; }

		bool makeMove(Move m);
		void unmakeMove(Move m);
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

		// Keeps track of the irreversible things in the gamestate.
		History historyStack[maxGameLength];

		// Miscellaneous, everything is pretty self explanatory.
		bool sideToMove;
		int castlingRights;
		int enPassantSquare;
		int fiftyMoveDistance;
		int hply;
		int phase;
		uint64_t hash, pawnHash, matHash;

		// These functions can be used to calculate different hash keys for the current position.
		// They are slow so they are only used when initializing, instead we update them incrementally.
		uint64_t calculateHash();
		uint64_t calculatePawnHash();
		uint64_t calculateMaterialHash();

		// Miscellaneous functions used by the program.
		bool attack(int sq, bool side);
		void makeCapture(int captured, int to);
		void unmakeCapture(int captured, int to);
		void makePromotion(int promotion, int to);
		void unmakePromotion(int promotion, int to);
		void makeEnPassant(int to);
		void unmakeEnPassant(int to);
};

extern Position root;

#endif
