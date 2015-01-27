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

#include "history.hpp"
#include <cstring>

HistoryTable::HistoryTable()
{
    clear();
}

void HistoryTable::clear()
{
    for (auto i = 0; i < 12; ++i)
    {
        history[i].fill(0);
        butterfly[i].fill(0);
    }
}

void HistoryTable::addCutoff(const Position& pos, const Move& move, const int depth)
{
    history[pos.getBoard(move.getFrom())][move.getTo()] += depth * depth;
}

void HistoryTable::addNotCutoff(const Position& pos, const Move& move, const int depth)
{
    butterfly[pos.getBoard(move.getFrom())][move.getTo()] += depth * depth;
}

int HistoryTable::getScore(const Position& pos, const Move& move) const
{
    return ((history[pos.getBoard(move.getFrom())][move.getTo()] * 100) / 
            (butterfly[pos.getBoard(move.getFrom())][move.getTo()] + 1));
}

void HistoryTable::age()
{
    for (auto i = 0; i < 12; ++i)
    {
        for (auto j = 0; j < 64; ++j)
        {
            history[i][j] /= 2;
            butterfly[i][j] /= 2;
        }
    }
}

