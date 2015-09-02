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
    auto tableSize = ((sizeInMegaBytes * 1024 * 1024) / sizeof(PawnHashTableEntry));
    // If size is not a power of two make it the biggest power of two smaller than size.
    if (Bitboards::moreThanOneBitSet(tableSize))
    {
        tableSize = static_cast<size_t>(std::pow(2, std::floor(log2(tableSize))));
    }

    mTable.clear();
    mTable.resize(tableSize);
    mTable.shrink_to_fit();
}

void PawnHashTable::clear()
{
    const auto tableSize = mTable.size();
    mTable.clear();
    mTable.resize(tableSize);
}

void PawnHashTable::save(HashKey phk, Bitboard passers, int scoreOp, int scoreEd, const std::array<uint8_t, 2>& pawnShieldScore)
{
    assert((phk & (mTable.size() - 1)) == (phk % mTable.size()));

    auto& hashEntry = mTable[phk & (mTable.size() - 1)];

    // Always overwrite.
    hashEntry.mHash = phk;
    hashEntry.mPassers = passers;
    hashEntry.mScoreOp = static_cast<int16_t>(scoreOp);
    hashEntry.mScoreEd = static_cast<int16_t>(scoreEd);
    hashEntry.mPawnShieldScore[Color::White] = pawnShieldScore[Color::White];
    hashEntry.mPawnShieldScore[Color::Black] = pawnShieldScore[Color::Black];
}

bool PawnHashTable::probe(HashKey phk, Bitboard& passers, int& scoreOp, int& scoreEd, std::array<uint8_t, 2>& pawnShieldScore) const
{
    assert((phk & (mTable.size() - 1)) == (phk % mTable.size()));

    const auto& hashEntry = mTable[phk & (mTable.size() - 1)];

    if (hashEntry.mHash == phk)
    {
        passers = hashEntry.mPassers;
        scoreOp = hashEntry.mScoreOp;
        scoreEd = hashEntry.mScoreEd;
        pawnShieldScore[Color::White] = hashEntry.mPawnShieldScore[Color::White];
        pawnShieldScore[Color::Black] = hashEntry.mPawnShieldScore[Color::Black];
        return true;
    }

    return false;
}
