#ifndef TTABLE_CPP
#define TTABLE_CPP

#include "ttable.h"
#include "eval.h"

// Maybe a class HashTable which includes the size?

vector<ttEntry> tt;
uint64_t ttSize = 0;

vector<pttEntry> ptt;
uint64_t pttSize = 0;

vector<perftTTEntry> perftTT;
uint64_t perftTTSize = 0;

// The three setSizes could perhaps be combined into one function. Think about that someday.

void ttSetSize(uint64_t size)
{
	tt.clear();

	// If size is not a power of two make it the biggest power of two smaller than size.
	if (size & (size - 1))
	{
		double power = floor(log2(size));
		size = (uint64_t)pow(2, power);
	}

	// If the hash table is too small for even a single entry, do nothing.
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

	if (size & (size - 1))
	{
		double power = floor(log2(size));
		size = (uint64_t)pow(2, power);
	}

	if (size < sizeof(pttEntry)) 
	{
		pttSize = 0;
		return;
	}

	pttSize = (size / sizeof(pttEntry)) - 1;
	ptt.resize(ttSize);
}

void perftTTSetSize(uint64_t size)
{
	perftTT.clear();

	if (size & (size - 1))
	{
		double power = floor(log2(size));
		size = (uint64_t)pow(2, power);
	}

	if (size < sizeof(perftTTEntry))
	{
		perftTTSize = 0;
		return;
	}

	perftTTSize = (size / sizeof(perftTTEntry)) - 1;
	perftTT.resize(perftTTSize);
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
			if (isMateScore(score))
			{
				if (score > 0)
				{
					score -= pos.getTurn();
				}
				else
				{
					score += pos.getTurn();
				}
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

void perftTTSave(Position & pos, uint64_t nodes, int depth)
{
	if (!perftTTSize)
	{
		return;
	}

	perftTTEntry * hashEntry = &perftTT[pos.getHash() % perftTTSize];

	hashEntry->hash = pos.getHash();
	hashEntry->data = (nodes & 0x00FFFFFFFFFFFFFF) | (uint64_t)depth << 56;
	hashEntry->hash ^= hashEntry->data;
}

uint64_t perftTTProbe(Position & pos, int depth)
{
	if (!perftTTSize)
	{
		return probeFailed;
	}

	perftTTEntry * hashEntry = &perftTT[pos.getHash() % perftTTSize];

	if ((hashEntry->hash ^ hashEntry->data) == pos.getHash())
	{
		if ((hashEntry->data >> 56) == depth)
		{
			return (hashEntry->data & 0x00FFFFFFFFFFFFFF);
		}
	}

	return probeFailed;
}

#endif