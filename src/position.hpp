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

/// @file position.hpp
/// @author Mikko Aarnos

#ifndef POSITION_HPP_
#define POSITION_HPP_

#include <array>
#include <string>
#include "bitboards.hpp"
#include "move.hpp"
#include "zobrist.hpp"
#include "color.hpp"
#include "piece.hpp"

/// @brief Represents a single board position.
class Position
{
public:
    /// @brief Constructs a Position from a given FEN string. This WILL fail if it is called before Bitboards::staticInitialize.
    /// @param fen The FEN string.
    Position(const std::string& fen); 

    /// @brief Get the piece on a given square.
    /// @param sq The square.
    /// @return The piece. Can be empty as well.
    Piece getBoard(Square sq) const;

    /// @brief Get all pieces of a given type and color.
    /// @param color The color.
    /// @param piece The piece type.
    /// @return A bitboard containing all pieces of the given type and color.
    Bitboard getBitboard(Color color, Piece piece) const;

    /// @brief Get all pieces of a given color.
    /// @param color The color.
    /// @return A bitboard containing all pieces of a given color.
    Bitboard getPieces(Color color) const;

    /// @brief Get all squares which are occupied.
    /// @return A bitboard with every non-empty square marked.
    Bitboard getOccupiedSquares() const noexcept;

    /// @brief Get all squares which are not occupied.
    /// @return A bitboard with every empty square marked.
    Bitboard getFreeSquares() const noexcept;

    /// @brief Get the current side to move.
    /// @return The side to move. Should only be Color::White or Color::Black.
    Color getSideToMove() const noexcept;

    /// @brief Get the current en passant square.
    /// @return The en passant square. Can be Square::NoSquare in case en passant is not possible currently.
    Square getEnPassantSquare() const noexcept;

    /// @brief Get the castling rights of this position.
    /// @return The castling rights.
    int8_t getCastlingRights() const noexcept;

    /// @brief Get the current fifty move distance.
    /// @return The fifty move distance.
    int8_t getFiftyMoveDistance() const noexcept;

    /// @brief Get the current game phase, which is used for tapered eval.
    /// @return The game phase. 
    int8_t getGamePhase() const noexcept;

    /// @brief Get the pinned pieces in the current position.
    /// @return A bitboard containing all pinned pieces.
    Bitboard getPinnedPieces() const noexcept;

    /// @brief Get the pieces which could cause a discovered check if they move a certain way.
    /// @return A bitboard containing all discovered check candidates.
    Bitboard getDiscoveredCheckCandidates() const noexcept;

    /// @brief Get the locations of all rooks and queens.
    /// @return A bitboard with a bit marked for any rook or queen.
    Bitboard getRooksAndQueens() const noexcept;

    /// @brief Get the locations of all rooks and queens of a given color.
    /// @param c The color.
    /// @return A bitboard with a bit marked for any rook or queen of a given color.
    Bitboard getRooksAndQueens(Color c) const;

    /// @brief Get the locations of all bishops and queens.
    /// @return A bitboard with a bit marked for any bishop or queen.
    Bitboard getBishopsAndQueens() const noexcept;

    /// @brief Get the locations of all bishops and queens of a given color.
    /// @param c The color.
    /// @return A bitboard with a bit marked for any bishop or queen of a given color.
    Bitboard getBishopsAndQueens(Color c) const;

    /// @brief Get the normal hash key for this position. That is usually used by the TT.
    /// @return The hash key.
    HashKey getHashKey() const noexcept;

    /// @brief Get the pawn hash key for this position. That is usually used by the PHT.
    /// @return The pawn hash key.
    HashKey getPawnHashKey() const noexcept;

    /// @brief Get the material hash key for this position. That is used by the EndgameModule and Syzygy tablebases.
    /// @return The material hash key.
    HashKey getMaterialHashKey() const noexcept;

    /// @brief Get the count of pieces of a given type and color.
    /// @param color The color of the piece.
    /// @param piece The piece type.
    /// @return The count.
    int8_t getPieceCount(Color color, Piece piece) const;

