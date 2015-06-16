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

/// @file move.hpp
/// @author Mikko Aarnos

#ifndef MOVE_HPP_
#define MOVE_HPP_

#include <cstdint>
#include <cassert>
#include "square.hpp"
#include "piece.hpp"

/// @brief Represents a single move. 
/// 
/// The move is packed into 16 bits for easy use of the TT. This raw value is sometimes manipulated as is, without using this class.
class Move
{
public:
    /// @brief Default constructor.
    Move() noexcept;

    /// @brief Constructs a move from a given raw move.
    /// @param rawMove The raw move.
    Move(uint16_t rawMove) noexcept;

    /// @brief Constructs a move from a given square to a given square with the given flags.
    /// @param from The from-square.
    /// @param to The to-square.
    /// @param flags The flags.
    Move(Square from, Square to, Piece flags) noexcept;

    /// @brief Gets the from-square of this move.
    /// @return The from-square.
    Square getFrom() const noexcept;

    /// @brief Gets the to-square of this move.
    /// @return The to-square.
    Square getTo() const noexcept;

    /// @brief Gets the flags of this move. Promotion, castling and en passant are notified with this.
    /// @return The flags.
    Piece getFlags() const noexcept;

    /// @brief Gets the raw move which is packed into 16 bits as stated previously.
    /// @return The raw move.
    uint16_t getRawMove() const noexcept;

    /// @brief Checks if the move contains no information (i.e. there is no raw move).
    /// @return True if the move is a dummy, false otherwise.
    bool empty() const noexcept;

    /// @brief Comparison operator for equality.
    /// @param m The move to compare against.
    /// @return True if the moves are the same, false otherwise.
    bool operator==(const Move& m) const noexcept;

    /// @brief Comparison operator for inequality.
    /// @param m The move to compare against.
    /// @return True if the moves are not the same, false otherwise.
    bool operator!= (const Move& m) const noexcept;

private:
    // 16 bits: 6 are used for the from-square, 6 are used for the to-square and 4 are used for different flags.
    uint16_t mRawMove;
};

inline Move::Move() noexcept : mRawMove(0)
{
}

inline Move::Move(uint16_t rawMove) noexcept : mRawMove(rawMove)
{
}

inline Move::Move(Square from, Square to, Piece flags) noexcept
{
    assert(from.isOk() && to.isOk() && flags.isOk());
    mRawMove = static_cast<uint16_t>(from) | (static_cast<uint16_t>(to) << 6) | (static_cast<uint16_t>(flags) << 12);
}

inline Square Move::getFrom() const noexcept
{ 
    return (mRawMove & 0x3f);
}

inline Square Move::getTo() const noexcept
{ 
    return ((mRawMove >> 6) & 0x3f);
}

inline Piece Move::getFlags() const noexcept
{ 
    return (mRawMove >> 12);
}

inline uint16_t Move::getRawMove() const noexcept
{ 
    return mRawMove;
}

inline bool Move::empty() const noexcept
{ 
    return !mRawMove;
}

inline bool Move::operator==(const Move& m) const noexcept
{
    return m.getRawMove() == mRawMove;
}

inline bool Move::operator!=(const Move& m) const noexcept
{
    return m.getRawMove() != mRawMove;
}

#endif
