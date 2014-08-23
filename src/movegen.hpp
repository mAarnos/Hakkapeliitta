#ifndef MOVEGEN_HPP_
#define MOVEGEN_HPP_

#include <vector>
#include "position.hpp"
#include "move.hpp"

class MoveGen
{
public:
    static void generatePseudoLegalMoves(Position & pos, std::vector<Move> & moves);
    static void generatePseudoLegalCaptureMoves(Position & pos, std::vector<Move> & moves);
private:
    template <bool side> 
    static void generatePseudoLegalMoves(Position & pos, std::vector<Move> & moves);

    template <bool side>
    static void generatePseudoLegalCaptureMoves(Position & pos, std::vector<Move> & moves);
};

#endif