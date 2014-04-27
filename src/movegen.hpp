#ifndef MOVEGEN_H_
#define MOVEGEN_H_

#include "defs.hpp"
#include "move.hpp"
#include "position.hpp"

int generateMoves(Position & pos, Move * mlist);
int generateCaptures(Position & pos, Move * mlist);
int generateEvasions(Position & pos, Move * mlist);

#endif
