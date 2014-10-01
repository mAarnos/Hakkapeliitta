#ifndef MOVEGEN_HPP_
#define MOVEGEN_HPP_

#include <vector>
#include "position.hpp"
#include "move.hpp"
#include "movelist.hpp"

class MoveGen
{
public:
    static void generatePseudoLegalMoves(Position & pos, MoveList & moves);
    static void generatePseudoLegalCaptureMoves(Position & pos, MoveList & moves);
private:
    template <bool side> 
    static void generatePseudoLegalMoves(Position & pos, MoveList & moves);

    template <bool side>
    static void generatePseudoLegalCaptureMoves(Position & pos, MoveList & moves);
};

#endif