#ifndef TBPROBE_H_
#define TBPROBE_H_

#include "position.hpp"

extern int TBLargest;

void init(string & path);
int probeWDL(Position & pos, int * success);
int probeDTZ(Position & pos, int * success);
bool rootProbe(Position & pos, int & TBScore);
bool rootProbeWDL(Position & pos, int & TBScore);

#endif
