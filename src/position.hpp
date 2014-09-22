#ifndef POSITION_H_
#define POSITION_H_

#include "defs.hpp"
#include "bitboard.hpp"
#include "move.hpp"
#include "hash.hpp"

// The maxinum supported game length in plies.
const int maxGameLength = 1200;

const array<int, Pieces> piecePhase = {
	0, 1, 1, 2, 4, 0
};
const int totalPhase = piecePhase[Pawn] * 16 + piecePhase[Knight] * 4 + piecePhase[Bishop] * 4 + piecePhase[Rook] * 4 + piecePhase[Queen] * 2;

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
        std::string boardToFen() const;
		void displayBoard();

		bool inCheck() { return isAttacked(bitScanForward(bitboards[King + sideToMove * 6]), !sideToMove); }
		bool isAttacked(int sq, bool side);

		// The first one returns all attacks to a square, the second one retuns all attacks by the side specified to a square.
		uint64_t attacksTo(int to);
		uint64_t attacksTo(int to, bool side);

		// Hack, this shouldn't exist but I haven't figured out a way to do generateEvasions without it.
		inline void replaceBitboard(uint64_t newBB, int bbNumber) { bitboards[bbNumber] = newBB; }

		inline int getPiece(int sq) { return board[sq]; }
		// Returns incorrect piecetype if there is nothing on the square. Use only if you known something is on the square specified.
		inline int getPieceType(int sq) { return (board[sq] % Pieces); }

		inline uint64_t getBitboard(bool colour, int piece) { return bitboards[piece + colour * 6]; }
		inline uint64_t getPieces(bool colour) { return bitboards[12 + colour]; }
		inline uint64_t getOccupiedSquares() { return bitboards[14]; }
		inline uint64_t getFreeSquares() { return ~bitboards[14]; }

		inline uint64_t getHash() const { return hash; }
		inline uint64_t getPawnHash() const { return pawnHash; }
		inline uint64_t getMaterialHash() const { return matHash; }
		
		inline bool getSideToMove() { return sideToMove; }
		inline int getEnPassantSquare() { return enPassantSquare; }
		inline int getCastlingRights() { return castlingRights; }
		inline int getFiftyMoveDistance() { return fiftyMoveDistance; }

		inline int getScoreOp() { return scoreOp; }
		inline int getScoreEd() { return scoreEd; }

		inline int calculateGamePhase() { return (phase * 256 + (totalPhase / 2)) / totalPhase; }

		inline int getKiller(int killerNumber, int ply) { return killer[killerNumber][ply]; }
		inline void setKiller(int killerNumber, int ply, int newKiller) { killer[killerNumber][ply] = newKiller; }
        inline void resetKillers() { memset(killer, 0, sizeof(killer)); }
		inline bool getIsInCheck(int ply) { return isInCheck[ply]; }
		inline void setIsInCheck(int ply, bool inCheck) { isInCheck[ply] = inCheck; }

		bool repetitionDraw();

		int SEE(Move m);

		bool makeMove(Move m);
		void unmakeMove(Move m);

		void makeNullMove();
		void unmakeNullMove();
	private:
		// All bitboards needed to represent the position.
		// 6 bitboards for different white pieces + 1 for all white pieces.
		// 6 bitboards for different black pieces + 1 for all black pieces.
		// 1 for all occupied squares.
		array<uint64_t, 15> bitboards;
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
		int scoreOp, scoreEd;
		uint64_t hash, pawnHash, matHash; 

		array<bool, maxGameLength> isInCheck;

		// Killer moves. Since these are highly position specific we have to make them thread specific.
		array<int, maxGameLength> killer[2];

		// These functions can be used to calculate different hash keys for the current position.
		// They are slow so they are only used when initializing, instead we update them incrementally.
		uint64_t calculateHash();
		uint64_t calculatePawnHash();
		uint64_t calculateMaterialHash();

		// Miscellaneous functions used by the program.
		void makeCapture(int captured, int to);
		void makePromotion(int promotion, int to);	
		void makeEnPassant(int to);
		void makeCastling(int from, int to);
		void unmakeCapture(int captured, int to);
		void unmakePromotion(int promotion, int to);
		void unmakeEnPassant(int to);
		void unmakeCastling(int from, int to);

		void writeHistory(int & captured);
        void readHistory(int & captured);
};

extern Position root;

#endif
