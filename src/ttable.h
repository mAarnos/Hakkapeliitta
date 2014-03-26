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
		uint64_t hash;
		// Change this to one uint64_t data with all the stuff below meshed in. This way we don't have to use #pragma pack and we can make the hashtable lockless.
		int32_t bestmove;
		int16_t score;
		uint8_t depth;
		uint8_t flags;
}; 
#pragma pack ()

class pttEntry 
{
	public:
		uint64_t hash;
		int score;
};

extern vector<ttEntry> tt;
extern uint64_t ttSize;

extern vector<pttEntry> ptt;
extern uint64_t pttSize;

extern void ttSetSize(uint64_t size);
extern void pttSetSize(uint64_t size);

extern void ttSave(uint8_t depth, int16_t score, uint8_t flags, int32_t best);
extern int ttProbe(uint8_t depth, int * alpha, int * beta, int * best);

extern void pttSave(uint64_t pHash, int32_t score);
extern int pttProbe(uint64_t pHash);

#endif
