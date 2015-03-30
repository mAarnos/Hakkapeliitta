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
#include <cassert>
#include <cmath>
#include <unordered_set>
#include "utils/clamp.hpp"

TranspositionTable::TranspositionTable():
table(nullptr), tableSize(0)
{
    setSize(32); 
}

TranspositionTable::~TranspositionTable()
{
    delete table;
}

void TranspositionTable::setSize(size_t sizeInMegaBytes)
{
    // Clear the tt completely to avoid any funny business.
    delete table;

    // If size is not a power of two make it the biggest power of two smaller than size.
    if (sizeInMegaBytes & (sizeInMegaBytes - 1))
    {
        sizeInMegaBytes = static_cast<int>(std::pow(2, std::floor(std::log2(sizeInMegaBytes))));
    }

    tableSize = ((sizeInMegaBytes * 1024 * 1024) / sizeof(Cluster));
    table = static_cast<Cluster*>(malloc(tableSize * sizeof(Cluster)));
    clear();
}

void TranspositionTable::clear()
{
    memset(table, 0, sizeof(Cluster) * tableSize);
    generation = 1;
}

void TranspositionTable::prefetch(const HashKey hk) const
{
    const auto* address = reinterpret_cast<char*>(&table[hk & (tableSize - 1)]);
#if defined (_MSC_VER) || defined(__INTEL_COMPILER)
    _mm_prefetch(address, _MM_HINT_T0);
#else
    __builtin_prefetch(address);
#endif
}

void TranspositionTable::save(const HashKey hk, const Move& bestMove, const int score, const int depth, const int flags)
{
    auto best = bestMove.getMove();
    auto hashEntry = table[hk & (tableSize - 1)].entries;
    auto replace = hashEntry;

    // Determine the least valuable entry to replace
    for (auto i = 0; i < 4; ++i, ++hashEntry)
    {
        if ((hashEntry->getHash() ^ hashEntry->getData()) == hk)
        {
            replace = hashEntry;
            if (!best)
            {
                best = hashEntry->getBestMove();
            }
            break;
        }

        // First replace entries which are from an older search, if that doesn't work consider depth.
        if ((hashEntry->getGeneration() == generation)
          - (replace->getGeneration() == generation)
          - (hashEntry->getDepth() < replace->getDepth()) < 0)
        {
            replace = hashEntry;
        }
    }

    replace->setData((static_cast<uint64_t>(best) | 
                      static_cast<uint64_t>(generation) << 16 | 
                      static_cast<uint64_t>(score & 0xffff) << 32 | 
                      static_cast<uint64_t>(depth & 0xff) << 48) | 
                      static_cast<uint64_t>(flags) << 56);
    replace->setHash(hk ^ replace->getData());

    assert(replace->getBestMove() == best);
    assert(replace->getGeneration() == generation);
    assert(replace->getScore() == score);
    assert(replace->getDepth() == depth);
    assert(replace->getFlags() == flags);
}

const TranspositionTableEntry* TranspositionTable::probe(const HashKey hk) const
{
    const auto* hashEntry = &table[hk & (tableSize - 1)].entries[0];

    for (auto i = 0; i < 4; ++i, ++hashEntry)
    {
        if ((hashEntry->getHash() ^ hashEntry->getData()) == hk)
        {
            return hashEntry;
        }
    }

    return nullptr;
}

std::vector<Move> TranspositionTable::extractPv(Position root) const
{
    std::vector<Move> pv;
    std::unordered_set<HashKey> previousHashes;
    Move m;

    for (auto ply = 0; ply < 128; ++ply)
    {
        const auto entry = probe(root.getHashKey());

        // No entry found -> end of PV
        if (!entry) 
            break;

        // No move found in the entry -> cannot add to the PV -> end of PV
        if (!entry->getBestMove()) 
            break;

        // Repetition draw -> end of PV
        if (previousHashes.count(root.getHashKey()) > 0) 
            break;

        // No exact score hash entry to use -> end of PV 
        // If we are very near the root we accept all flags as we absolutely need the move to be played and a ponder move is very important as well.
        if (entry->getFlags() != ExactScore && ply >= 2) 
            break;

        m.setMove(entry->getBestMove());
        pv.push_back(m);
        previousHashes.insert(root.getHashKey());
        root.makeMove(m);
    }

    return pv;
}


