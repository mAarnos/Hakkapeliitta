#ifndef SEARCH_H
#define SEARCH_H

#include "bitboard.h"
#include "board.h"
#include "movegen.h"
#include "magic.h"

// triangular principal variation array
extern Move pv[600][600];
extern int pvLength[600];
extern bool followpv;

// stores the latest pv the search has returned
extern int lastpvLength;
extern Move lastpv[600];

extern bool allowNullMove;

const int deltaPruningSafetyMargin = 50;

const int aspirationWindow = 50;

extern string numberToNotation[64];
extern string numberToPromotion[16];

extern bool useTB;
extern int tbhits;

void think();
int perft(int depth);
int alphabetaPVS(int depth, int alpha, int beta);
extern U64 nodeCount;
extern int repetitionCount();

const int onePly = 2;

#endif