    /// @brief Get the count of non-pawn pieces (excluding kings, there are always 2 of those) for a given color.
    /// @param color The color.
    /// @return The count.
    int8_t getNonPawnPieceCount(Color color) const;

    /// @brief Get the PST score for the opening phase.
    /// @return The score.
    int16_t getPstScoreOp() const noexcept;

    /// @brief Get the PST score for the ending phase.
    /// @return The score.
    int16_t getPstScoreEd() const noexcept;

    /// @brief Get the current ply of the game since the beginning.
    /// @return The ply.
    int16_t getGamePly() const noexcept;

    /// @brief Makes a given move on the board. We use copy-make so unmake is unnecessary.
    /// @param move The move.
    void makeMove(const Move& move);

    /// @brief Makes a null move. Unmake is unnecessary due to copy-make.
    void makeNullMove();

    /// @brief Checks if the current side to move is in check.
    /// @return True if the side to mvoe is in check, false otherwise.
    bool inCheck() const;

    /// @brief Checks if a given square is attacked by a given color.
    /// @param sq The square.
    /// @param color The color.
    /// @return True if the square is attacked, false otherwise.
    bool isAttacked(Square sq, Color color) const;

    /// @brief Checks if a given square is attacked by a given color with the given occupied squares. Used sometimes in MoveGen.
    /// @param sq The square.
    /// @param color The color.
    /// @param occupied The occupied squares.
    /// @return True if the square is attacked, false otherwise.
    bool isAttacked(Square sq, Color color, Bitboard occupied) const;

    /// @brief Checks if a given move is pseudo-legal in the current position. If we are in check this checks full legality.
    /// @param move The move.
    /// @param inCheck Whether the current position is in check or not.
    /// @return True if the move is pseudo-legal, false otherwise.
    bool pseudoLegal(const Move& move, bool inCheck) const;

    /// @brief Checks if a move is legal with one caveat: doesn't work when in check and reports all moves as legal when in check. This behaviour is dealt with the legal evasion generator.
    /// @param move The move.
    /// @param inCheck Whether the current position is in check or not.
    /// @return True if the move is legal, false otherwise.
    bool legal(const Move& move, bool inCheck) const;

    /// @brief Checks if a given move gives check.
    /// @param move The move.
    /// @return 0 if the move isn't a checking move, 1 if it is and 2 if it is a discovered check.
    int givesCheck(const Move& move) const;

    /// @brief Calculates the SEE score of a given move in the current position.
    /// @param move The move.
    /// @return The SEE score. The higher the better.
    int16_t SEE(const Move& move) const;

    /// @brief Calculates the MVV-LVA score of a given move in the current position.
    /// @param move The move.
    /// @return The MVV-LVA score. The higher the better.
    int16_t mvvLva(const Move& move) const;

    /// @brief Check if a given move is a capture or promotion.
    /// @param move The move.
    /// @return True if the move is a capture or a promotion, false otherwise.
    bool captureOrPromotion(const Move& move) const;

private:
    // All bitboards needed to represent the position.
    // 6 bitboards for different white pieces.
    // 6 bitboards for different black pieces.
    // Then 1 for all white pieces and 1 for all black pieces.
    std::array<Bitboard, 14> mBitboards;
    // The board as a one-dimensional array.
    // We have it because often we want to know what piece is on which square or something like that.
    std::array<Piece, 64> mBoard;

    // Miscellaneous, everything is pretty self explanatory.
    HashKey mHashKey, mPawnHashKey, mMaterialHashKey;
    Bitboard mPinned, mDcCandidates;
    Color mSideToMove;
    int8_t mCastlingRights;
    Square mEnPassant;
    int8_t mFiftyMoveDistance;
    std::array<int8_t, 12> mPieceCounts;
    std::array<int8_t, 2> mNonPawnPieceCounts;
    int8_t mGamePhase;
    int16_t mGamePly;
    int16_t mPstScoreOp, mPstScoreEd;

    void makeMove(const Move& move, bool side);

    template <bool side> 
    bool isAttacked(Square sq, Bitboard occupied) const;

