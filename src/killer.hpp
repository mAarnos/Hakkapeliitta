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
    void clear();
private:
    std::array<int16_t, 1200> killers[2];
};

#endif
