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

#ifndef MOVESORT_HPP_
#define MOVESORT_HPP_

#include "movelist.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "history.hpp"
#include "killer.hpp"
#include "counter.hpp"
#include "search.hpp"

class MoveSort
{
public:
    MoveSort(const Position& pos, const HistoryTable& history, uint16_t ttMove, uint16_t k1, uint16_t k2, uint16_t counter, bool inCheck);

    Move next();
private:
    static MoveGen moveGen;
    const Position& pos;
    const HistoryTable& historyTable;
    MoveList moveList, temp;
    Move ttMove;
    uint16_t k1, k2, counter;
    int phase;
    int currentLocation;

    void generateNextPhase();
    void scoreEvasions();
    Move selectionSort(int startingLocation);

    MoveSort& operator=(const MoveSort&) = delete;
};

#endif
