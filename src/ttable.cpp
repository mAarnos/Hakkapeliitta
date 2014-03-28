#ifndef TTABLE_CPP
#define TTABLE_CPP

#include "ttable.h"
#include "eval.h"

vector<ttEntry> tt;
uint64_t ttSize = 0;

vector<pttEntry> ptt;
uint64_t pttSize = 0;

// This and pttSetSize might be possible to combine into one function. Think about that someday.
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

int ttProbe(Position & pos, uint8_t depth, int & alpha, int & beta, int & best)
{
	if (!ttSize)
	{
		return probeFailed;
	}

	ttEntry * hashEntry = &tt[pos.getHash() % ttSize];
	if (hashEntry->hash == pos.getHash())
	{
		best = hashEntry->bestmove;
		if (hashEntry->depth >= depth)
		{
			int score = hashEntry->score;

			// correct the mate score back.
			if (score >= maxMateScore)
			{
				score -= pos.getTurn();
			}
			else if (score <= -maxMateScore)
			{
				score += pos.getTurn();
			}

			if (hashEntry->flags == ttExact)
			{
				return score;
			}

			if (hashEntry->flags == ttAlpha)
			{
				if (score <= alpha)
				{
					return score;
				}
				if (score < beta)
				{
					beta = score;
					return probeFailed;
				}
			}

			if ((hashEntry->flags == ttBeta) && (score >= beta))
			{
				if (score >= beta)
				{
					return score;
				}
				if (score > alpha)
				{
					alpha = score;
					return probeFailed;
				}
			}
		}
	}

	return probeFailed;
}

int pttProbe(uint64_t pHash)
{
	if (!pttSize)
	{
		return probeFailed;
	}

	pttEntry * hashEntry = &ptt[pHash % pttSize];

	if (hashEntry->hash == pHash)
	{
		return hashEntry->score;
	}

	return probeFailed;
}

void ttSave(Position & pos, uint8_t depth, int16_t score, uint8_t flags, int32_t best)
{
	if (!ttSize)
	{
		return;
	}

	// We only store pure mate scores so that we can use them in other parts of the search tree too without incorrect scores.
	if (isMateScore(score))
	{
		if (score > 0)
		{
			score += (int16_t)pos.getTurn();
		}
		else
		{
			score -= (int16_t)pos.getTurn();
		}
	}

	ttEntry * hashEntry = &tt[pos.getHash() % ttSize];

	hashEntry->hash = pos.getHash();
	hashEntry->bestmove = best;
	hashEntry->score = score;
	hashEntry->flags = flags;
	hashEntry->depth = depth;
}

void pttSave(uint64_t pHash, int32_t score)
{
	if (!pttSize)
	{
		return;
	}

	pttEntry * hashEntry = &ptt[pHash % pttSize];
	hashEntry->hash = pHash;
	hashEntry->score = score;
}

#endif