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
const int mateScore = 32767; // mate in 0
const int minMateScore = 32767 - 1000; // mate in 500
const int infinity = mateScore + 1;

// Different constants dealing with pruning and reductions.
const int aspirationWindow = 16;
const int baseNullReduction = 3;
const int futilityDepth = 7;
const int deltaPruningMargin = 50;
const int reverseFutilityDepth = 5;
const int lmrDepthLimit = 3;
const int lmpDepth = 6;
const int razoringDepth = 3;
const int seePruningDepth = 3;

// Move ordering scores.
// Delete as soon as MoveSort works everywhere.
const int16_t hashMoveScore = 30000;
const int16_t captureMoveScore = hashMoveScore >> 1;
const std::array<int16_t, 1 + 4> killerMoveScore = {
    0, hashMoveScore >> 2, hashMoveScore >> 3, hashMoveScore >> 4, hashMoveScore >> 5
};
const int16_t counterMoveScore = hashMoveScore >> 6;

#endif
