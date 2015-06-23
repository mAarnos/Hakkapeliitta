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

/// @file search_listener.hpp
/// @author Mikko Aarnos

#ifndef SEARCH_LISTENER_HPP_
#define SEARCH_LISTENER_HPP_

#include <vector>
#include "move.hpp"

/// @brief An interface for outputting info during the search.
class SearchListener
{
public:
    /// @brief Send info on the current root move we are searching.
    /// @param move The move we are currently searching.
    /// @param depth The depth we are searching the move to.
    /// @param i The position of this move in the move list. Smaller number means it is searched earlier.
    virtual void infoCurrMove(const Move& move, int depth, int i) = 0;

    /// @brief Send info which should be sent regularly (i.e. once a second).
    /// @param nodeCount The current amount of nodes searched.
    /// @param tbHits The current amount of tablebase probes done.
    /// @param searchTime The current amount of time spent searching, in milliseconds.
    virtual void infoRegular(uint64_t nodeCount, uint64_t tbHits, uint64_t searchTime) = 0;

    /// @brief Send info on a new PV.
    /// @param pv The current principal variation.
    /// @param searchTime The current amount of time spent searching, in milliseconds.
    /// @param nodeCount The current amount of nodes searched.
    /// @param tbHits The current amount of tablebase probes done.
    /// @param depth The current depth.
    /// @param score The score of the new PV.
    /// @param flags Information on the type of PV. Can be exact, upperbound or lowerbound, just like TT entries.
    /// @param selDepth The max selective search depth reached so far.
    virtual void infoPv(const std::vector<Move>& pv, uint64_t searchTime,
                        uint64_t nodeCount, uint64_t tbHits,
                        int depth, int score, int flags, int selDepth) = 0;

    /// @brief When we are finishing the search send info on the best move.
    /// @param pv The current principal variation.
    /// @param searchTime The current amount of time spent searching, in milliseconds.
    /// @param nodeCount The current amount of nodes searched.
    /// @param tbHits The current amount of tablebase probes done.
    virtual void infoBestMove(const std::vector<Move>& pv, uint64_t searchTime,
                              uint64_t nodeCount, uint64_t tbHits) = 0;
};


#endif