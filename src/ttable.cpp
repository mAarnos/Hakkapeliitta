#ifndef TTABLE_CPP
#define TTABLE_CPP

#include "ttable.h"
#include "eval.h"

// Maybe a class HashTable which includes the size?

HashTable<ttEntry> tt;
HashTable<pttEntry> ptt;
HashTable<perftTTEntry> perftTT;

int ttProbe(Position & pos, uint8_t depth, int & alpha, int & beta, int & best)
{
	if (tt.isEmpty())
	{
		return probeFailed;
	}

	ttEntry * hashEntry = &tt.getEntry(pos.getHash() % tt.getSize());

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
	if (ptt.isEmpty())
	{
		return probeFailed;
	}

	pttEntry * hashEntry = &ptt.getEntry(pHash % ptt.getSize());

	if (hashEntry->hash == pHash)
	{
		return hashEntry->score;
	}

	return probeFailed;
}

void ttSave(Position & pos, uint8_t depth, int16_t score, uint8_t flags, int32_t best)
{
	if (tt.isEmpty())
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

	ttEntry * hashEntry = &tt.getEntry(pos.getHash() % tt.getSize());

	hashEntry->hash = pos.getHash();
	hashEntry->bestmove = best;
	hashEntry->score = score;
	hashEntry->flags = flags;
	hashEntry->depth = depth;
}

void pttSave(uint64_t pHash, int32_t score)
{
	if (ptt.isEmpty())
	{
		return;
	}

	pttEntry * hashEntry = &ptt.getEntry(pHash % ptt.getSize());

	hashEntry->hash = pHash;
	hashEntry->score = score;
}

void perftTTSave(Position & pos, uint64_t nodes, int depth)
{
	if (perftTT.isEmpty())
	{
		return;
	}

	perftTTEntry * hashEntry = &perftTT.getEntry(pos.getHash() % perftTT.getSize());

	hashEntry->hash = pos.getHash();
	hashEntry->data = (nodes & 0x00FFFFFFFFFFFFFF) | (uint64_t)depth << 56;
	hashEntry->hash ^= hashEntry->data;
}

uint64_t perftTTProbe(Position & pos, int depth)
{
	if (perftTT.isEmpty())
	{
		return probeFailed;
	}

	perftTTEntry * hashEntry = &perftTT.getEntry(pos.getHash() % perftTT.getSize());

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