#ifndef POSITION_HPP_
#define POSITION_HPP_

#include <array>
#include <string>
#include "bitboard.hpp"
#include "move.hpp"
#include "zobrist.hpp"
#include "color.hpp"
#include "piece.hpp"

class Position
{
public:
    Position();
    Position(const std::string& fen);

    std::string displayPositionAsString() const;

    Piece getBoard(Square sq) const { return board[sq]; }
    Bitboard getBitboard(Color colour, Piece piece) const { return bitboards[piece + colour * 6]; }
    Bitboard getPieces(Color colour) const { return bitboards[12 + colour]; }
    Bitboard getOccupiedSquares() const { return bitboards[12] | bitboards[13]; }
    Bitboard getFreeSquares() const { return ~getOccupiedSquares(); }
    Color getSideToMove() const { return sideToMove; }
    Square getEnPassantSquare() const { return enPassant; }
    int8_t getCastlingRights() const { return castlingRights; }
    int8_t getFiftyMoveDistance() const { return fiftyMoveDistance; }
    int8_t getGamePhase() const { return gamePhase; }
    Bitboard getPinnedPieces() const { return pinned; }
    Bitboard getDiscoveredCheckCandidates() const { return dcCandidates; }

    // Accesing hash keys.
    HashKey getHashKey() const { return hashKey; }
    HashKey getPawnHashKey() const { return pawnHashKey; }
    HashKey getMaterialHashKey() const { return materialHashKey; }

    // Getting information on the amounts of pieces.
    int8_t getPieceCount(Color color, Piece piece) const { return pieceCount[piece + color * 6]; }
    int8_t getTotalPieceCount() const { return totalPieceCount; }

    // Makes the move on the board.
    // We use copy-make so no unmake is necessary.
    void makeMove(const Move& move);

    bool inCheck() const { return isAttacked(Bitboards::lsb(getBitboard(sideToMove, Piece::King)), !sideToMove); };
    bool isAttacked(Square sq, Color side) const;

    // Checks if a move is legal with one caveat, doesn't work when in check and reports all moves as legal when in check.
    // This behaviour is dealt with the legal evasion generator.
    bool legal(const Move& move, bool inCheck) const;

    // Checks if a move gives check. 
    // Returns 0 if it isn't, 2 if the move is a discovered check and 1 if it is a normal check.
    int givesCheck(const Move& move) const;
private:
	// All bitboards needed to represent the position.
	// 6 bitboards for different white pieces + 1 for all white pieces.
	// 6 bitboards for different black pieces + 1 for all black pieces.
	// 1 for all occupied squares.
	std::array<Bitboard, 14> bitboards;
	// The board as a one-dimensional array.
	// We have it because often we want to know what piece is on which square or something like that.
	std::array<Piece, 64> board;

	// Miscellaneous, everything is pretty self explanatory.
    HashKey hashKey, pawnHashKey, materialHashKey;
    Bitboard pinned, dcCandidates;
	Color sideToMove;
    int8_t castlingRights;
    Square enPassant;
    int8_t fiftyMoveDistance;
    int8_t totalPieceCount;
    std::array<int8_t, 12> pieceCount;
    int8_t gamePhase;
    short gamePly;

	// These functions can be used to calculate different hash keys for the current position.
	// They are slow so they are only used when initializing, instead we update them incrementally.
    HashKey calculateHash() const;
    HashKey calculatePawnHash() const;
    HashKey calculateMaterialHash() const;

    template <bool side> 
    void makeMove(const Move& move);

    template <bool side> 
    bool isAttacked(Square sq) const;

    Bitboard discoveredCheckCandidates() const { return checkBlockers(sideToMove, !sideToMove); }
    Bitboard pinnedPieces(Color c) const { return checkBlockers(c, c); }
    Bitboard checkBlockers(Color c, Color kingColor) const;

    // Testing functions.
    bool verifyHashKeysAndPhase() const;
    bool verifyPieceCounts() const;
    bool verifyBoardAndBitboards() const;

    // Some stuff needed for making moves.
    static const std::array<int, 64> castlingMask;
    static const std::array<int8_t, 6> piecePhase;
    static const int8_t totalPhase;
};

#endif
