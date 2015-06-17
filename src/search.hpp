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

#include <thread>
#include <condition_variable>
#include "tt.hpp"
#include "history.hpp"
#include "killer.hpp"
#include "counter.hpp"
#include "evaluation.hpp"
#include "pht.hpp"
#include "utils/stopwatch.hpp"
#include "search_parameters.hpp"
#include "movelist.hpp"
#include "utils/threadpool.hpp"

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

/// @brief The core of this program, the search function.
class Search
{
public:
    /// @brief Default constructor.
    /// @param sl The SearchListener into which we should output our searchtime info.
    Search(SearchListener& sl);

    /// @brief Start searching a given root position with a given set of parameters.
    /// @param root The root position.
    /// @param sp The set of parameters given to the search function.
    ///
    /// Oh yeah, this function only starts the search, the actual searching is done by a different thread.
    /// Also, this function blocks until the search has started to prevent some problems.
    /// Usually the blocking time is very short, 5-10ms at most.
    void go(const Position& root, SearchParameters sp);

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

    /// @brief Checks if we are currently searching.
    /// @return True if we are searching.
    bool isSearching() const;

    /// @brief Stops the search function. 
    void stopSearching();

    /// @brief Stop pondering. This does not mean stopping the search.
    void stopPondering();

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
    ThreadPool tp;
    TranspositionTable transpositionTable;
    Evaluation evaluation;
    KillerTable killerTable;
    CounterMoveTable counterMoveTable;
    HistoryTable historyTable;
    SearchListener& listener;
    Stopwatch sw;

    void think(const Position& root, SearchParameters searchParameters);

    template <bool pvNode>
    int search(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss);

    int quiescenceSearch(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss);

    // Time allocation variables.
    bool searchNeedsMoreTime;
    int nodesToTimeCheck;
    uint64_t nextSendInfo;
    uint64_t targetTime;
    uint64_t maxTime;
    uint64_t maxNodes;

    // Search statistics
    uint64_t tbHits;
    uint64_t nodeCount;
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

    // Used in go for making sure that commands are synchronized.
    std::mutex waitMutex;
    std::condition_variable waitCv;

    // Used for ordering root moves.
    void orderRootMoves(const Position& pos, MoveList& moveList, const Move& ttMove) const;

    // Used for ordering captures in the quiescence search.
    void orderCaptures(const Position& pos, MoveList& moveList, const Move& ttMove) const;

    // Used for getting the PV out of the TT:
    std::vector<Move> extractPv(const Position& root) const;
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

inline bool Search::isSearching() const
{
    return searching;
}

inline void Search::stopSearching()
{
    searching = false;
}

inline void Search::stopPondering()
{
    pondering = false;
}

#endif