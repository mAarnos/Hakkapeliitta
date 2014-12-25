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

    auto tableSize = ((sizeInMegaBytes * 1024 * 1024) / sizeof(PawnHashTableEntry));
    table.resize(tableSize);
}

void PawnHashTable::clear()
{
    auto tableSize = table.size();
    table.clear();
    table.resize(tableSize);
}

void PawnHashTable::save(const Position& pos, int scoreOp, int scoreEd)
{
    auto& hashEntry = table[pos.getPawnHashKey() & (table.size() - 1)];

    hashEntry.setData((scoreOp & 0xffff) | (scoreEd << 16));
    hashEntry.setHash(static_cast<uint32_t>(pos.getPawnHashKey()) ^ hashEntry.getData());

    assert(static_cast<int16_t>(hashEntry.getData()) == scoreOp);
    assert(static_cast<int16_t>(hashEntry.getData() >> 16) == scoreEd);
}

bool PawnHashTable::probe(const Position& pos, int& scoreOp, int& scoreEd) const
{
    const auto& hashEntry = table[pos.getPawnHashKey() & (table.size() - 1)];

    if ((hashEntry.getHash() ^ hashEntry.getData()) == static_cast<uint32_t>(pos.getPawnHashKey()))
    {
        scoreOp = static_cast<int16_t>(hashEntry.getData());
        scoreEd = static_cast<int16_t>(hashEntry.getData() >> 16);
        return true;
    }

    return false;
}
