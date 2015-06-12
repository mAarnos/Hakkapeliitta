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

/// @file piece.hpp
/// @author Mikko Aarnos

#ifndef PIECE_HPP_
#define PIECE_HPP_

#include <cassert>
#include <cstdint>

/// @brief Represents a single piece or piece type.
///
/// This piece or piecetype is encoded as a int8_t. For representing a piece, values from 0 to 13 are legal and values from 0 to 12 are well-defined.
/// For representing a piece type, values from 0 to 5 are legal and well-defined.
class Piece
{
public:
    /// @brief Default constructor.
    Piece() noexcept;

    /// @brief Constructs a Piece from a given int8_t. 
    /// @param piece The int8_t.
    Piece(int8_t piece) noexcept;

    enum : int8_t
    {
        Pawn = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4, King = 5
    };

    enum : int8_t
    {
        WhitePawn = 0, WhiteKnight = 1, WhiteBishop = 2, WhiteRook = 3, WhiteQueen = 4, WhiteKing = 5,
        BlackPawn = 6, BlackKnight = 7, BlackBishop = 8, BlackRook = 9, BlackQueen = 10, BlackKing = 11, 
        Empty = 12, NoPiece = 13
    };

    /// @return The int8_t.
    operator int8_t() const noexcept;
    /// @return A reference to the int8_t.
    operator int8_t&() noexcept;

    /// @brief Debugging function, used for checking if the piece is well-defined.
    /// @return True if the piece is well-defined, false otherwise.
    bool isOk() const noexcept;

    /// @brief Debugging function, used for checking if the piece type is well-defined. Should only be called if the given piece represents a piecetype.
    /// @return True if the piece type is well-defined, false otherwise.
    bool pieceTypeIsOk() const noexcept;

    /// @brief Debugging function, used for checking if the piece is actually representing a _real_ piece.
    /// @return True if the piece is not empty or not defined, false otherwise.
    bool canRepresentPieceType() const noexcept;

    /// @brief Gets the piece type of the given piece. We assume that this piece is representing a piece (not a piece type) at the moment.
    /// @return The piece type.
    Piece getPieceType() const noexcept;

private:
    int8_t mPiece;
};

inline Piece::Piece() noexcept : mPiece(NoPiece)
{
};

inline Piece::Piece(int8_t piece) noexcept : mPiece(piece)
{
};

inline Piece::operator int8_t() const noexcept
{ 
    return mPiece;
}

inline Piece::operator int8_t&() noexcept
{
    return mPiece;
}

inline bool Piece::isOk() const noexcept
{
    return (mPiece >= Piece::WhitePawn && mPiece <= Piece::Empty);
}

inline bool Piece::canRepresentPieceType() const noexcept
{
    return (mPiece >= Piece::WhitePawn && mPiece <= Piece::BlackKing);
}

inline bool Piece::pieceTypeIsOk() const noexcept
{
    return (mPiece >= Piece::Pawn && mPiece <= Piece::King);
}

inline Piece Piece::getPieceType() const noexcept
{
    assert(canRepresentPieceType());
    return (mPiece % 6);
}

#endif
