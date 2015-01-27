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

#ifndef KILLER_HPP_
#define KILLER_HPP_

#include <cstdint>
#include <array>
#include "move.hpp"

// Killer moves for the search function encapsulated.
class KillerTable
{
public:
    KillerTable();

    // Add a killer move for the given ply. Assumes that the move is not a capture or a promotion.
    void addKiller(const Move& move, int ply);
    // Checks if the given move is a killer move. Returns 0 if it is not and 1-4 in case it is (1 is best, 4 is worst). 
    int isKiller(const Move& move, int ply) const;
    // Clears the killer table.
    void clear();
private:
    std::array<std::array<uint16_t, 2>, 128> killers;
};

#endif
