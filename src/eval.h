#ifndef EVAL_H
#define EVAL_H

#include "defs.h"
#include "position.h"

extern int eval(Position & pos);

extern array<int, 12> pieceValuesOpening;
extern array<int, 12> pieceValuesEnding;

#endif