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

#ifndef SQUARE_HPP_
#define SQUARE_HPP_

#include <cassert>
#include <cstdint>

// Represents a single square of the board.
class Square
{
public:
    Square() : square(NoSquare) {};
    Square(const int newSquare) { assert(newSquare >= A1 && newSquare <= NoSquare); square = static_cast<int8_t>(newSquare); };

    enum : int8_t
    {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
        NoSquare
    };

    operator int8_t() const { return square; }
    operator int8_t&() { return square; }
private:
    int8_t square;
};

// Checks if the square is okay, i.e. >= A1 and <= NoSquare. 
inline bool squareIsOkLoose(const Square sq)
{
    return (sq >= Square::A1 && sq <= Square::NoSquare);
}

// Checks if the square is on the board, i.e. >= A1 and <= H8. 
inline bool squareIsOkStrict(const Square sq)
{
    return (sq >= Square::A1 && sq <= Square::H8);
}

// Returns the number of the file the square is on.
inline int file(const Square sq)
{
    assert(squareIsOkStrict(sq));
    return (sq % 8);
}

// Returns the number of the rank the square is on.
inline int rank(const Square sq)
{
    assert(squareIsOkStrict(sq));
    return (sq / 8);
}

#endif