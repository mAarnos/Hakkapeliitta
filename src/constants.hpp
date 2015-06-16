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

/// @file constants.hpp
/// @author Mikko Aarnos

#ifndef CONSTANTS_HPP_
#define CONSTANTS_HPP_

/// @brief Maximal ply from the root position which can be reached during search.
const int maxPly = 128;

/// @brief Enum for castling rights. We OR these together to represent different combinations. For example, 9 means WhiteOO and BlackOOO are legal.
enum CastlingRights 
{
    WhiteOO = 1, WhiteOOO = 2, BlackOO = 4, BlackOOO = 8
};

// Some constants defining mate values.
// TODO: lower mateScore to 30000 or so.
/// @brief The score given to a mate in 0 moves.
const int mateScore = 32767; 

/// @brief The score given to a mate in 500 moves. We do not recognize worse mates, hence the name.
const int minMateScore = 32767 - 1000; 

/// @brief The absolute maximum score used in the search function.
const int infinity = mateScore + 1;

/// @brief The aspiration window.
const int aspirationWindow = 16;

/// @brief Always reduce at least this much when using null move.
const int baseNullReduction = 3;

/// @brief The max depth we use futility pruning at.
const int futilityDepth = 7;

/// @brief The delta pruning margin.
const int deltaPruningMargin = 50;

/// @brief The max depth we use reverse futility pruning at.
const int reverseFutilityDepth = 5;

/// @brief The min depth we use LMR at.
const int lmrDepthLimit = 3;

/// @brief The max depth we use LMP at.
const int lmpDepth = 6;

/// @brief The max depth we use razoring at.
const int razoringDepth = 3;

/// @brief The max depth we use SEE pruning at.
const int seePruningDepth = 3;

// Move ordering scores.
// Delete as soon as MoveSort works everywhere.
/// @brief The move ordering score given to a TT-move.
const int16_t hashMoveScore = 30000;

/// @brief The move ordering score given to a good capture move.
const int16_t captureMoveScore = hashMoveScore >> 1;

/// @brief The move ordering scores given to killer moves, ordered by killer level.
const std::array<int16_t, 1 + 4> killerMoveScore = {
    0, hashMoveScore >> 2, hashMoveScore >> 3, hashMoveScore >> 4, hashMoveScore >> 5
};

/// @brief The move ordering score given to a counter move.
const int16_t counterMoveScore = hashMoveScore >> 6;

#endif
