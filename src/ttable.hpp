#ifndef TTABLE_H_
#define TTABLE_H_

#include "defs.hpp"
#include "position.hpp"

const uint32_t probeFailed = UINT_MAX;
const uint16_t ttMoveNone = USHRT_MAX;

// Flags for exact, upperbound and lowerbound scores.
enum { ttExact = 0, ttAlpha = 1, ttBeta = 2 };

// A single entry in the transposition table. In actuality it is a bucket containing four entries and not a single entry.
// Doing this has some desirable properties when deciding what entries to overwrite.
// We have four entries because the common cache line size nowadays is 64 bytes.
// uint64_t hash[4] + uint64_t data[4] = 8*uint64_t = 64 bytes.
// Basically this means that a single tt entry fits perfectly into the cacheline.
class ttEntry 
{
	public:
		inline uint64_t getHash(int entry) { return hash[entry]; }
		inline uint64_t getData(int entry) { return data[entry]; }
		inline void setHash(int entry, uint64_t newHash) { hash[entry] = newHash; }
		inline void setData(int entry, uint64_t newData) { data[entry] = newData; }
		inline uint16_t getBestMove(int entry) { return (uint16_t)data[entry]; }
		inline uint16_t getGeneration(int entry) { return (uint16_t)(data[entry] >> 16); }
		inline int16_t getScore(int entry) { return (int16_t)(data[entry] >> 32); }
		inline uint8_t getDepth(int entry) { return (uint8_t)(data[entry] >> 48); }
		inline uint8_t getFlags(int entry) { return data[entry] >> 56; }
	private:
		array<uint64_t, 4> hash;
		array<uint64_t, 4> data;
}; 

// A single entry in the pawn hash table. Unlike with the tt, we have no need for buckets here(I think, TODO: check that?).
// Notice that the size of the hash is smaller than with ttEntry. 
// This is simply because pawn hash collisions are very rare even with 32 bits so we don't need to have a long hash.
// Also, here collisions are far less dangerous than when with tt. 
// With the pawn hash table if something happens we just get a false score. With the tt we might get an illegal move which crashes the engine.
class pttEntry 
{
	public:
		inline uint32_t getHash() { return hash; }
		inline uint32_t getData() { return data; }
		inline void setHash(uint32_t newHash) { hash = newHash; }
		inline void setData(uint32_t newData) { data = newData; }
	private:
		uint32_t hash;
		uint32_t data;
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
			generation = 0;
		}

		inline uint64_t getSize() { return tableSize; }
		inline t& getEntry(uint64_t entry) { return table[entry]; }
		inline void startNewSearch() { generation++; }
		inline uint32_t getGeneration() { return generation; }
	private:
		vector<t> table;
		uint64_t tableSize = 0;
		uint32_t generation = 0;
};

extern HashTable<ttEntry> tt;
extern HashTable<pttEntry> ptt;

void ttSave(Position & pos, int ply, uint64_t depth, int64_t score, uint64_t flags, uint16_t best);
int ttProbe(Position & pos, int ply, int depth, int & alpha, int & beta, uint16_t & best, bool & ttAllowNull);

void pttSave(Position & pos, int score);
int pttProbe(Position & pos);

#endif
