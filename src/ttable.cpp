#ifndef TTABLE_CPP
#define TTABLE_CPP

#include "ttable.h"

vector<ttEntry> tt;
uint64_t ttSize = 0;

vector<pttEntry> ptt;
uint64_t pttSize = 0;

void ttSetSize(uint64_t size)
{
	tt.clear();

	// If size is not a power of two make it the biggest power of two smaller than size.
	if (size & (size - 1))
	{
		size--;
		for (int i = 1; i < 32; i *= 2)
		{
			size |= size >> i;
		}
		size++;
		size >>= 1;
	}

	// If the transposition table is too small for even a single entry, do nothing.
	if (size < sizeof(ttEntry)) 
	{
		ttSize = 0;
		return;
	}

	ttSize = (size / sizeof(ttEntry)) - 1;
	tt.resize(ttSize);
}

void pttSetSize(uint64_t size)
{
	ptt.clear();

	// If size is not a power of two make it the biggest power of two smaller than size.
	if (size & (size - 1))
	{
		size--;
		for (int i = 1; i < 32; i *= 2)
		{
			size |= size >> i;
		}
		size++;
		size >>= 1;
	}

	// If too small a pawn hash table for even a single entry do nothing.
	if (size < sizeof(pttEntry)) 
	{
		pttSize = 0;
		return;
	}

	pttSize = (size / sizeof(pttEntry)) - 1;
	ptt.resize(ttSize);

	return;
}

int ttProbe(uint8_t depth, int * alpha, int * beta, int * best)
{
	if (!ttSize)
	{
		return 0;
	}

	return 0;
}

int pttProbe(uint64_t hash)
{
	if (!pttSize)
	{
		return 0;
	}

	return 0;
}

void ttSave(uint8_t depth, int16_t score, uint8_t flags, int32_t best)
{
	if (!ttSize)
	{
		return;
	}
}

void pttSave(uint64_t hash, int32_t score)
{
	if (!pttSize)
	{
		return;
	}
}

#endif