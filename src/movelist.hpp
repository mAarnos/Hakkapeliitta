#ifndef MOVELIST_HPP_
#define MOVELIST_HPP_

#include <array>
#include <type_traits>
#include <cassert>
#include "move.hpp"

class MoveList
{
public:
    MoveList() : numberOfMoves(0) {};

    // Perfect forwarder for pushing both lvalues and rvalues with the same efficiency.
    // Why? Because I could.
    template<class T, class = std::enable_if<std::is_same<T, Move>::value>>
    void push_back(T && move) 
    {  
        moveList[numberOfMoves++] = std::forward<T>(move); 
    }

    Move & operator[](int index) { return moveList[index]; }
    const Move & operator[](int index) const { return moveList[index]; }
    void clear() { numberOfMoves = 0; };
    int size() const { return numberOfMoves; }
    bool empty() const { return numberOfMoves == 0; }
private:
    std::array<Move, 218> moveList;
    int numberOfMoves;
};

#endif
