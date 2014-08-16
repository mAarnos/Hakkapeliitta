#include "killer.hpp"
#include <cassert>

KillerTable::KillerTable()
{
    for (auto & entry: killers)
    {
        entry.killers.fill(0);
    }
}

void KillerTable::addKiller(const Move & move, int ply)
{
    assert(ply >= 0 && ply < static_cast<int>(killers.size()));

    int32_t candidateKiller = move.getPacket();
    // Only replace if we won't have two same killers.
    if (candidateKiller != killers[ply].killers[0]) 
    {
        killers[ply].killers[1] = killers[ply].killers[0];
        killers[ply].killers[0] = candidateKiller;
    }
}

int KillerTable::isKiller(const Move & move, int ply)
{
    assert(ply >= 0 && ply < static_cast<int>(killers.size()));

    int32_t possibleKiller = move.getPacket();
    if (possibleKiller == killers[ply].killers[0])
    {
        return 4;
    }
    else if (possibleKiller == killers[ply].killers[1])
    {
        return 3;
    }
    else if (ply > 1 && killers[ply - 2].killers[0])
    {
        return 2;
    }
    else if (ply > 1 && killers[ply - 2].killers[1])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


