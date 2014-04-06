#ifndef TTABLE_H
#define TTABLE_H

#include "defs.h"
#include "position.h"

const uint32_t probeFailed = UINT_MAX;

// Flags for exact, upperbound and lowerbound scores.
enum { ttExact, ttAlpha, ttBeta };

class ttEntry 
{
	public:
		uint64_t hash;
		uint64_t data;
}; 

class pttEntry 
{
	public:
		uint64_t hash;
		int32_t score;
}; 

// Thanks to the template we have to keep all functions which use t in here, ttable.h.
// Is there any reasonable way to get rid of this behaviour while keeping the template?
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
				size = (uint64_t)pow(2, floor(log2(size)));
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

		void clear()
		{
			table.clear();
			table.resize(tableSize);
		}

		inline uint64_t getSize() { return tableSize; }
		inline t& getEntry(uint64_t entry) { return table[entry]; }
		inline int isEmpty() { return table.empty(); }
	private:
		vector<t> table;
		uint64_t tableSize = 0;
};

extern HashTable<ttEntry> tt;
extern HashTable<pttEntry> ptt;
extern HashTable<ttEntry> perftTT;

void ttSave(Position & pos, uint64_t depth, int64_t score, uint64_t flags, int64_t best);
int ttProbe(Position & pos, int ply, int depth, int & alpha, int & beta, int & best, bool & ttAllowNull);

void pttSave(Position & pos, int32_t score);
int pttProbe(Position & pos);

void perftTTSave(Position & pos, uint64_t nodes, int depth);
uint64_t perftTTProbe(Position & pos, int depth);

#endif
