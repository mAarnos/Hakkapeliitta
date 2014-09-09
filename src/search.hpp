#ifndef SEARCH_HPP_
#define SEARCH_HPP_

#include "position.hpp"
#include "history.hpp"
#include "killer.hpp"
#include <vector>

class Search
{
public:
    static int qSearch(Position & pos, int ply, int alpha, int beta);
    static std::array<Move, 32> pv[32];
    static std::array<int, 32> pvLength;
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