#ifndef SEARCH_HPP_
#define SEARCH_HPP_

#include "position.hpp"
#include "history.hpp"
#include "killer.hpp"

class Search
{
public:
    static int qSearch(Position & pos, int alpha, int beta);
private:
    HistoryTable historyTable;
    KillerTable killerTable;

    static const int aspirationWindow;
    static const int nullReduction;
    static const int futilityDepth;
    static const std::array<int, 1 + 4> futilityMargins;
    static const int lmrFullDepthMoves;
    static const int lmrReductionLimit;
};



#endif