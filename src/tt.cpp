#include "tt.hpp"
#include <cassert>
#include <cmath>
#include <unordered_set>
#include "utils\clamp.hpp"
#include "search.hpp"

TranspositionTable::TranspositionTable()
{
    setSize(32); 
}

void TranspositionTable::setSize(size_t sizeInMegaBytes)
{
    // Clear the tt completely to avoid any funny business.
    table.clear();
    generation = 1;

    // If size is not a power of two make it the biggest power of two smaller than size.
    if (sizeInMegaBytes & (sizeInMegaBytes - 1))
    {
        sizeInMegaBytes = static_cast<int>(std::pow(2, std::floor(std::log2(sizeInMegaBytes))));
    }

    auto tableSize = ((sizeInMegaBytes * 1024 * 1024) / sizeof(TranspositionTableEntry));
    table.resize(tableSize);
}

void TranspositionTable::clear()
{
    auto tableSize = table.size();
    table.clear();
    table.resize(tableSize);
    generation = 1;
}

void TranspositionTable::prefetch(HashKey hk)
{
    auto address = reinterpret_cast<char*>(&table[hk & (table.size() - 1)]);
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
    auto& hashEntry = table[pos.getHashKey() & (table.size() - 1)];

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

bool TranspositionTable::probe(const Position& pos, int ply, Move& move, int& score, int depth, int& alpha, int& beta) const
{
    assert(ply >= 0 && ply < 128);

    const auto& hashEntry = table[pos.getHashKey() & (table.size() - 1)];

    for (auto entry = 0; entry < 4; ++entry)
    {
        // TODO: Add avoid nullmove trick here.
        if ((hashEntry.getHash(entry) ^ hashEntry.getData(entry)) == pos.getHashKey())
        {
            move.setMove(hashEntry.getBestMove(entry));
            auto hashScore = hashEntry.getScore(entry);
            auto hashDepth = hashEntry.getDepth(entry);
            auto flags = hashEntry.getFlags(entry);

            if (hashDepth >= depth)
            {
                // Correct the mate score back.
                if (isMateScore(hashScore))
                {
                    hashScore += static_cast<int16_t>(hashScore > 0 ? -ply : ply);
                    assert(hashScore < infinity && hashScore > -infinity);
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

            break; 
        }
    }

    return false;
}

std::vector<Move> TranspositionTable::extractPv(Position root) const
{
    std::vector<Move> pv;
    std::unordered_set<HashKey> previousHashes;
    Move m(0, 0, 0, 0);
    History history;

    for (auto ply = 0; ply < 128; ++ply)
    {
        const auto& hashEntry = table[root.getHashKey() & (table.size() - 1)];

        int entry;
        for (entry = 0; entry < 4; ++entry)
        {
            if ((hashEntry.getHash(entry) ^ hashEntry.getData(entry)) == root.getHashKey())
            {
                break;
            }
        }

        if ((entry > 3) // No entry found
        || (hashEntry.getFlags(entry) != ExactScore && ply >= 2) // Always try to get a move to ponder on.
        || !hashEntry.getBestMove(entry) // No move found in the entry.
        || previousHashes.count(root.getHashKey()) > 0 && ply >= 2) // Repetition.
        {
            break;
        }
        m.setMove(hashEntry.getBestMove(entry));
        pv.push_back(m);
        previousHashes.insert(root.getHashKey());
        root.makeMove(m, history);
    }

    assert(pv.size() >= 1);
    return pv;
}


