#ifndef EVAL_HPP_
#define EVAL_HPP_

#include <array>
#include <vector>
#include <unordered_set>
#include "position.hpp"
#include "zobrist.hpp"
// #include "pht.hpp"

class Evaluation
{
public:
    static void initialize();

    static int evaluate(const Position& pos, bool& zugzwangLikely);

    static std::array<std::array<int, 64>, 12> pieceSquareTableOpening;
    static std::array<std::array<int, 64>, 12> pieceSquareTableEnding;
    // static PawnHashTable pawnHashTable;

    static bool drawnEndgame(const HashKey mhk) { return drawnEndgames.count(mhk) > 0; }
private:
    // Things related to endgame knowledge.
    static void initializeDrawnEndgames();
    static std::unordered_set<HashKey> drawnEndgames;

    // Evaluation function in parts.
    template <bool hardwarePopcnt> 
    static int evaluate(const Position& pos, bool& zugzwangLikely);

    template <bool hardwarePopcnt> 
    static int mobilityEval(const Position& pos, std::array<int, 2>& kingSafetyScore, int phase, bool& zugzwangLikely);

    static int pawnStructureEval(const Position& pos, int phase);
    static int kingSafetyEval(const Position& pos, int phase, std::array<int, 2>& kingSafetyScore);
};

#endif