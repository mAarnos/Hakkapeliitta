#include "history.hpp"
#include <cstring>

HistoryTable::HistoryTable()
{
    clear();
}

void HistoryTable::clear()
{
    memset(history, 0, sizeof(history));
}

void HistoryTable::addMove(const Position & pos, const Move & move, int depth)
{
    history[pos.getBoard(move.getFrom())][move.getTo()] += depth * depth;
}

int HistoryTable::getScore(const Position & pos, const Move & move) const
{
    return history[pos.getBoard(move.getFrom())][move.getTo()];
}

