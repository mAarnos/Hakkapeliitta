#ifndef SEARCH_HPP_
#define SEARCH_HPP_

#include <vector>
#include <cstdint>
#include "movelist.hpp"
#include "position.hpp"
#include "history.hpp"
#include "killer.hpp"

const int maxPly = 1200;
const int mateScore = 32767;
const int maxMateScore = 32767 - maxPly;
const int infinity = mateScore + 1;

inline int matedInPly(int ply)
{
    return (-mateScore + ply);
}

inline int mateInPly(int ply)
{
    return (mateScore - ply);
}

inline bool isMateScore(int score)
{
    assert(score < infinity && score > -infinity);
    return (score <= -maxMateScore || score >= maxMateScore);
}

class Search
{
public:
    static void initialize();

    static int qSearch(Position & pos, int ply, int alpha, int beta, bool inCheck);

    static std::array<int, 2> contempt;

    // Delete these, only used for tuning.
    static std::array<Move, 32> pv[32];
    static std::array<int, 32> pvLength;
private:
    static HistoryTable historyTable;
    static KillerTable killerTable;

    static const int aspirationWindow;
    static const int nullReduction;
    static const int futilityDepth;
    static const std::array<int, 1 + 4> futilityMargins;
    static const int lmrFullDepthMoves;
    static const int lmrReductionLimit;
    static std::array<int, 256> lmrReductions;
    static const int lmpDepth;
    static const std::array<int, 1 + 4> lmpMargins;

    // Move ordering scores.
    static const int16_t hashMove;
    static const int16_t captureMove;
    static const std::array<int16_t, 1 + 4> killerMove;

    static void orderMoves(const Position & pos, MoveList & moveList, const Move & ttMove, int ply);
    static void orderCaptures(const Position & pos, MoveList & moveList);
    static void selectMove(MoveList & moveList, int currentMove);
};

#endif