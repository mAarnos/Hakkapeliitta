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

/// @file history.hpp
/// @author Mikko Aarnos

#ifndef HISTORY_HPP_
#define HISTORY_HPP_

#include <array>
#include "move.hpp"
#include "position.hpp"

/// @brief A table for storing history heuristic information.
class HistoryTable
{
public:
    /// @brief Default constructor.
    HistoryTable();

    /// @brief Update the score of a move which SUCCEEDED in causing a beta cutoff in a confirmed CUT-node.
    /// @param pos The position.
    /// @param move The move.
    /// @param depth The depth.
    void addCutoff(const Position& pos, const Move& move, int depth);

    /// @brief Update the score of a move which FAILED to cause a beta cutoff in a confirmed CUT-node.
    /// @param pos The position.
    /// @param move The move.
    /// @param depth The depth.
    void addNotCutoff(const Position& pos, const Move& move, int depth);

    /// @brief Get the history score for a given move in a given position.
    /// @param pos The position.
    /// @param move The move.
    /// @return The score. We use 16 bits because a move is encoded as 16 bits and so we have a nice pair of 32 bits.
    int16_t getScore(const Position& pos, const Move& move) const;

    /// @brief Clears the history table.
    void clear();

    /// @brief Ages the history table so that scores for previous positions won't dominate.
    void age();

private:
    std::array<std::array<int, 64>, 12> mHistory;
    std::array<std::array<int, 64>, 12> mButterfly;
};

#endif
