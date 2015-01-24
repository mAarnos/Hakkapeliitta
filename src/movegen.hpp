#ifndef MOVEGEN_HPP_
#define MOVEGEN_HPP_

#include <vector>
#include "position.hpp"
#include "move.hpp"
#include "movelist.hpp"

class MoveGen
{
public:
    void generatePseudoLegalMoves(const Position& pos, MoveList& moves);
    void generatePseudoLegalCaptures(const Position& pos, MoveList& moves);
    void generateLegalEvasions(const Position& pos, MoveList& moves);
private:
    template <bool side> 
    void generatePseudoLegalMoves(const Position& pos, MoveList& moves);

    template <bool side>
    void generatePseudoLegalCaptures(const Position& pos, MoveList& moves);

    template <bool side>
    void generateLegalEvasions(const Position& pos, MoveList& moves);
};

#endif
