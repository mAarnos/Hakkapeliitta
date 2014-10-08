#include "tt.hpp"
#include <cassert>
#include <cmath>
#include "search.hpp"

TranspositionTable::TranspositionTable(size_t sizeInBytes)
{
    setSize(sizeInBytes);
}

void TranspositionTable::save(const Position & pos, int ply, const Move & move, int score, int depth, int flags)
{
    assert(table.size() > 0);
    assert(depth >= 0 && depth <= 255);
    assert(score < infinity && score > -infinity);
    assert(flags >= 0 && flags <= 255);

    auto best = move.getMove();
    auto & hashEntry = table[pos.getHashKey() & (table.size() - 1)];

    // We only store pure mate scores so that we can use them in other parts of the search tree too.
    if (isMateScore(score))
    {
        score += (score > 0 ? ply : -ply);
    }

    auto replace = 0;
    for (auto i = 0; i < 4; ++i)
    {
        if ((hashEntry.getHash(i) ^ hashEntry.getData(i)) == pos.getHashKey())
        {
            replace = i;
            if (!best)
            {
                best = hashEntry.getBestMove(replace);
            }
            break;
        }

        // Here we check if we have found an entry which is worse than the current worse entry.
        // If the entry is from a earlier search or has a smaller depth it is worse and is made the new worst entry.
        auto c1 = (hashEntry.getGeneration(replace) > hashEntry.getGeneration(i));
        auto c2 = (hashEntry.getDepth(replace) > hashEntry.getDepth(i));

        if (c1 || c2)
        {
            replace = i;
        }
    }

    hashEntry.setData(replace, (static_cast<uint64_t>(best) | 
                                static_cast<uint64_t>(generation) << 16 | 
                                static_cast<uint64_t>(score & 0xffff) << 32 | 
                                static_cast<uint64_t>(depth & 0xff) << 48) | 
                                static_cast<uint64_t>(flags) << 56);
    hashEntry.setHash(replace, pos.getHashKey() ^ hashEntry.getData(replace));

    assert(hashEntry.getBestMove(replace) == best);
    assert(hashEntry.getGeneration(replace) == generation);
    assert((hashEntry.getScore(replace)) == score);
    assert(hashEntry.getDepth(replace) == depth);
    assert(hashEntry.getFlags(replace) == flags);
}

bool TranspositionTable::probe(const Position & pos, int ply, Move & move, int & score, int depth, int & alpha, int & beta) const
{
    const auto & hashEntry = table[pos.getHashKey() & (table.size() - 1)];

    int entry;
    for (entry = 0; entry < 4; ++entry)
    {
        if ((hashEntry.getHash(entry) ^ hashEntry.getData(entry)) == pos.getHashKey())
        {
            break;
        }
    }

    if (entry < 4)
    {
        move.setMove(hashEntry.getBestMove(entry));
        auto hashScore = hashEntry.getScore(entry);
        auto hashDepth = hashEntry.getDepth(entry);
        auto flags = hashEntry.getFlags(entry);

        if (hashDepth >= depth)
        {
            // Correct the mate score back.
            if (isMateScore(score))
            {
                score += (score > 0 ? -ply : ply);
            }

            if (flags == ExactScore)
            {
                score = hashScore;
                return true;
            }

            if (flags == UpperBoundScore)
            {
                if (hashScore <= alpha)
                {
                    score = hashScore;
                    return true;
                }
                if (hashScore < beta)
                {
                    beta = hashScore;
                    return false;
                }
            }
            else if (flags == LowerBoundScore)
            {
                if (hashScore >= beta)
                {
                    score = hashScore;
                    return true;
                }
                if (hashScore > alpha)
                {
                    alpha = hashScore;
                    return false;
                }
            }
        }
    }

    return false;
}

void TranspositionTable::setSize(size_t sizeInBytes)
{
    // Clear the tt completely to avoid any funny business.
    table.clear();
    generation = 0;

    // If size is not a power of two make it the biggest power of two smaller than size.
    if (sizeInBytes & (sizeInBytes - 1))
    {
        sizeInBytes = static_cast<size_t>(pow(2, floor(log2(sizeInBytes))));
    }

    // Do not allow tt sizes of less than 1MB.
    if (sizeInBytes < 1024 * 1024)
    {
        sizeInBytes = 1024 * 1024;
    }

    auto tableSize = (sizeInBytes / sizeof(TranspositionTableEntry));
    table.resize(tableSize);
}

void TranspositionTable::clear()
{
    auto tableSize = table.size();
    table.clear();
    table.resize(tableSize);
    generation = 0;
}

void TranspositionTable::prefetch(HashKey hk)
{
    auto address = reinterpret_cast<char *>(&table[hk & (table.size() - 1)]);
#if defined (_MSC_VER) || defined(__INTEL_COMPILER)
    _mm_prefetch(address, _MM_HINT_T0);
#else
    __builtin_prefetch(address);
#endif
}
