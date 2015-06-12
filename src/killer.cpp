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

#include "killer.hpp"

KillerTable::KillerTable()
{
    clear();
}

void KillerTable::update(const Move& move, int ply)
{
    // Make sure that we won't have two same killers stored.
    if (move != mKillers[ply][0])
    {
        mKillers[ply][1] = mKillers[ply][0];
        mKillers[ply][0] = move;
    }
}

std::pair<Move, Move> KillerTable::getKillers(int ply) const
{
    return std::make_pair(mKillers[ply][0], mKillers[ply][1]);
}

void KillerTable::clear()
{
    mKillers.fill({});
}

