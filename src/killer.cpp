#include "killer.hpp"
#include <cassert>
#include <cstring>

KillerTable::KillerTable()
{
    clear();
}

void KillerTable::clear()
{
    memset(killers, 0, sizeof(killers));
}

void KillerTable::addKiller(const Move& move, int ply)
{
    assert(ply >= 0 && ply < 128);

    auto candidateKiller = move.getMove();
    // Only replace if we won't have two same killers if we replace.
    if (candidateKiller != killers[0][ply]) 
    {
        killers[1][ply] = killers[0][ply];
        killers[0][ply] = candidateKiller;
    }
}

int KillerTable::isKiller(const Move& move, int ply) const
{
    assert(ply >= 0 && ply < 128);

    auto possibleKiller = move.getMove();
    if (possibleKiller == killers[0][ply])
    {
        return 4;
    }
    else if (possibleKiller == killers[1][ply])
    {
        return 3;
    }
    else if (ply > 1 && possibleKiller == killers[0][ply - 2])
    {
        return 2;
    }
    else if (ply > 1 && possibleKiller == killers[1][ply - 2])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


