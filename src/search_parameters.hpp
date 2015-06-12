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

/// @file search_parameters.hpp
/// @author Mikko Aarnos

#ifndef SEARCH_PARAMETERS_HPP_
#define SEARCH_PARAMETERS_HPP_

#include <array>
#include <vector>
#include "move.hpp"

/// @brief Contains options for the search function.
struct SearchParameters 
{
    /// @brief Default constructor.
    SearchParameters() noexcept;

    /// @brief Only search these moves. If empty search all.
    std::vector<Move> searchMoves; 

    /// @brief Whether this search is for pondering or not.
    bool ponder; 

    /// @brief Whether the "Ponder" UCI option is set or not.
    bool ponderOption;

    /// @brief The value for the "Contempt" UCI option.
    int contempt;

    /// @brief Absolute time limits for white and black in milliseconds.
    std::array<int, 2> time; 

    /// @brief Increments for white and black in milliseconds.
    std::array<int, 2> increment; 
    
    /// @brief Moves to go until next time control.
    int movesToGo; 

    /// @brief Don't search deeper than this depth.
    int depth; 

    /// @brief Don't search more nodes than this.
    size_t nodes; 

    /// @brief Try to find a mate in x.
    int mate; 

    /// @brief Use exactly this much time in milliseconds for the search.
    int moveTime; 

    /// @brief Whether the search is infinite or not.
    bool infinite; 

    /// @brief The amount of plies from the position depicted by the initial FEN string received.
    int rootPly;

    /// @brief The hash keys of all positions encountered during the game so far, in order.
    std::vector<HashKey> hashKeys;
};

inline SearchParameters::SearchParameters() noexcept :
    ponder(false), ponderOption(false), contempt(0), time({ { 0, 0 } }), increment({ { 0, 0 } }),
    movesToGo(0), depth(0), nodes(0), mate(0), moveTime(0), infinite(false), rootPly(0)
{
};

#endif