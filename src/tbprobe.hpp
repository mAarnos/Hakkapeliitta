#ifndef TBPROBE_H_
#define TBPROBE_H_

#include "position.hpp"

int probe_wdl(Position & pos, int * success);
int probe_dtz(Position & pos, int * success);
bool root_probe(Position & pos, int & TBScore, Move * moveStack, int & generatedMoves);
bool root_probe_wdl(Position & pos, int & TBScore, Move * moveStack, int & generatedMoves);

#endif
