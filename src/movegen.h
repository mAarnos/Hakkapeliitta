#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"
#include "move.h"
#include "position.h"

int generateMoves(Position & pos, Move * mlist);
int generateCaptures(Position & pos, Move * mlist);

#endif
