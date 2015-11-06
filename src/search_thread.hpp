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

/// @file search_thread.hpp
/// @author Mikko Aarnos

#ifndef SEARCH_THREAD_HPP_
#define SEARCH_THREAD_HPP_

#include <thread>
#include "evaluation.hpp"
#include "killer.hpp"
#include "counter.hpp"
#include "history.hpp"

/// @brief A single thread we use when searching. 
class SearchThread : public std::thread
{
    friend class Search;
public:
    /// @brief Default constructor.
    template<class Fn, class... Args>
    SearchThread(Fn&& f, Args&&... args);

private:
    Evaluation evaluation;
    KillerTable killerTable;
    CounterMoveTable counterMoveTable;
    HistoryTable historyTable;

    // Search statistics
    uint64_t tbHits;
    uint64_t nodeCount;
    int selDepth;

    // Used for detecting repetitions.
    std::vector<HashKey> repetitionHashes;
};

template<class Fn, class... Args>
inline SearchThread::SearchThread(Fn&& f, Args&&... args) 
    : std::thread(std::forward<Fn>(f), std::forward<Args>(args)...), tbHits(0), nodeCount(0), selDepth(0)
{
}

#endif