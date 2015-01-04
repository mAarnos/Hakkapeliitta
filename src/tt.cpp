#include "tt.hpp"
#include <cassert>
#include <cmath>
#include <unordered_set>
#include "utils/clamp.hpp"
#include "search.hpp"

TranspositionTable::TranspositionTable():
table(nullptr), tableSize(0)
{
    setSize(32); 
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

void TranspositionTable::prefetch(HashKey hk)
{
    auto address = reinterpret_cast<char*>(&table[hk & (tableSize - 1)]);
#if defined (_MSC_VER) || defined(__INTEL_COMPILER)
    _mm_prefetch(address, _MM_HINT_T0);
#else
    __builtin_prefetch(address);
#endif
}

void TranspositionTable::save(const Position& pos, int ply, const Move& move, int score, int depth, int flags)
{
    assert(ply >= 0 && ply < 128);
    assert(depth >= 0 && depth <= 255);
    assert(score < infinity && score > -infinity);
    assert(flags >= 0 && flags <= 255);

    auto best = move.getMove();
    auto hashEntry = table[pos.getHashKey() & (tableSize - 1)].entries;
    auto replace = hashEntry;

    // We only store pure mate scores so that we can use them in other parts of the search tree too.
    if (isMateScore(score))
    {
        score += (score > 0 ? ply : -ply);
    }

    for (auto i = 0; i < 4; ++i, ++hashEntry)
    {
        if ((hashEntry->getHash() ^ hashEntry->getData()) == pos.getHashKey())
        {
            replace = hashEntry;
            if (!best)
            {
                best = hashEntry->getBestMove();
            }
            break;
        }

        // Here we check if we have found an entry which is worse than the current worse entry.
        // If the entry is from a earlier search or has a smaller depth it is worse and is made the new worst entry.
        auto c1 = (replace->getGeneration() > hashEntry->getGeneration());
        auto c2 = (replace->getDepth() > hashEntry->getDepth());

        if (c1 || c2)
        {
            replace = hashEntry;
        }
    }

    replace->setData((static_cast<uint64_t>(best) | 
                      static_cast<uint64_t>(generation) << 16 | 
                      static_cast<uint64_t>(score & 0xffff) << 32 | 
                      static_cast<uint64_t>(depth & 0xff) << 48) | 
                      static_cast<uint64_t>(flags) << 56);
    replace->setHash(pos.getHashKey() ^ replace->getData());

    assert(replace->getBestMove() == best);
    assert(replace->getGeneration() == generation);
    assert(replace->getScore() == score);
    assert(replace->getDepth() == depth);
    assert(replace->getFlags() == flags);
}

const TranspositionTableEntry* TranspositionTable::probe(const Position& pos) const
{
    const TranspositionTableEntry* hashEntry = &table[pos.getHashKey() & (tableSize - 1)].entries[0];

    for (auto i = 0; i < 4; ++i, ++hashEntry)
    {
        if ((hashEntry->getHash() ^ hashEntry->getData()) == pos.getHashKey())
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
    Move m(0, 0, 0, 0);
    History history;

    for (auto ply = 0; ply < 128; ++ply)
    {
        auto entry = probe(root);

        if (!entry // No entry found
        || (entry->getFlags() != ExactScore && ply >= 2) // Always try to get a move to ponder on.
        || !entry->getBestMove() // No move found in the entry.
        || (previousHashes.count(root.getHashKey()) > 0 && ply >= 2)) // Repetition.
        {
            break;
        }
        m.setMove(entry->getBestMove());
        pv.push_back(m);
        previousHashes.insert(root.getHashKey());
        root.makeMove(m, history);
    }

    return pv;
}


