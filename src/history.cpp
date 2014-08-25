#include "history.hpp"
#include <cstring>

HistoryTable::HistoryTable()
{
    clear();
}

void HistoryTable::clear()
{
    memset(butterfly, 0, sizeof(butterfly));
}

void HistoryTable::addMove(const Move & move, int color, int depth)
{
    butterfly[color][move.getFrom()][move.getTo()] = depth * depth;
}

int HistoryTable::getScore(const Move & move, int color) const
{
    return butterfly[color][move.getFrom()][move.getTo()];
}

