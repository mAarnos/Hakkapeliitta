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

class Search
{
public:
    Search(TranspositionTable& transpositionTable, PawnHashTable& pawnHashTable, KillerTable& killerTable, HistoryTable& historyTable);
private:
    TranspositionTable& transpositionTable;
    KillerTable& killerTable;
    HistoryTable& historyTable;
    Evaluation evaluation;
    MoveGen moveGen;

    int quiescenceSearch(const Position& pos, int depth, int ply, int alpha, int beta, bool inCheck);

    Search& operator=(const Search&) = delete;
};

#endif