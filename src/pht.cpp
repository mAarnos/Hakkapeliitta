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

#include "pht.hpp"
#include "bitboards.hpp"
#include <cassert>
#include <cmath>

PawnHashTable::PawnHashTable()
{
    setSize(4); 
}

void PawnHashTable::setSize(size_t sizeInMegaBytes)
{
    // Clear the PHT completely to avoid any funny business.
    mTable.clear();

    // If size is not a power of two make it the biggest power of two smaller than size.
    if (Bitboards::moreThanOneBitSet(sizeInMegaBytes))
    {
        sizeInMegaBytes = static_cast<size_t>(std::pow(2, std::floor(log2(sizeInMegaBytes))));
    }

    const auto tableSize = ((sizeInMegaBytes * 1024 * 1024) / sizeof(PawnHashTableEntry));
    mTable.resize(tableSize);
}

void PawnHashTable::clear()
{
    const auto tableSize = mTable.size();
    mTable.clear();
    mTable.resize(tableSize);
}

void PawnHashTable::save(HashKey phk, int scoreOp, int scoreEd)
{
    auto& hashEntry = mTable[phk & (mTable.size() - 1)];

    hashEntry.setData((scoreOp & 0xffff) | (scoreEd << 16));
    hashEntry.setHash(phk ^ hashEntry.getData());

    assert(hashEntry.getScoreOp() == scoreOp);
    assert(hashEntry.getScoreEd() == scoreEd);
}

bool PawnHashTable::probe(HashKey phk, int& scoreOp, int& scoreEd) const
{
    const auto& hashEntry = mTable[phk & (mTable.size() - 1)];

    if ((hashEntry.getHash() ^ hashEntry.getData()) == phk)
    {
        scoreOp = hashEntry.getScoreOp();
        scoreEd = hashEntry.getScoreEd();
        return true;
    }

    return false;
}
