#ifndef KILLER_HPP_
#define KILLER_HPP_

#include <cstdint>
#include <array>
#include "move.hpp"

// Killer moves for the search function encapsulated.
class KillerTable
{
public:
    KillerTable();

    // Add a killer move for the given ply. Assumes that the move is not a capture or a promotion.
    void addKiller(const Move& move, int ply);
    // Checks if the given move is a killer move. Returns 0 if it is not and 1-4 in case it is (1 is best, 4 is worst). 
    int isKiller(const Move& move, int ply) const;
    // Clears the killer table.
    void clear();
private:
    std::array<std::array<uint16_t, 2>, 128> killers;
};

#endif
