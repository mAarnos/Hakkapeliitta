#include "pht.hpp"
#include <cassert>

PawnHashTable::PawnHashTable()
{
    auto sizeInBytes = 1024 * 1024 * 4; // 4MB
    // Here, just like with the transposition table, - 1 could avoid some nasty things.
    auto sizeInEntries = sizeInBytes / sizeof(PawnHashTableEntry) - 1;
    table.resize(sizeInEntries); 
}

void PawnHashTable::clear()
{
    auto tableSize = table.size();
    table.clear();
    table.resize(tableSize);
}

// Pawn hash table is broken, should save scoreOp and scoreEd but for some reason this is -5 elo.
// Retest in 2015.
void PawnHashTable::save(const Position & pos, int score)
{
    // assert(score < infinity && score > -infinity);
    assert(table.size() > 0);

    auto hashEntry = &table[pos.getPawnHashKey() % table.size()];

    hashEntry->setData(score);
    hashEntry->setHash(static_cast<uint32_t>(pos.getPawnHashKey()) ^ hashEntry->getData());

    assert(static_cast<int>(hashEntry->getData()) == score);
}

bool PawnHashTable::probe(const Position & pos, int & score) const
{
    assert(table.size() > 0);

    auto hashEntry = &table[pos.getPawnHashKey() % table.size()];

    if ((hashEntry->getHash() ^ hashEntry->getData()) == static_cast<uint32_t>(pos.getPawnHashKey()))
    {
        score = hashEntry->getData();
        return true;
    }

    return false;
}
