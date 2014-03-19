#ifndef TTABLE_CPP
#define TTABLE_CPP

#include "ttable.h"
#include "board.h"
#include "eval.h"

ttEntry * tt;
int ttSize = 0;

pttEntry * ptt;
int pttSize = 0;

int ttSetSize(int size)
{
	free(tt); // necessary so no undefined behaviour occurs

	// size is not a power of two make it the biggest power of two smaller than size
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

	// if too small a transposition table for even a single entry, do nothing
	if (size < sizeof(ttEntry)) 
	{
		ttSize = 0;
		return 0;
	}

	ttSize = (size / sizeof(ttEntry)) - 1;
	tt = (ttEntry *) malloc(size);
	memset(tt, 0, size);

	return 0;
}

int pttSetSize(int size)
{
	free(ptt); // necessary so no undefined behaviour occurs

	// size is not a power of two make it the biggest power of two smaller than size
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

	// if too small a pawn hash table for even a single entry, do nothing
	if (size < sizeof(pttEntry)) 
	{
		pttSize = 0;
		return 0;
	}

	pttSize = (size / sizeof(pttEntry)) - 1;
	ptt = (pttEntry *) malloc(size);
	memset(ptt, 0, size);

	return 0;
}

int ttProbe(unsigned __int8 depth, int * alpha, int * beta, int * best, bool * ttAllowNull) 
{
	ttEntry * hashEntry = &tt[Hash % ttSize];
 
	if (hashEntry->Hash == Hash) 
	{
		*best = hashEntry->bestmove;
		if (hashEntry->flags == ttAlpha && hashEntry->depth >= depth - 1 - (depth > 6 ? 3 : 2) && hashEntry->score < *beta) 
		{
			*ttAllowNull = false;
		}
		if (hashEntry->depth >= depth) 
		{
			int score = hashEntry->score;

			// correct the mate score back to usable form
			if (score >= mateScore - 600)
			{
				score -= ply;
			}
			else if (score <= -mateScore + 600)
			{
				score += ply;
			}

			// handles every possible case, no need to optimize
			if (hashEntry->flags == ttExact)
				return score;

			if ((hashEntry->flags == ttAlpha) && (score <= *alpha))
				return score;
			
			if ((hashEntry->flags == ttAlpha) && (score < *beta))
			{
				*beta = score; 
				return Invalid;
			}
			
			if ((hashEntry->flags == ttBeta) && (score >= *beta))
				return score;
			
			if ((hashEntry->flags == ttBeta) && (score > *alpha))
			{
				*alpha = score;
				return Invalid;
			}
			
		}
	}
	return Invalid;
}

int pttProbe(U64 hash)
{
	pttEntry * hashEntry = &ptt[hash % pttSize];

	if (hashEntry->Hash == hash) 
	{
		return hashEntry->score;
	}

	return Invalid;
}

void ttSave(unsigned __int8 depth, __int16 score, unsigned __int8 flags, int best) 
{
	ttEntry * hashEntry = &tt[Hash % ttSize];

	if (score >= mateScore - 600)
	{
		score += (short)ply;
	}
	else if (score <= -mateScore + 600)
	{
		score -= (short)ply;
	}

	// always replace replacement scheme
	hashEntry->Hash = Hash;
	hashEntry->bestmove = best;
	hashEntry->score = score;
	hashEntry->flags = flags;
	hashEntry->depth = depth;
}

void pttSave(U64 hash, int score)
{
	pttEntry * hashEntry = &ptt[hash % pttSize];

	// always replace replacement scheme
	hashEntry->Hash = hash;
	hashEntry->score = score;
}

#endif