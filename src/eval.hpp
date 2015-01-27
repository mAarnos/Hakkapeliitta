#ifndef EVAL_HPP_
#define EVAL_HPP_

#include <array>
#include <vector>
#include <unordered_set>
#include "position.hpp"
#include "zobrist.hpp"
#include "endgame.hpp"
#include "pht.hpp"

class Evaluation
{
public:
    Evaluation();

    int evaluate(const Position& pos, bool& zugzwangLikely);

    // These two have to be annoyingly static, as we use them in position.cpp to incrementally update the pst eval.
    static std::array<std::array<short, 64>, 12> pieceSquareTableOpening;
    static std::array<std::array<short, 64>, 12> pieceSquareTableEnding;
private:
    // Contains information on some endgames.
    EndgameModule endgameModule;
    // Used for hashing pawn eval scores.
    PawnHashTable pawnHashTable;

    // Evaluation function in parts.
    template <bool hardwarePopcnt> 
    int evaluate(const Position& pos, bool& zugzwangLikely);

    template <bool hardwarePopcnt> 
    int mobilityEval(const Position& pos, std::array<int, 2>& kingSafetyScore, int phase, bool& zugzwangLikely);

    int pawnStructureEval(const Position& pos, int phase);
    int kingSafetyEval(const Position& pos, int phase, std::array<int, 2>& kingSafetyScore);
};

#endif