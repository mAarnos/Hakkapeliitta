#ifndef SEARCH_CPP
#define SEARCH_CPP

#include "search.h"
#include "movegen.h"
#include "ttable.h"
#include "eval.h"
#include "uci.h"
#include "movegen.h"

uint64_t nodeCount = 0;

array<int, maxGameLength> killer[2];
array<int, Squares> butterfly[Colours][Squares];

uint64_t perft(Position & pos, int depth)
{
	if (depth == 0)
	{
		return 1;
	}

	Move moveStack[256];
	int generatedMoves = generateMoves(pos, moveStack);
	for (int i = 0; i < generatedMoves; i++)
	{
		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}
		nodeCount += perft(pos, depth - 1);
		pos.unmakeMove(moveStack[i]);
	}

	return 0;
}

uint64_t perftHash(Position & pos, int depth)
{
	uint64_t nodes = 0;
	uint64_t value;

	if (depth == 0)
	{
		return 1;
	}

	if ((value = perftTTProbe(pos, depth)) != probeFailed)
	{
		return value;
	}

	Move moveStack[256];
	int generatedMoves = generateMoves(pos, moveStack);
	for (int i = 0; i < generatedMoves; i++)
	{
		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}
		nodes += perftHash(pos, depth - 1);
		pos.unmakeMove(moveStack[i]);
	}

	perftTTSave(pos, nodes, depth);

	return nodes;
}

// repetitionCount is used to detect twofold repetitions of the current position.
// Also detects fifty move rule draws.
bool Position::repetitionDraw()
{
	if (fiftyMoveDistance >= 100)
	{
		return true;
	}
	// We can skip every position where sideToMove is different from the current one. Also we don't need to go further than hply-fifty because the position must be different.
	for (int i = hply - 2; i >= (hply - fiftyMoveDistance) && i >= 0; i -= 2)  
	{
		if (historyStack[i].hash == hash)
		{
			return true;
		}
	}
	return false;
}

void orderMoves(Position & pos, Move * moveStack, int moveAmount, int &ttMove, int ply)
{
	for (int i = 0; i < moveAmount; i++)
	{
		if (ttMove == moveStack[i].getMove())
		{
			moveStack[i].setScore(hashMove);
		}
		else if (pos.getPiece(moveStack[i].getTo()) != Empty || moveStack[i].getPromotion() < 6)
		{
			int score = pos.SEE(moveStack[i]);
			if (score >= 0)
			{
				score += captureMove;
			}
			moveStack[i].setScore(score);
		}
		else if (moveStack[i].getMove() == killer[0][ply])
		{
			moveStack[i].setScore(killerMove1);
		}
		else if (moveStack[i].getMove() == killer[1][ply])
		{
			moveStack[i].setScore(killerMove2);
		}
		else
		{
			moveStack[i].setScore(butterfly[pos.getSideToMove()][moveStack[i].getFrom()][moveStack[i].getTo()]);
		}
	}
}

void orderCaptures(Position & pos, Move * moveStack, int moveAmount)
{
	for (int i = 0; i < moveAmount; i++)
	{
		moveStack[i].setScore(pos.SEE(moveStack[i]));
	}
}

void selectMove(Move * moveStack, int moveAmount, int i)
{
	int k = i;
	int best = moveStack[i].getScore();

	for (int j = i + 1; j < moveAmount; j++)
	{
		if (moveStack[j].getScore() > best)
		{
			best = moveStack[j].getScore();
			k = j;
		}
	}
	if (k > i)
	{
		swap(moveStack[i], moveStack[k]);
	}
}

int qsearch(Position & pos, int alpha, int beta)
{
	int i, value, bestscore;

	if (searching == false)
	{
		return 0;
	}

	bestscore = value = eval(pos);
	if (value > alpha)
	{
		if (value < beta)
		{
			alpha = value;
		}
		else
		{
			return value;
		}
	}

	Move moveStack[64];
	int generatedMoves = generateCaptures(pos, moveStack);
	for (i = 0; i < generatedMoves; i++)
	{
		selectMove(moveStack, generatedMoves, i);
		// We only try good captures, so if we have reached the bad captures we can stop.
		if (moveStack[i].getScore() < 0)
		{
			break;
		}
		// Delta pruning. Check if the score is below the delta-pruning safety margin.
		// we can return alpha straight away as all captures following a failure are equal or worse to it - that is they will fail as well
		if (value + moveStack[i].getScore() + deltaPruningSafetyMargin < alpha)
		{
			return bestscore;
		}

		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}

		nodeCount++;

		/*
		if (--countdown <= 0)
		{
			readClockAndInput();
		}
		*/
		value = -qsearch(pos, -beta, -alpha);
		pos.unmakeMove(moveStack[i]);

		if (value > bestscore)
		{
			bestscore = value;
			if (value > alpha)
			{
				if (value < beta)
				{
					alpha = value;
				}
				else
				{
					return value;
				}
			}
		}
	}

	return bestscore;
}

#endif