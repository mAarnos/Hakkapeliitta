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

/// @file square.hpp
/// @author Mikko Aarnos

#ifndef SQUARE_HPP_
#define SQUARE_HPP_

#include <cassert>
#include <cstdint>

/// @brief Represents a single square of the board.
///
/// The square is encoded as an int8_t, of which values 0 to 64 are legal and 0 to 63 are well-defined.
class Square
{
public:
    /// @brief Default constructor.
    Square() noexcept;

    /// @brief Constructs a Square from a given int. Should be int8_t but that causes a metric ton of warnings which require ugly fixes.
    /// @param square The int.
    Square(int square) noexcept;

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

    /// @return The int8_t.
    operator int8_t() const noexcept;
    /// @return A reference to the int8_t.
    operator int8_t&() noexcept;

    /// @brief Debugging function, used for checking if the square is well-defined.
    /// @return True if the square is well-defined, false otherwise.
    bool isOk() const noexcept;

private:
    int8_t mSquare;
};

inline Square::Square() noexcept : mSquare(NoSquare)
{
}

inline Square::Square(int square) noexcept : mSquare(static_cast<int8_t>(square))
{ 
}

inline Square::operator int8_t() const noexcept
{
    return mSquare;
}

inline Square::operator int8_t&() noexcept
{
    return mSquare;
}

inline bool Square::isOk() const noexcept
{
    return (mSquare >= Square::A1 && mSquare <= Square::H8);
}

/// @brief Used for calculating the file of a given square. Not inside the class above because we always don't have an instance of Square when we want to call this.
/// @param sq The square.
/// @return The file as a number, 0 for A-file, 1 for B-file and so on.
inline int file(Square sq) noexcept
{
    assert(sq.isOk());
    return (sq % 8);
}

/// @brief Used for calculating the rank of a given square. Here for the same reason as file.
/// @param sq The square.
/// @return The rank as a number, 0 for 1st rank, 1 for 2nd rank and so on.
inline int rank(Square sq) noexcept
{
    assert(sq.isOk());
    return (sq / 8);
}

#endif
