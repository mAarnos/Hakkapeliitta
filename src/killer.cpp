#include "killer.hpp"
#include <cassert>
#include <cstring>

KillerTable::KillerTable()
{
    clear();
}

void KillerTable::clear()
{
    for (auto i = 0; i < 128; ++i)
    {
        killers[i].fill(0);
    }
}

void KillerTable::addKiller(const Move& move, int ply)
{
    auto candidateKiller = move.getMove();
    // Only replace if we won't have two same killers if we replace.
    if (candidateKiller != killers[ply][0])
    {
        killers[ply][1] = killers[ply][0];
        killers[ply][0] = candidateKiller;
    }
}

int KillerTable::isKiller(const Move& move, int ply) const
{
    auto possibleKiller = move.getMove();
    if (possibleKiller == killers[ply][0])
    {
        return 1;
    }
    else if (possibleKiller == killers[ply][1])
    {
        return 2;
    }
    else if (ply > 1 && possibleKiller == killers[ply - 2][0])
    {
        return 3;
    }
    else if (ply > 1 && possibleKiller == killers[ply - 2][1])
    {
        return 4;
    }
    else
    {
        return 0;
    }
}


