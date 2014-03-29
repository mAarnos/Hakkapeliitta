#ifndef TTABLE_H
#define TTABLE_H

#include "defs.h"
#include "position.h"

const uint32_t probeFailed = UINT_MAX;

// Flags for exact, upperbound and lowerbound scores.
enum { ttExact, ttAlpha, ttBeta };

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
		int32_t score;
}; 

class perftTTEntry
{
	public:
		uint64_t hash;
		uint64_t data;
};

template <class t>
class HashTable
{
	public:
		void setSize(uint64_t size)
		{
			table.clear();

			// If size is not a power of two make it the biggest power of two smaller than size.
			if (size & (size - 1))
			{
				double power = floor(log2(size));
				size = (uint64_t)pow(2, power);
			}

			// If size is too small to hold even a single entry do nothing.
			if (size < sizeof(t))
			{
				tableSize = 0;
				return;
			}

			tableSize = (size / sizeof(t)) - 1;
			table.resize(tableSize);
		}

		inline uint64_t getTableSize() { return tableSize; }
		inline t& getEntry(uint64_t entry) { return table[entry]; }
	private:
		vector<t> table;
		uint64_t tableSize;
};

extern vector<ttEntry> tt;
extern uint64_t ttSize;

extern vector<pttEntry> ptt;
extern uint64_t pttSize;

extern vector<perftTTEntry> perftTT;
extern uint64_t perftTTSize;

extern void ttSetSize(uint64_t size);
extern void ttSave(uint64_t hash, uint8_t depth, int16_t score, uint8_t flags, int32_t best);
extern int ttProbe(uint64_t hash, uint8_t depth, int * alpha, int * beta, int * best);

extern void pttSetSize(uint64_t size);
extern void pttSave(uint64_t pHash, int32_t score);
extern int pttProbe(uint64_t pHash);

extern void perftTTSetSize(uint64_t size);
extern void perftTTSave(Position & pos, uint64_t nodes, int depth);
extern uint64_t perftTTProbe(Position & pos, int depth);

#endif
