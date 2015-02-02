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

#ifndef SEARCH_PARAMETERS_HPP_
#define SEARCH_PARAMETERS_HPP_

#include <array>
#include <vector>
#include "move.hpp"

class SearchParameters 
{
public:
    SearchParameters() : 
        ponder(false), time({{ 0, 0 }}), increment({{ 0, 0 }}), 
        movesToGo(25), depth(0), nodes(0), mate(0), moveTime(0), infinite(false) 
    {
    };

    std::vector<Move> searchMoves; // Search only these moves. If empty search all.
    bool ponder; // Whether to ponder or not.
    std::array<int, 2> time; // Absolute time limits for white and black.
    std::array<int, 2> increment; // Increments for white and black.
    int movesToGo; // Moves to go until next time control.
    int depth; // Don't search deeper than this.
    int nodes; // Don't search more nodes than this.
    int mate; // Try to find a mate in x.
    int moveTime; // Use exactly this much time in milliseconds.
    bool infinite; // Whether the search is infinite or not.
};

#endif