#include "pht.hpp"
#include <cassert>

PawnHashTable::PawnHashTable()
{
    auto sizeInBytes = 1024 * 1024 * 4; // 4MB
    auto sizeInEntries = sizeInBytes / sizeof(PawnHashTableEntry);
    table.resize(sizeInEntries); 
}

void PawnHashTable::clear()
{
    auto tableSize = table.size();
    table.clear();
    table.resize(tableSize);
}

void PawnHashTable::save(const Position & pos, int scoreOp, int scoreEd)
{
    auto & hashEntry = table[pos.getPawnHashKey() % table.size()];

    hashEntry.setData((scoreOp & 0xffff) | (scoreEd << 16));
    hashEntry.setHash(static_cast<uint32_t>(pos.getPawnHashKey()) ^ hashEntry.getData());

    assert(static_cast<int16_t>(hashEntry->getData()) == scoreOp);
    assert(static_cast<int16_t>(hashEntry->getData() >> 16) == scoreEd);
}

bool PawnHashTable::probe(const Position & pos, int & scoreOp, int & scoreEd) const
{
    auto & hashEntry = table[pos.getPawnHashKey() % table.size()];

    if ((hashEntry.getHash() ^ hashEntry.getData()) == static_cast<uint32_t>(pos.getPawnHashKey()))
    {
        scoreOp = static_cast<int16_t>(hashEntry.getData());
        scoreEd = static_cast<int16_t>(hashEntry.getData() >> 16);
        return true;
    }

    return false;
}
