#include "ttable.hpp"
#include "eval.hpp"

HashTable<ttEntry> tt;
HashTable<pttEntry> ptt;

int ttProbe(const Position & pos, int ply, int depth, int & alpha, int & beta, uint16_t & best)
{
	ttEntry * hashEntry = &tt.getEntry(pos.getHash() % tt.getSize());

	int entry;
	for (entry = 0; entry < 4; entry++)
	{
		if ((hashEntry->getHash(entry) ^ hashEntry->getData(entry)) == pos.getHash())
		{
			break;
		}
	}

	if (entry < 4)
	{
		best = hashEntry->getBestMove(entry);
		int score = hashEntry->getScore(entry);
		int hashDepth = hashEntry->getDepth(entry);
		int flags = hashEntry->getFlags(entry);

		if (hashDepth >= depth)
		{
			// Correct the mate score back.
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

			if (flags == ttBeta)
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

void ttSave(const Position & pos, int ply, uint64_t depth, int64_t score, uint64_t flags, uint16_t best)
{
	assert(depth >= 0 && depth <= 255);
	assert(score < infinity && score > -infinity);
	assert(flags >= 0 && flags <= 255);
	assert(best >= 0 && best <= 65535);

	// We only store pure mate scores so that we can use them in other parts of the search tree too without incorrect scores.
	if (isMateScore(static_cast<int>(score)))
	{
		if (score > 0)
		{
			score += ply;
		}
		else
		{
			score -= ply;
		}
	}
	
	ttEntry * hashEntry = &tt.getEntry(pos.getHash() % tt.getSize());
	int replace = 0;
	for (int i = 0; i < 4; i++)
	{
		if ((hashEntry->getHash(i) ^ hashEntry->getData(i)) == pos.getHash())
		{
			replace = i;
			if (best == ttMoveNone)
			{
				best = hashEntry->getBestMove(replace);
			}
			break;
		}
		// Here we check if we have found an entry which is worse than the current worse entry.
		// If the entry is from a earlier search or has a smaller depth it is worse and is made the new worst entry.
		bool c1 = (hashEntry->getGeneration(replace) > hashEntry->getGeneration(i));
		bool c2 = (hashEntry->getDepth(replace) > hashEntry->getDepth(i));
		if (c1 || c2)
		{
			replace = i;
		}
	}
	hashEntry->setData(replace, (best | tt.getGeneration() << 16 | (score & 0xffff) << 32 | ((depth & 0xff) << 48) | flags << 56));
	hashEntry->setHash(replace, pos.getHash() ^ hashEntry->getData(replace));

	assert(hashEntry->getBestMove(replace) == best);
	assert(hashEntry->getGeneration(replace) == tt.getGeneration());
	assert((hashEntry->getScore(replace)) == score);
	assert(hashEntry->getDepth(replace) == depth);
	assert(hashEntry->getFlags(replace) == flags);
}

void pttSave(const Position & pos, int score)
{
	assert(score < infinity && score > -infinity);

	pttEntry * hashEntry = &ptt.getEntry(pos.getPawnHash() % ptt.getSize());

	hashEntry->setData(score);
	hashEntry->setHash((uint32_t)pos.getPawnHash() ^ hashEntry->getData());

	assert((int)hashEntry->getData() == score);
}

int pttProbe(const Position & pos)
{
	pttEntry * hashEntry = &ptt.getEntry(pos.getPawnHash() % ptt.getSize());

	if ((hashEntry->getHash() ^ hashEntry->getData()) == (uint32_t)pos.getPawnHash())
	{
		return hashEntry->getData();
	}

	return probeFailed;
}