#ifndef SEARCH_H
#define SEARCH_H

#include "defs.h"
#include "position.h"


extern uint64_t nodeCount;

void think();
uint64_t perft(Position & pos, int depth);

const int deltaPruningSafetyMargin = 50;
const int aspirationWindow = 50;

#endif