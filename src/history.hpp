#ifndef HISTORY_HPP_
#define HISTORY_HPP_

#include <array>
#include "move.hpp"

class HistoryTable
{
public:
    HistoryTable();

    void addMove(const Move & move, int color, int depth);
    int getScore(const Move & move, int color) const;

    void clear();
private:
    std::array<int, 64> butterfly[2][64];
};

#endif
