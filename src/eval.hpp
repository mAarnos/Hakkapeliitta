#ifndef EVAL_HPP_
#define EVAL_HPP_

#include <array>
#include <unordered_map>
#include "position.hpp"
#include "zobrist.hpp"

class Evaluation
{
public:
    static void initialize();

    static int evaluate(const Position & pos);

    static std::array<int, 64> pieceSquareTableOpening[12];
    static std::array<int, 64> pieceSquareTableEnding[12];
private:
    static void initializeKnownEndgames();

    static std::array<int, 6> pieceValuesOpening;
    static std::array<int, 6> pieceValuesEnding;

    static std::array<int, 64> openingPST[6];
    static std::array<int, 64> endingPST[6];

    static std::array<int, 64> mobilityOpening[6];
    static std::array<int, 64> mobilityEnding[6];

    static std::unordered_map<HashKey, int> knownEndgames;

    template <bool hardwarePopcntEnabled> static int evaluate(const Position & pos);
};

#endif