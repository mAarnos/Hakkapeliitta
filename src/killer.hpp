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

/// @file killer.hpp
/// @author Mikko Aarnos

#ifndef KILLER_HPP_
#define KILLER_HPP_

#include <cstdint>
#include <array>
#include "move.hpp"
#include "constants.hpp"

/// @brief A table for storing killer moves.
class KillerTable
{
public:
    /// @brief Default constructor.
    KillerTable();

    /// @brief Updates the killer moves for a given ply. Assumes that the new killer move is not a capture or a promotion.
    /// @param move The new killer move.
    /// @param ply The ply.
    void update(const Move& move, int ply);

    /// @brief Get the killer moves for a given ply.
    /// @param ply The ply.
    /// @return A pair of killer moves, the first one being the more recent one.
    std::pair<Move, Move> getKillers(int ply) const;

    /// @brief Clears the entire killer table.
    void clear();

    /// @brief Clears a given ply of the killer table.
    /// @param ply The ply.
    void clear(int ply);

private:
    std::array<std::array<Move, 2>, maxPly> mKillers;
};

#endif
