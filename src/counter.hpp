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

#ifndef COUNTER_HPP_
#define COUNTER_HPP_

#include <cstdint>
#include <array>
#include "position.hpp"
#include "move.hpp"

// Counter moves (last move which refuted the previous move of the opponent) for the search function encapsulated.
class CounterMoveTable
{
public:
    CounterMoveTable();

    // Updates the countermove of opponentMove to be move.
    void updateCounterMoves(const Position& pos, const Move& move, const Move& opponentMove);
    // Checks if a move is a counter move given the last move of the opponent.
    uint16_t getCounterMove(const Position& pos, const Move& opponentMove) const;
    // Clears the countermove table.
    void clear();
private:
    std::array<std::array<uint16_t, 64>, 12> counterMoves;
};

#endif
