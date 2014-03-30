#ifndef SEARCH_H
#define SEARCH_H

#include "defs.h"
#include "position.h"

uint64_t perft(Position & pos, int depth);
uint64_t perftHash(Position & pos, int depth);

#endif