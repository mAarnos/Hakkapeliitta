#ifndef SEARCH_H_
#define SEARCH_H_

#include "defs.hpp"
#include "position.hpp"

void think();
uint64_t perft(Position & pos, int depth);

const int aspirationWindow = 50;

const int nullReduction = 3;

const int futilityDepth = 4;
const array<int, 1 + futilityDepth> futilityMargin = {
	50, 125, 125, 300, 300, 
};

const int razoringDepth = 3;
const array<int, 1 + razoringDepth> razoringMargin = {
    0, 300, 300, 300
};

const int staticNullMoveDepth = 3;
const array<int, 1 + staticNullMoveDepth> staticNullMoveMargin = {
    0, 260, 445, 900
};

const int lmpDepth = 4;
const array<int, 1 + lmpDepth> lmpMoveCount = {
    0, 4, 8, 16, 32
};

extern int syzygyProbeLimit;

const int fullDepthMoves = 4;
const int reductionLimit = 3;

extern int lastRootScore;
extern int bestRootScore;

#endif