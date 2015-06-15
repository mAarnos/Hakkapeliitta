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

/// @file search.hpp
/// @author Mikko Aarnos

#ifndef SEARCH_HPP_
#define SEARCH_HPP_

#include "tt.hpp"
#include "history.hpp"
#include "killer.hpp"
#include "counter.hpp"
#include "evaluation.hpp"
#include "pht.hpp"
#include "utils\stopwatch.hpp"
#include "search_parameters.hpp"
#include "movelist.hpp"
#include <thread>
#include <condition_variable>

/// @brief The core of this program, the search function.
class Search
{
public:
    /// @brief Default constructor.
    Search();

    /// @brief Clears the TT, PHT, killer table, history table and the counter move table. 
    ///
    /// Can take a long time with very large TT and PHT.
    void clearSearch();

    /// @brief Used for setting the size of the TT. 
    /// @param sizeInMegaBytes The new size of the TT.
    ///
    /// Can take a long time with a large value of sizeInMegaBytes.
    void setTranspositionTableSize(size_t sizeInMegaBytes);

    /// @brief Used for setting the size of the PHT. 
    /// @param sizeInMegaBytes The new size of the PHT.
    ///
    /// Can take a long time with a large value of sizeInMegaBytes.
    void setPawnHashTableSize(size_t sizeInMegaBytes);

private:
    // A stack used for holding information which needs to be accessible to other levels of recursion.
    struct SearchStack
    {
        SearchStack(int newPly)
        {
            mAllowNullMove = true;
            mPly = newPly;
        }

        Move mCurrentMove;
        int mPly;
        bool mAllowNullMove;
    };
    
    // Different classes used by the search function.
    TranspositionTable transpositionTable;
    Evaluation evaluation;
    KillerTable killerTable;
    CounterMoveTable counterMoveTable;
    HistoryTable historyTable;
    Stopwatch sw;

    void think(const Position& root, SearchParameters searchParameters);

    template <bool pvNode>
    int search(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss);

    int quiescenceSearch(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss);

    // Time allocation variables.
    bool searchNeedsMoreTime;
    int nodesToTimeCheck;
    int nextSendInfo;
    int targetTime;
    int maxTime;
    size_t maxNodes;

    // Search statistics
    size_t tbHits;
    size_t nodeCount;
    int selDepth;

    // Flags related to stopping the search.
    bool searching;
    bool pondering;
    bool infinite;

    // These three are used to detect repetitions.
    // Note that the repetitions can include positions which happened during position set-up.
    // If this were not the case we would not require rootPly.
    // Actually, we check for 2-fold repetitions instead of 3-fold repetitions like FIDE-rules require.
    // If you think about it for a while, you notice that 2-fold is all we need.
    int rootPly;
    std::vector<HashKey> repetitionHashes;
    bool repetitionDraw(const Position& pos, int ply) const;

    // Used for changing the values of draws inside the search.
    std::array<int, 2> contempt;

    // A 2D array of LMR reductions which has to be initialized at run time.
    std::array<std::array<int, 64>, 64> lmrReductions;

    // A array of LMP move counts which has to be initialized at run time.
    std::array<int, 1 + lmpDepth> lmpMoveCounts;

    // Used for ordering captures in the quiescence search.
    void orderCaptures(const Position& pos, MoveList& moveList, const Move& ttMove) const;
};

inline void Search::clearSearch() 
{ 
    transpositionTable.clear();
    evaluation.clearPawnHashTable(); 
    killerTable.clear(); 
    historyTable.clear();
    counterMoveTable.clear();
}

inline void Search::setTranspositionTableSize(size_t sizeInMegaBytes)
{
    transpositionTable.setSize(sizeInMegaBytes);
}

inline void Search::setPawnHashTableSize(size_t sizeInMegaBytes)
{ 
    evaluation.setPawnHashTableSize(sizeInMegaBytes);
}

#endif