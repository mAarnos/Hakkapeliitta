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
#include "search_parameters.hpp"

class SearchStack
{
public:
    void clear(int newPly)
    {
        move.setMove(0);
        move.setScore(0);
        allowNullMove = true;
        ply = newPly;
    }

    Move move;
    int ply;
    bool allowNullMove;
};

class Search
{
public:
    Search();

    void think(const Position& root, SearchParameters searchParameters, int newRootPly, std::array<HashKey, 1024> newRepetitionHashKeys, int contemptValue);

    void stopSearching() { searching = false; }
    void stopPondering() { pondering = false; }
    bool isSearching() const { return searching; }
    bool isPondering() const { return pondering; }

    void clearSearch() { transpositionTable.clear(); evaluation.clearPawnHashTable(); killerTable.clear(); historyTable.clear(); }
    void setTranspositionTableSize(size_t newSize) { transpositionTable.setSize(newSize); }
    void setPawnHashTableSize(size_t newSize) { evaluation.setPawnHashTableSize(newSize); }
private:
    TranspositionTable transpositionTable;
    KillerTable killerTable;
    HistoryTable historyTable;
    Evaluation evaluation;
    MoveGen moveGen;

    template <bool pvNode>
    int search(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss);
    int quiescenceSearch(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss);

    void orderCaptures(const Position& pos, MoveList& moveList, const Move& ttMove);
    void orderMoves(const Position& pos, MoveList& moveList, const Move& ttMove, int ply) const;

    std::array<int, 2> contempt;
    int nextSendInfo;
    int targetTime;
    int maxTime;
    size_t maxNodes;

    // Search statistics
    int tbHits;
    size_t nodeCount;
    int nodesToTimeCheck;
    int selDepth;

    bool searchNeedsMoreTime;

    bool searching;
    bool pondering;
    bool infinite;

    Stopwatch sw;

    // These two are used to detect repetitions.
    // Note that the repetitions can include positions which happened during position set-up.
    // If this were not the case we would not require rootPly.
    int rootPly;
    std::array<HashKey, 1024> repetitionHashes;

    // Array of LMR reduction which have to be initialized at run time.
    std::array<std::array<int, 64>, 64> lmrReductions;

    // And here is the way to check repetitions, obviously.
    // Actually, this checks for 2-fold repetitions instead of 3-fold repetitions like FIDE-rules require.
    // If you think about it for a while, you notice that 2-fold is all we need.
    bool repetitionDraw(const Position& pos, int ply) const;

    void infoCurrMove(const Move& move, int depth, int nr);
    void infoPv(const std::vector<Move>& moves, int depth, int score, int flags);

    Search& operator=(const Search&) = delete;
};

#endif