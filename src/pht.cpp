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
#include <cassert>
#include <cmath>
#include "utils/clamp.hpp"

PawnHashTable::PawnHashTable()
{
    setSize(4); 
}

void PawnHashTable::setSize(size_t sizeInMegaBytes)
{
    // Clear the tt completely to avoid any funny business.
    table.clear();

    // If size is not a power of two make it the biggest power of two smaller than size.
    if (sizeInMegaBytes & (sizeInMegaBytes - 1))
    {
        sizeInMegaBytes = static_cast<int>(std::pow(2, std::floor(std::log2(sizeInMegaBytes))));
    }

    const auto tableSize = ((sizeInMegaBytes * 1024 * 1024) / sizeof(PawnHashTableEntry));
    table.resize(tableSize);
}

void PawnHashTable::clear()
{
    const auto tableSize = table.size();
    table.clear();
    table.resize(tableSize);
}

void PawnHashTable::save(const HashKey hk, const int scoreOp, const int scoreEd)
{
    auto& hashEntry = table[hk & (table.size() - 1)];

    hashEntry.setData((scoreOp & 0xffff) | (scoreEd << 16));
    hashEntry.setHash(hk ^ hashEntry.getData());

    assert(static_cast<int16_t>(hashEntry.getData()) == scoreOp);
    assert(static_cast<int16_t>(hashEntry.getData() >> 16) == scoreEd);
}

bool PawnHashTable::probe(const HashKey hk, int& scoreOp, int& scoreEd) const
{
    const auto& hashEntry = table[hk & (table.size() - 1)];

    if ((hashEntry.getHash() ^ hashEntry.getData()) == hk)
    {
        scoreOp = static_cast<int16_t>(hashEntry.getData());
        scoreEd = static_cast<int16_t>(hashEntry.getData() >> 16);
        return true;
    }

    return false;
}
