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

#ifndef PIECE_HPP_
#define PIECE_HPP_

#include <cassert>
#include <cstdint>

// Represents a single piece or piecetype.
class Piece
{
public:
    Piece() : piece(NoPiece) {};
    Piece(const int8_t newPiece) : piece(newPiece) {};

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

    operator int8_t() const { return piece; }
    operator int8_t&() { return piece; }
private:
    int8_t piece;
};

// Checks if the piece is ok, i.e. >= WhitePawn and <= Empty. 
inline bool pieceIsOk(const Piece p)
{
    return (p >= Piece::WhitePawn && p <= Piece::Empty);
}

// Checks if the piece type is ok. Should only be called if the given piece represents a piecetype.
inline bool pieceTypeIsOk(const Piece p)
{
    return (p >= Piece::Pawn && p <= Piece::King);
}

// Gets the piece type of the given piece. We assume that p is representing a piece at the moment.
inline Piece getPieceType(const Piece p)
{
    assert(pieceIsOk(p) && p != Piece::Empty);
    return (p % 6);
}

#endif