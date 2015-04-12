/*
    Hakkapeliitta - A UCI chess engine. Copyright (C) 2013-2015 Mikko Aarnos.

    Hakkapeliitta is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Hakkapeliitta is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Hakkapeliitta. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef POSITION_HPP_
#define POSITION_HPP_

#include <array>
#include <string>
#include "bitboard.hpp"
#include "move.hpp"
#include "zobrist.hpp"
#include "color.hpp"
#include "piece.hpp"

// Represents a single board position. 
class Position
{
public:
    // Constructs an invalid position, not the starting position.
    Position();
    Position(const std::string& fen); 

    // Initializes a position from a FEN string.
    // Will throw an exception if the FEN string is corrupted.
    void initializePositionFromFen(const std::string& fen);
    // For displaying the position in console.
    std::string displayPositionAsString() const;
    // For turning a position into a FEN string.
    std::string toFen() const;

    // Accessing different kinds of board data.
    Piece getBoard(Square sq) const;
    Bitboard getBitboard(Color colour, Piece piece) const;
    Bitboard getPieces(Color colour) const;
    Bitboard getOccupiedSquares() const;
    Bitboard getFreeSquares() const;
    Color getSideToMove() const;
    Square getEnPassantSquare() const;
    int8_t getCastlingRights() const;
    int8_t getFiftyMoveDistance() const;
    int8_t getGamePhase() const;
    Bitboard getPinnedPieces() const;
    Bitboard getDiscoveredCheckCandidates() const;
    HashKey getHashKey() const;
    HashKey getPawnHashKey() const;
    HashKey getMaterialHashKey() const;
    int8_t getPieceCount(Color color, Piece piece) const;
    int8_t getTotalPieceCount() const;
    int16_t getPstScoreOp() const;
    int16_t getPstScoreEd() const;

    // Makes the move on the board.
    // We use copy-make so unmake is unnecessary.
    void makeMove(const Move& move);

    // Makes a null move.
    // Unmake is unnecessary due to copy-make.
    void makeNullMove();

    bool inCheck() const;
    bool isAttacked(Square sq, Color side) const;
    bool isAttacked(Square sq, Color side, Bitboard occupied) const;

    // Checks if a random move is pseudo-legal in the current position.
    bool pseudoLegal(const Move& move, bool inCheck) const;

    // Checks if a move is legal with one caveat, doesn't work when in check and reports all moves as legal when in check.
    // This behaviour is dealt with the legal evasion generator.
    bool legal(const Move& move, bool inCheck) const;

    // Checks if a move gives check. 
    // Returns 0 if it isn't, 2 if the move is a discovered check and 1 if it is a normal check.
    int givesCheck(const Move& move) const;

    // Calculate the SEE score of a move for the current position.
    int16_t SEE(const Move& move) const;

    // Calculate the MVV-LVA score of a move.
    int16_t mvvLva(const Move& move) const;

    // Check if the given move is a capture or promotion.
    bool captureOrPromotion(const Move& move) const;
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
    int16_t gamePly;
    int16_t pstScoreOp;
    int16_t pstScoreEd;

	// These functions can be used to calculate different hash keys for the current position.
	// They are slow so they are only used when initializing, instead we update them incrementally.
    HashKey calculateHash() const;
    HashKey calculatePawnHash() const;
    HashKey calculateMaterialHash() const;

    template <bool side> 
    void makeMove(const Move& move);

    template <bool side> 
    bool isAttacked(Square sq, Bitboard occupied) const;

    Bitboard discoveredCheckCandidates() const;
    Bitboard pinnedPieces(Color c) const;
    Bitboard checkBlockers(Color c, Color kingColor) const;

    // Testing functions.
    bool verifyPsts() const;
    bool verifyHashKeysAndPhase() const;
    bool verifyPieceCounts() const;
    bool verifyBoardAndBitboards() const;
};

inline Piece Position::getBoard(const Square sq) const 
{ 
    return board[sq]; 
}

inline Bitboard Position::getBitboard(const Color colour, const Piece piece) const 
{ 
    return bitboards[piece + colour * 6]; 
}

inline Bitboard Position::getPieces(const Color colour) const 
{ 
    return bitboards[12 + colour]; 
}

inline Bitboard Position::getOccupiedSquares() const
{ 
    return bitboards[12] | bitboards[13]; 
}

inline Bitboard Position::getFreeSquares() const 
{ 
    return ~getOccupiedSquares();
}

inline Color Position::getSideToMove() const 
{ 
    return sideToMove; 
}

inline Square Position::getEnPassantSquare() const
{ 
    return enPassant; 
}

inline int8_t Position::getCastlingRights() const 
{ 
    return castlingRights; 
}

inline int8_t Position::getFiftyMoveDistance() const 
{ 
    return fiftyMoveDistance; 
}

inline int8_t Position::getGamePhase() const 
{ 
    return gamePhase; 
}

inline Bitboard Position::getPinnedPieces() const 
{ 
    return pinned; 
}

inline Bitboard Position::getDiscoveredCheckCandidates() const 
{ 
    return dcCandidates; 
}

inline HashKey Position::getHashKey() const 
{ 
    return hashKey; 
}

inline HashKey Position::getPawnHashKey() const 
{ 
    return pawnHashKey; 
}

inline HashKey Position::getMaterialHashKey() const 
{ 
    return materialHashKey; 
}

inline int8_t Position::getPieceCount(const Color color, const Piece piece) const
{ 
    return pieceCount[piece + color * 6]; 
}

inline int8_t Position::getTotalPieceCount() const
{
    return totalPieceCount; 
}

inline int16_t Position::getPstScoreOp() const
{
    return pstScoreOp;
}

inline int16_t Position::getPstScoreEd() const
{
    return pstScoreEd;
}

inline bool Position::inCheck() const 
{ 
    return isAttacked(Bitboards::lsb(getBitboard(sideToMove, Piece::King)), !sideToMove); 
}

inline Bitboard Position::discoveredCheckCandidates() const 
{ 
    return checkBlockers(sideToMove, !sideToMove); 
}

inline Bitboard Position::pinnedPieces(const Color c) const 
{
    return checkBlockers(c, c); 
}

#endif
