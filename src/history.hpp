#ifndef HISTORY_HPP_
#define HISTORY_HPP_

#include <array>
#include "move.hpp"
#include "position.hpp"

class HistoryTable
{
public:
    HistoryTable();

    void addCutoff(const Position & pos, const Move & move, int depth);
    void addNotCutoff(const Position & pos, const Move & move, int depth);
    int getScore(const Position & pos, const Move & move) const;
    void clear();
    void age();
private:
    std::array<int, 64> history[12];
    std::array<int, 64> butterfly[12];
};

#endif
