#ifndef TTABLE_H
#define TTABLE_H

#include "defs.h"

const int Invalid = -11111;

// flags for exact, upperbound and lowerbound scores
enum { ttExact, ttAlpha, ttBeta };

// we pack the ttEntry so that we can fit as much as possible of them into the TT
#pragma pack (1)
struct ttEntry {
	U64 Hash;
	int bestmove;
	__int16 score;
	unsigned __int8 depth;
	unsigned __int8 flags;
}; 
#pragma pack ()

struct pttEntry {
	U64 Hash;
	int score;
};

extern ttEntry * tt;
extern int ttSize;

extern pttEntry * ptt;
extern int pttSize;

extern int ttSetSize(int size);
extern int pttSetSize(int size);

extern void ttSave(unsigned __int8 depth, __int16 score, unsigned __int8 flags, int best);
extern int ttProbe(unsigned __int8 depth, int * alpha, int * beta, int * best, bool * ttAllowNull);

extern void pttSave(U64 pHash, int score);
extern int pttProbe(U64 pHash);

#endif
