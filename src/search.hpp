#ifndef SEARCH_H_
#define SEARCH_H_

#include "defs.hpp"
#include "position.hpp"

void think();
uint64_t perft(Position & pos, int depth);

const int aspirationWindow = 50;

const int futilityDepth = 4 * onePly;
const array<int, 1 + futilityDepth> futilityMargin = {
	50, 125, 125, 125, 125, 300, 300, 300, 300
};

extern int syzygyProbeLimit;

#endif