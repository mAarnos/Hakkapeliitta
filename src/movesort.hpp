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

/// @file movesort.hpp
/// @author Mikko Aarnos

#ifndef MOVESORT_HPP_
#define MOVESORT_HPP_

#include "movelist.hpp"
#include "position.hpp"
#include "history.hpp"

/// @brief Used for generating and sorting moves incrementally.
///
/// Only supports generating moves for the main search currently. Should be extended to quiescence search but that loses elo currently.
class MoveSort
{
public:
    /// @brief Constructs a MoveSort object for use during the main search.
    /// @param pos The current position.
    /// @param history A reference to the history heuristic table. 
    /// @param ttMove The current transposition table move.
    /// @param k1 The first killer move.
    /// @param k2 The second killer move.
    /// @param counter The counter move.
    /// @param inCheck Whether the position is in check or not.
    MoveSort(const Position& pos, const HistoryTable& history, Move ttMove, Move k1, Move k2, Move counter, bool inCheck);

    /// @brief Generates the next best (according to heuristics) move.
    /// @return A move. If there are no more moves, the move is empty.
    Move next();

private:
    const Position& mPos;
    const HistoryTable& mHistoryTable;
    MoveList mMoveList, mTemp;
    Move mTtMove, mKiller1, mKiller2, mCounter;
    int mPhase;
    int mCurrentLocation;

    void generateNextPhase();
    void scoreEvasions();
    void selectionSort(int startingLocation);

    MoveSort& operator=(const MoveSort&) = delete;
};

#endif
