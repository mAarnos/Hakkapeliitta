#ifndef MOVELIST_HPP_
#define MOVELIST_HPP_

#include <array>
#include <cassert>
#include "move.hpp"

class MoveList
{
public:
    MoveList() : numberOfMoves(0) {};

    void push_back(const Move & move) { moveList[numberOfMoves++] = move; }
    Move & at(size_t index) { assert(index < numberOfMoves); return moveList[index]; }
    void clear() { numberOfMoves = 0; };
    size_t size() { return numberOfMoves; }
private:
    std::array<Move, 218> moveList;
    size_t numberOfMoves;
};

#endif
