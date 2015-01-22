#ifndef KILLER_HPP_
#define KILLER_HPP_

#include <cstdint>
#include <array>
#include "move.hpp"

class KillerTable
{
public:
    KillerTable();

    void addKiller(const Move& move, const int ply);
    int isKiller(const Move& move, const int ply) const;
    void clear();
private:
    std::array<std::array<uint16_t, 2>, 128> killers;
};

#endif
