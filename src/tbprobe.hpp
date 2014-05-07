#ifndef TBPROBE_H_
#define TBPROBE_H_

#include "position.hpp"

extern int TBLargest;

void init(const std::string & path);
int probe_wdl(Position & pos, int * success);
int probe_dtz(Position & pos, int * success);
bool root_probe(Position & pos, int & TBScore, Move * moveStack, int & generatedMoves);
bool root_probe_wdl(Position & pos, int & TBScore, Move * moveStack, int & generatedMoves);

#endif
