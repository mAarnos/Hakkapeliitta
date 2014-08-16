#ifndef KILLER_HPP_
#define KILLER_HPP_

#include <cstdint>
#include <array>
#include "move.hpp"

class KillerTable
{
public:
    KillerTable();

    void addKiller(const Move & move, int ply);
    int isKiller(const Move & move, int ply);
private:
    class KillerTableEntry
    {
    public:
        std::array<int32_t, 2> killers;
    };

    std::array<KillerTableEntry, 1200> killers;
};

#endif
