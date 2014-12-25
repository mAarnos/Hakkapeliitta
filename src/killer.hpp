#ifndef KILLER_HPP_
#define KILLER_HPP_

#include <cstdint>
#include <array>
#include "move.hpp"

class KillerTable
{
public:
    KillerTable();

    void addKiller(const Move& move, int ply);
    int isKiller(const Move& move, int ply) const;
    void clear();
private:
    std::array<uint16_t, 128> killers[2];
};

#endif
