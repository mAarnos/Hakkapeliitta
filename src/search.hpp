#ifndef SEARCH_H_
#define SEARCH_H_

#include "defs.hpp"
#include "position.hpp"

void think();
uint64_t perft(Position & pos, int depth);

const int deltaPruningSafetyMargin = 50;
const int aspirationWindow = 50;

extern int syzygyProbeLimit;

#endif