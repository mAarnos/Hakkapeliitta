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
#include <cassert>
#include <cstring>

KillerTable::KillerTable()
{
    clear();
}

void KillerTable::clear()
{
    for (auto i = 0; i < 128; ++i)
    {
        killers[i].fill(0);
    }
}

void KillerTable::addKiller(const Move& move, const int ply)
{
    const auto candidateKiller = move.getMove();
    // Only replace if we won't have two same killers if we replace.
    if (candidateKiller != killers[ply][0])
    {
        killers[ply][1] = killers[ply][0];
        killers[ply][0] = candidateKiller;
    }
}

int KillerTable::isKiller(const Move& move, const int ply) const
{
    const auto possibleKiller = move.getMove();
    if (possibleKiller == killers[ply][0])
    {
        return 1;
    }
    else if (possibleKiller == killers[ply][1])
    {
        return 2;
    }
    else
    {
        return 0;
    }
}

uint16_t KillerTable::getKillerA(int ply) const
{
    return killers[ply][0];
}

uint16_t KillerTable::getKillerB(int ply) const
{
    return killers[ply][1];
}

