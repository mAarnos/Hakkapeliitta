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

#ifndef SEARCH_HPP_
#define SEARCH_HPP_

#include "movegen.hpp"
#include "tt.hpp"
#include "history.hpp"
#include "killer.hpp"
#include "eval.hpp"
#include "pht.hpp"
#include "utils\stopwatch.hpp"

class Search
{
public:
    Search(TranspositionTable& transpositionTable, PawnHashTable& pawnHashTable, KillerTable& killerTable, HistoryTable& historyTable);

    int quiescenceSearch(const Position& pos, int depth, int ply, int alpha, int beta, bool inCheck);
private:
    TranspositionTable& transpositionTable;
    KillerTable& killerTable;
    HistoryTable& historyTable;
    Evaluation evaluation;
    MoveGen moveGen;

    void orderMoves(const Position& pos, MoveList& moveList, const Move& ttMove, int ply);

    Search& operator=(const Search&) = delete;

    std::array<int, 2> contempt;
    int targetTime;
    int maxTime;

    // Search statistics
    int tbHits;
    size_t nodeCount;
    int nodesToTimeCheck;
    int selDepth;

    bool infinite;

    Stopwatch sw;

    // These two are used to detect repetitions.
    // Note that the repetitions can include positions which happened during position set-up.
    // If this were not the case we would not require rootPly.
    int rootPly;
    std::array<HashKey, 1024> repetitionHashes;

    // And here is the way to check repetitions, obviously.
    // Actually, this checks for 2-fold repetitions instead of 3-fold repetitions like FIDE-rules require.
    // If you think about it for a while, you notice that 2-fold is all we need.
    bool repetitionDraw(const Position& pos, int ply);
};

#endif