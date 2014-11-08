#include "history.hpp"
#include <cstring>

HistoryTable::HistoryTable()
{
    clear();
}

void HistoryTable::clear()
{
    memset(history, 0, sizeof(history));
    memset(butterfly, 0, sizeof(butterfly));
}

void HistoryTable::addCutoff(const Position& pos, const Move& move, int depth)
{
    history[pos.getBoard(move.getFrom())][move.getTo()] += depth * depth;
}

void HistoryTable::addNotCutoff(const Position& pos, const Move& move, int depth)
{
    butterfly[pos.getBoard(move.getFrom())][move.getTo()] += depth * depth;
}

int HistoryTable::getScore(const Position& pos, const Move& move) const
{
    return (history[pos.getBoard(move.getFrom())][move.getTo()] * 100 / 
            (butterfly[pos.getBoard(move.getFrom())][move.getTo()] + 1));
}

void HistoryTable::age()
{
    for (auto i = 0; i < 12; ++i)
    {
        for (auto j = 0; j < 64; ++j)
        {
            history[i][j] /= 2;
            butterfly[i][j] /= 2;
        }
    }
}

