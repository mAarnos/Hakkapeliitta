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
public:
private:
    Evaluation evaluation;
    KillerTable killerTable;
    CounterMoveTable counterMoveTable;
    HistoryTable historyTable;
};

#endif