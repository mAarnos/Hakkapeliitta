#ifndef EVAL_HPP_
#define EVAL_HPP_

#include <array>
#include <vector>
#include <unordered_map>
#include "position.hpp"
#include "zobrist.hpp"
#include "pht.hpp"

class Evaluation
{
public:
    static void initialize();

    static int evaluate(const Position & pos);

    static std::array<int, 64> pieceSquareTableOpening[12];
    static std::array<int, 64> pieceSquareTableEnding[12];

    static std::unordered_map<HashKey, int> drawnEndgames;
    static PawnHashTable pawnHashTable;
private:
    static const std::array<int, 6> pieceValuesOpening;
    static const std::array<int, 6> pieceValuesEnding;

    static const std::array<int, 64> openingPST[6];
    static const std::array<int, 64> endingPST[6];

    static const std::vector<int> mobilityOpening[6];
    static const std::vector<int> mobilityEnding[6];

    static const std::array<int, 6> attackWeight;
    static const std::array<int, 100> kingSafetyTable;

    static const std::array<int, 8> passedBonusOpening;
    static const std::array<int, 8> passedBonusEnding;
    static const std::array<int, 8> doubledPenaltyOpening;
    static const std::array<int, 8> doubledPenaltyEnding;
    static const std::array<int, 8> isolatedPenaltyOpening;
    static const std::array<int, 8> isolatedPenaltyEnding;
    static const std::array<int, 8> backwardPenaltyOpening;
    static const std::array<int, 8> backwardPenaltyEnding;

    static const int bishopPairBonusOpening;
    static const int bishopPairBonusEnding;
    static const int sideToMoveBonus;

    static void initializeDrawnEndgames();

    template <bool hardwarePopcnt> 
    static int evaluate(const Position & pos);

    template <bool hardwarePopcnt> 
    static int mobilityEval(const Position & pos, std::array<int, 2> & kingSafetyScore, int phase);

    static int pawnStructureEval(const Position & pos, int phase);
    static int kingSafetyEval(const Position & pos, int phase, std::array<int, 2> & kingSafetyScore);

    template <bool side>
    static int evaluatePawnShelter(const Position & pos);

    static int interpolateScore(int scoreOp, int scoreEd, int phase) 
    {
        return ((scoreOp * (64 - phase)) + (scoreEd * phase)) / 64;
    }
};

#endif