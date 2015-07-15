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

#include "tt.hpp"
#include "bitboards.hpp"
#include <cassert>
#include <cmath>
#include <unordered_set>

TranspositionTable::TranspositionTable()
{
    setSize(32); 
}

void TranspositionTable::setSize(size_t sizeInMegaBytes)
{
    // If size is not a power of two make it the biggest power of two smaller than size.
    if (Bitboards::moreThanOneBitSet(sizeInMegaBytes))
    {
        sizeInMegaBytes = static_cast<size_t>(std::pow(2, std::floor(log2(sizeInMegaBytes))));
    }

    const auto tableSize = ((sizeInMegaBytes * 1024 * 1024) / sizeof(std::array<TranspositionTableEntry, bucketSize>));
    mTable.clear();
    mTable.resize(tableSize);
    mTable.shrink_to_fit();
    mGeneration = 1;
}

void TranspositionTable::clear()
{
    const auto tableSize = mTable.size();
    mTable.clear();
    mTable.resize(tableSize);
    mGeneration = 1;
}

void TranspositionTable::prefetch(HashKey hk) const
{
    const auto* address = reinterpret_cast<const char*>(&mTable[hk & (mTable.size() - 1)]);
#if defined (_MSC_VER) || defined(__INTEL_COMPILER)
    _mm_prefetch(address, _MM_HINT_T0);
#else
    __builtin_prefetch(address);
#endif
}

void TranspositionTable::save(HashKey hk, const Move& move, int score, int depth, int flags)
{
    auto best = move;
    auto hashEntry = &mTable[hk & (mTable.size() - 1)][0];
    auto replace = hashEntry;

    // Determine the least valuable entry to replace.
    for (auto i = 0; i < bucketSize; ++i, ++hashEntry)
    {
        // If there already is an entry for this hashkey, replace it immediately.
        // If that entry was any good we wouldn't have gotten here.
        if ((hashEntry->getHash() ^ hashEntry->getData()) == hk)
        {
            replace = hashEntry;
            if (best.empty())
            {
                best = hashEntry->getBestMove();
            }
            break;
        }

        // First replace entries which are from an older search, if that doesn't work consider depth.
        if ((hashEntry->getGeneration() == mGeneration)
          - (replace->getGeneration() == mGeneration)
          - (hashEntry->getDepth() < replace->getDepth()) < 0)
        {
            replace = hashEntry;
        }
    }

    replace->setData((static_cast<uint64_t>(best.getRawMove()) | 
                      static_cast<uint64_t>(mGeneration) << 16 | 
                      static_cast<uint64_t>(score & 0xffff) << 32 | 
                      static_cast<uint64_t>(depth & 0xff) << 48) | 
                      static_cast<uint64_t>(flags) << 56);
    // Use Dr. Hyatt's lockless hashing to make sure that there are no corrupted TT entries which remain undetected.
    // Not really necessary until we have multithreading.
    replace->setHash(hk ^ replace->getData());

    // In multithreaded mode these could potetially fail. Think about that someday, even though we don't HAVE multithreading yet.
    assert(replace->getBestMove() == best);
    assert(replace->getGeneration() == mGeneration);
    assert(replace->getScore() == score);
    assert(replace->getDepth() == depth);
    assert(replace->getFlags() == flags);
}

const TranspositionTable::TranspositionTableEntry* TranspositionTable::probe(HashKey hk) const
{
    const auto* hashEntry = &mTable[hk & (mTable.size() - 1)][0];

    for (auto i = 0; i < bucketSize; ++i, ++hashEntry)
    {
        if ((hashEntry->getHash() ^ hashEntry->getData()) == hk)
        {
            return hashEntry;
        }
    }

    return nullptr;
}

void TranspositionTable::startNewSearch() noexcept
{ 
    ++mGeneration; 
}

int TranspositionTable::hashFull() const noexcept
{
    auto cnt = 0;

    for (auto i = 0; i < 1000 / bucketSize; ++i)
    {
        const auto* hashEntry = &mTable[i][0];
        for (auto j = 0; j < bucketSize; ++j, ++hashEntry)
        {
            if (hashEntry->getFlags() != Flags::Empty)
            {
                cnt += 1;
            }
        }
    }

    return cnt;
}



