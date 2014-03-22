#ifndef TTABLE_H
#define TTABLE_H

#include "defs.h"

const int Invalid = -11111;

// Flags for exact, upperbound and lowerbound scores.
enum { ttExact, ttAlpha, ttBeta };

// We pack the ttEntry so that we can fit as much as possible of them into the TT.
// Maybe test someday whether packing is good or not.
#pragma pack (1)
class ttEntry 
{
	public:
		uint64_t Hash;
		int32_t bestmove;
		int16_t score;
		uint8_t depth;
		uint8_t flags;
}; 
#pragma pack ()

class pttEntry 
{
	public:
		uint64_t Hash;
		int score;
};

extern vector<ttEntry> tt;
extern uint64_t ttSize;

extern vector<pttEntry> ptt;
extern uint64_t pttSize;

void ttSetSize(uint64_t size);
void pttSetSize(uint64_t size);

void ttSave(uint8_t depth, int16_t score, uint8_t flags, int32_t best);
int ttProbe(uint8_t depth, int * alpha, int * beta, int * best);

void pttSave(uint64_t pHash, int32_t score);
int pttProbe(uint64_t pHash);

#endif
