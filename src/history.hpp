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

#ifndef HISTORY_HPP_
#define HISTORY_HPP_

#include <array>
#include "move.hpp"
#include "position.hpp"

// History heuristic for the search function encapsulated.
class HistoryTable
{
public:
    HistoryTable();

    // Change the score of a move which caused a beta-cutoff at the given depth.
    void addCutoff(const Position& pos, const Move& move, int depth);
    // Change the score of a move which failed to cause a beta-cutoff at the given depth when some other move did cause one.
    void addNotCutoff(const Position& pos, const Move& move, int depth);
    // Get the history score for the given move.
    int getScore(const Position& pos, const Move& move) const;
    // Clears the table.
    void clear();
    // From time to time we should age the table to make sure that scores for previous positions won't dominate the scores.
    void age();
private:
    std::array<std::array<int, 64>, 12> history;
    std::array<std::array<int, 64>, 12> butterfly;
};

#endif
