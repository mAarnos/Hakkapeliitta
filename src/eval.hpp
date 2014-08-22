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
private:
    static PawnHashTable pawnHashTable;

    static std::array<int, 6> pieceValuesOpening;
    static std::array<int, 6> pieceValuesEnding;

    static std::array<int, 64> openingPST[6];
    static std::array<int, 64> endingPST[6];

    static std::vector<int> mobilityOpening[6];
    static std::vector<int> mobilityEnding[6];

    const static std::array<int, 64> flip;

    static void initializeKnownEndgames();
    static std::unordered_map<HashKey, int> knownEndgames;

    template <bool hardwarePopcntEnabled> static int evaluate(const Position & pos);
    template <bool hardwarePopcntEnabled> static int mobilityEval(const Position & pos, int phase);
};

#endif