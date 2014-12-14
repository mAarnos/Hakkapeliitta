#ifndef SEARCH_HPP_
#define SEARCH_HPP_

#include <vector>
#include <cstdint>
#include <cassert>
#include "movelist.hpp"
#include "position.hpp"
#include "history.hpp"
#include "killer.hpp"
#include "tt.hpp"
#include "utils\stopwatch.hpp"

const int mateScore = 32767; // mate in 0
const int maxMateScore = 32767 - 1000; // mate in 500
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

    static void think(const Position& pos);

    // UCI-protocol uses these to communicate things to the search function.
    static int contemptValue;
    static bool searching;
    static bool pondering;
    static bool infinite;
    static int targetTime;
    static int maxTime;

    static TranspositionTable transpositionTable;
    static HistoryTable historyTable;
    static KillerTable killerTable;

    static int quiescenceSearch(Position& pos, int ply, int alpha, int beta, bool inCheck);
private:
    template <bool pvNode>
    static int search(Position& pos, int depth, int ply, int alpha, int beta, int allowNullMove, bool inCheck);

    static std::array<HashKey, 128> repetitionHashes;
    static std::array<int, 2> contempt;

    // Pruning margins and depth limits.
    static const int aspirationWindow;
    static const int nullReduction;
    static const int futilityDepth;
    static const std::array<int, 1 + 4> futilityMargins;
    static const int reverseFutilityDepth;
    static const std::array<int, 1 + 3> reverseFutilityMargins;
    static const int lmrFullDepthMoves;
    static const int lmrReductionLimit;
    static std::array<int, 256> lmrReductions;
    static const int lmpDepth;
    static const std::array<int, 1 + 4> lmpMoveCount;
    static const int razoringDepth;
    static const std::array<int, 1 + 3> razoringMargins;

    // Move ordering scores.
    static const int16_t hashMoveScore;
    static const int16_t captureMoveScore;
    static const std::array<int16_t, 1 + 4> killerMoveScore;

    // Functions for move ordering.
    static void orderMoves(const Position& pos, MoveList& moveList, const Move& ttMove, int ply);
    static void orderCaptures(const Position& pos, MoveList& moveList);
    static void selectMove(MoveList& moveList, int currentMove);

	// Statistics during search.
	static int tbHits;
    static int nodeCount;
    static int nodesToTimeCheck;
    static int selDepth;
	static Stopwatch sw;

	// Functions for printing info during search.
	// Converts a move to algebraic notation.
	static std::string algebraicMove(const Move& move);
	// Same but for a list of moves.
	static std::string algebraicMoves(const std::vector<Move>& moves);
	static void infoCurrMove(const Move& move, int depth, int nr);
	static void infoPv(const std::vector<Move>& moves, int depth, int score, int flags);

    static bool repetitionDraw(const Position& pos, int ply);
};

#endif