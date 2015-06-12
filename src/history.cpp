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

HistoryTable::HistoryTable()
{
    clear();
}

void HistoryTable::clear()
{
    for (auto i = 0; i < 12; ++i)
    {
        mHistory[i].fill(0);
        mButterfly[i].fill(0);
    }
}

void HistoryTable::addCutoff(const Position& pos, const Move& move, const int depth)
{
    mHistory[pos.getBoard(move.getFrom())][move.getTo()] += depth * depth;
}

void HistoryTable::addNotCutoff(const Position& pos, const Move& move, const int depth)
{
    mButterfly[pos.getBoard(move.getFrom())][move.getTo()] += depth * depth;
}

int16_t HistoryTable::getScore(const Position& pos, const Move& move) const
{
    const auto hScore = mHistory[pos.getBoard(move.getFrom())][move.getTo()];
    const auto bScore = mButterfly[pos.getBoard(move.getFrom())][move.getTo()];

    if (hScore + bScore == 0)
    {
        return 50; // no information means that the move could be good or bad, so give a neutral score.
    }

    return static_cast<int16_t>((hScore * 100) / (hScore + bScore));
}

void HistoryTable::age()
{
    for (auto i = 0; i < 12; ++i)
    {
        for (auto j = 0; j < 64; ++j)
        {
            mHistory[i][j] /= 2;
            mButterfly[i][j] /= 2;
        }
    }
}

