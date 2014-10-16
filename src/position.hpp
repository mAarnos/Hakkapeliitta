#ifndef POSITION_HPP_
#define POSITION_HPP_

#include <array>
#include <string>
#include "bitboard.hpp"
#include "move.hpp"
#include "zobrist.hpp"
#include "color.hpp"
#include "piece.hpp"

class History
{
public:
    HashKey hash;
    HashKey pHash;  
    Piece captured;
    Square ep;
    int fifty;
    int castle;
};

class Position
{
public:
    Position();

	void initializePositionFromFen(const std::string & fen);
    std::string positionToFen() const;
	void displayBoard() const;

    Piece getBoard(Square sq) const { return board[sq]; }

    Bitboard getBitboard(Color colour, Piece piece) const { return bitboards[piece + colour * 6]; }
    Bitboard getPieces(Color colour) const { return bitboards[12 + colour]; }
    Bitboard getOccupiedSquares() const { return bitboards[14]; }
    Bitboard getFreeSquares() const { return ~bitboards[14]; }

    Color getSideToMove() const { return sideToMove; }
    Square getEnPassantSquare() const { return enPassant; }
    int getCastlingRights() const { return castlingRights; }
    int getFiftyMoveDistance() const { return fiftyMoveDistance; }

    HashKey getHashKey() const { return hashKey; }
    HashKey getPawnHashKey() const { return pawnHashKey; }
    HashKey getMaterialHashKey() const { return materialHashKey; }

    bool makeMove(const Move & move, History & history);
    void unmakeMove(const Move & move, const History & history);
    void makeNullMove(History & history);
    void unmakeNullMove(const History & history);

    int getPieceCount(Color color, Piece piece) const { return pieceCount[piece + color * 6]; }

    bool inCheck() const { return isAttacked(Bitboards::lsb(getBitboard(sideToMove, Piece::King)), !sideToMove); };
    bool isAttacked(Square sq, Color side) const;

    int calculateGamePhase() const { return (phase * 256 + (totalPhase / 2)) / totalPhase; }
    int getRawGamePhase() const { return phase; }

    int getPstMaterialScoreOpening() const { return pstMaterialScoreOp; }
    int getPstMaterialScoreEnding() const { return pstMaterialScoreEd; }

    int SEE(const Move & m) const;
private:
	// All bitboards needed to represent the position.
	// 6 bitboards for different white pieces + 1 for all white pieces.
	// 6 bitboards for different black pieces + 1 for all black pieces.
	// 1 for all occupied squares.
	std::array<Bitboard, 15> bitboards;
	// The board as a one-dimensional array.
	// We have it because often we want to know what piece is on which square or something like that.
	std::array<Piece, 64> board;

	// Miscellaneous, everything is pretty self explanatory.
    HashKey hashKey, pawnHashKey, materialHashKey;
	Color sideToMove;
    int castlingRights;
    Square enPassant;
	int fiftyMoveDistance;
    int ply;
    int phase;
    int pstMaterialScoreOp, pstMaterialScoreEd;
    std::array<int, 12> pieceCount;

	// These functions can be used to calculate different hash keys for the current position.
	// They are slow so they are only used when initializing, instead we update them incrementally.
    HashKey calculateHash() const;
    HashKey calculatePawnHash() const;
    HashKey calculateMaterialHash() const;

    template <bool side> bool makeMove(const Move & move, History & history);
    template <bool side> void unmakeMove(const Move & move, const History & history);
    template <bool side> bool isAttacked(Square sq) const;

    static const std::array<int, 64> castlingMask;
    static const std::array<int, 6> piecePhase;
    static const int totalPhase;
};

#endif
