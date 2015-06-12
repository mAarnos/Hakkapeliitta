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
    SearchParameters();

    /// @brief Only search these moves. If empty search all.
    std::vector<Move> mSearchMoves; 

    /// @brief Whether this search is for pondering or not.
    bool mPonder; 

    /// @brief Whether the "Ponder" UCI option is set or not.
    bool mPonderOption;

    /// @brief The value for the "Contempt" UCI option.
    int mContempt;

    /// @brief Absolute time limits for white and black in milliseconds.
    std::array<int, 2> mTime; 

    /// @brief Increments for white and black in milliseconds.
    std::array<int, 2> mIncrement; 
    
    /// @brief Moves to go until next time control.
    int mMovesToGo; 

    /// @brief Don't search deeper than this depth.
    int mDepth; 

    /// @brief Don't search more nodes than this.
    size_t mNodes; 

    /// @brief Try to find a mate in x.
    int mMate; 

    /// @brief Use exactly this much time in milliseconds for the search.
    int mMoveTime; 

    /// @brief Whether the search is infinite or not.
    bool mInfinite; 

    /// @brief The amount of plies from the position depicted by the initial FEN string received.
    int mRootPly;

    /// @brief The hash keys of all positions encountered during the game so far, in order.
    std::vector<HashKey> mHashKeys;
};

inline SearchParameters::SearchParameters():
    mPonder(false), mPonderOption(false), mContempt(0), mTime({ { 0, 0 } }), mIncrement({ { 0, 0 } }),
    mMovesToGo(0), mDepth(0), mNodes(0), mMate(0), mMoveTime(0), mInfinite(false), mRootPly(0)
{
};

#endif