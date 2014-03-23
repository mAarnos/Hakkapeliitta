#ifndef SEARCH_H
#define SEARCH_H

#include "defs.h"
#include "position.h"

extern uint64_t nodeCount;

int perft(int depth, Position & pos);

#endif