    Bitboard discoveredCheckCandidates() const;
    Bitboard pinnedPieces(Color c) const;
    Bitboard checkBlockers(Color c, Color kingColor) const;

    // These functions can be used to calculate different hash keys for the current position.
    // They are slow so they are only used when initializing, instead we update them incrementally.
    HashKey calculateHash() const;
    HashKey calculatePawnHash() const;
    HashKey calculateMaterialHash() const;

    // Debugging functions.
    bool verifyPsts() const;
    bool verifyHashKeysAndPhase() const;
    bool verifyPieceCounts() const;
    bool verifyBoardAndBitboards() const;
};

inline Piece Position::getBoard(Square sq) const 
{ 
    return mBoard[sq]; 
}

inline Bitboard Position::getBitboard(Color colour, Piece piece) const 
{ 
    return mBitboards[piece + colour * 6]; 
}

inline Bitboard Position::getPieces(Color colour) const 
{ 
    return mBitboards[12 + colour]; 
}

inline Bitboard Position::getOccupiedSquares() const noexcept
{ 
    return mBitboards[12] | mBitboards[13]; 
}

inline Bitboard Position::getFreeSquares() const noexcept
{ 
    return ~getOccupiedSquares();
}

inline Color Position::getSideToMove() const noexcept
{ 
    return mSideToMove; 
}

inline Square Position::getEnPassantSquare() const noexcept
{ 
    return mEnPassant; 
}

inline int8_t Position::getCastlingRights() const noexcept
{ 
    return mCastlingRights; 
}

inline int8_t Position::getFiftyMoveDistance() const noexcept
{ 
    return mFiftyMoveDistance; 
}

inline int8_t Position::getGamePhase() const noexcept
{ 
    return mGamePhase; 
}

inline Bitboard Position::getPinnedPieces() const noexcept
{ 
    return mPinned; 
}

inline Bitboard Position::getDiscoveredCheckCandidates() const noexcept
{ 
    return mDcCandidates; 
}

inline HashKey Position::getHashKey() const noexcept
{ 
    return mHashKey;
}

inline HashKey Position::getPawnHashKey() const noexcept
{ 
    return mPawnHashKey; 
}

inline HashKey Position::getMaterialHashKey() const noexcept
{ 
    return mMaterialHashKey; 
}

inline int8_t Position::getPieceCount(Color color, Piece piece) const
{ 
    return mPieceCounts[piece + color * 6]; 
}

inline int8_t Position::getNonPawnPieceCount(Color side) const
{
    return mNonPawnPieceCounts[side];
}

inline int16_t Position::getPstScoreOp() const noexcept
{
    return mPstScoreOp;
}

inline int16_t Position::getPstScoreEd() const noexcept
{
    return mPstScoreEd;
}

inline int16_t Position::getGamePly() const noexcept
{
    return mGamePly;
}

inline bool Position::inCheck() const 
{ 
    return isAttacked(Bitboards::lsb(getBitboard(mSideToMove, Piece::King)), !mSideToMove); 
}

inline Bitboard Position::discoveredCheckCandidates() const 
{ 
    return checkBlockers(mSideToMove, !mSideToMove); 
}

inline Bitboard Position::getRooksAndQueens() const noexcept
{
    return mBitboards[Piece::WhiteQueen] | mBitboards[Piece::BlackQueen] | mBitboards[Piece::WhiteRook] | mBitboards[Piece::BlackRook];
}

inline Bitboard Position::getRooksAndQueens(Color c) const
{
    return getBitboard(c, Piece::Queen) | getBitboard(c, Piece::WhiteRook);
}

inline Bitboard Position::getBishopsAndQueens() const noexcept
{
    return mBitboards[Piece::WhiteQueen] | mBitboards[Piece::BlackQueen] | mBitboards[Piece::WhiteBishop] | mBitboards[Piece::BlackBishop];
}

inline Bitboard Position::getBishopsAndQueens(Color c) const
{
    return getBitboard(c, Piece::Queen) | getBitboard(c, Piece::Bishop);
}

inline Bitboard Position::pinnedPieces(Color c) const 
{
    return checkBlockers(c, c); 
}

#endif
