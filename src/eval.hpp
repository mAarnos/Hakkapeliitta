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

#ifndef EVAL_HPP_
#define EVAL_HPP_

#include <array>
#include <vector>
#include <unordered_set>
#include "position.hpp"
#include "zobrist.hpp"
#include "endgame.hpp"
#include "pht.hpp"

class Evaluation
{
public:
    Evaluation(PawnHashTable& pawnHashTable);

    int evaluate(const Position& pos, bool& zugzwangLikely);

    // These two have to be annoyingly static, as we use them in position.cpp to incrementally update the pst eval.
    static std::array<std::array<short, 64>, 12> pieceSquareTableOpening;
    static std::array<std::array<short, 64>, 12> pieceSquareTableEnding;
private:
    // Contains information on some endgames.
    EndgameModule endgameModule;
    // Used for hashing pawn eval scores.
    PawnHashTable& pawnHashTable;

    // Evaluation function in parts.
    template <bool hardwarePopcnt> 
    int evaluate(const Position& pos, bool& zugzwangLikely);

    template <bool hardwarePopcnt> 
    int mobilityEval(const Position& pos, std::array<int, 2>& kingSafetyScore, int phase, bool& zugzwangLikely);

    int pawnStructureEval(const Position& pos, int phase);
    int kingSafetyEval(const Position& pos, int phase, std::array<int, 2>& kingSafetyScore);

    Evaluation& operator=(const Evaluation&) = delete;
};

#endif