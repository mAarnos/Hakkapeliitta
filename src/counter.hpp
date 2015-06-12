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

/// @file counter.hpp
/// @author Mikko Aarnos

#ifndef COUNTER_HPP_
#define COUNTER_HPP_

#include <cstdint>
#include <array>
#include "position.hpp"
#include "move.hpp"

/// @brief A table for storing counter moves.
class CounterMoveTable
{
public:
    /// @brief Default constructor.
    CounterMoveTable();

    /// @brief Set move to be the counter move of opponentMove.
    /// @param pos The position.
    /// @param move The new counter move.
    /// @param opponentMove The opponent move which was countered.
    void update(const Position& pos, const Move& move, const Move& opponentMove);

    /// @brief Get the counter move of a given move.
    /// @param pos The position.
    /// @param move The move.
    /// @return The countermove.
    Move getCounterMove(const Position& pos, const Move& move) const;

    /// @brief Clears the counter move table.
    void clear();

private:
    std::array<std::array<Move, 64>, 12> mCounterMoves;
};

#endif
