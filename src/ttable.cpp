#ifndef TTABLE_CPP
#define TTABLE_CPP

#include "ttable.h"
#include "eval.h"

// Maybe a class HashTable which includes the size?

HashTable<ttEntry> tt;
HashTable<pttEntry> ptt;
HashTable<ttEntry> perftTT;

int ttProbe(Position & pos, int ply, uint8_t depth, int & alpha, int & beta, int & best)
{
	if (tt.isEmpty())
	{
		return probeFailed;
	}

	ttEntry * hashEntry = &tt.getEntry(pos.getHash() % tt.getSize());

	if ((hashEntry->hash ^ hashEntry->data) == pos.getHash())
	{
		best = (int)hashEntry->data;
		// (best | score << 32 | depth << 48 | flags << 56)
		if ((hashEntry->data & 0x00FF000000000000) >> 48 >= depth)
		{
			int score = (hashEntry->data & 0x0000FFFF00000000) >> 32;
			int flags = hashEntry->data >> 56;

			// Correct the mate score back.
			// Check that this works, might be ply - 1 or ply + 1.
			if (isMateScore(score))
			{
				if (score > 0)
				{
					score -= ply;
				}
				else
				{
					score += ply;
				}
			}

			if (flags == ttExact)
			{
				return score;
			}

			if (flags == ttAlpha)
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

			if ((flags == ttBeta) && (score >= beta))
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

void ttSave(Position & pos, uint64_t depth, int64_t score, uint64_t flags, int64_t best)
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
			score = mateScore;
		}
		else
		{
			score -= -mateScore;
		}
	}

	ttEntry * hashEntry = &tt.getEntry(pos.getHash() % tt.getSize());

	hashEntry->hash = pos.getHash();
	hashEntry->data = (best | score << 32 | depth << 48 | flags << 56);
	hashEntry->hash ^= hashEntry->data;
}

void pttSave(Position & pos, int32_t score)
{
	if (ptt.isEmpty())
	{
		return;
	}

	pttEntry * hashEntry = &ptt.getEntry(pos.getPawnHash() % ptt.getSize());

	hashEntry->hash = pos.getPawnHash();
	hashEntry->score = score;
}

void perftTTSave(Position & pos, uint64_t nodes, int depth)
{
	if (perftTT.isEmpty())
	{
		return;
	}

	ttEntry * hashEntry = &perftTT.getEntry(pos.getHash() % perftTT.getSize());

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

	ttEntry * hashEntry = &perftTT.getEntry(pos.getHash() % perftTT.getSize());

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