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

int alphabetaPVS(Position & pos, int ply, int depth, int alpha, int beta, bool allowNullMove)
{
	bool movesFound, pvFound;
	bool check;
	int value, generatedMoves;
	int ttMove = probeFailed;
	int bestmove = -1;
	int ttFlag = ttAlpha;
	bool ttAllowNull = true;
	int bestscore = -mateScore;

	// Check extension.
	// Makes sure that the quiescence search is NEVER started while being in check.
	check = pos.inCheck(pos.getSideToMove());
	if (check)
	{
		depth += onePly;
	}

	if (depth <= 0)
	{
		return qsearch(pos, alpha, beta);
	}

	// Check for repetition and fifty move draws.
	if (pos.repetitionDraw())
	{
		return drawScore;
	}

	// Probe the transposition table.
	if ((value = ttProbe(pos, ply, depth, alpha, beta, ttMove, ttAllowNull)) != probeFailed)
	{
		return value;
	}

	// Null move pruning, both static and dynamic.
	if (allowNullMove && ttAllowNull)
	{
		if (!check)
		{
			if (pos.calculateGamePhase() != 256)
			{
				// Here's static.
				if (depth <= 3 * onePly)
				{
					int staticEval = eval(pos);
					if (depth == 1 * onePly && staticEval - 260 >= beta)
					{
						return staticEval;
					}
					else if (depth == 2 * onePly && staticEval - 445 >= beta)
					{
						return staticEval;
					}
					else if (depth == 3 * onePly && staticEval - 900 >= beta)
					{
						depth -= onePly;
					}
				}
				nodeCount++;
				/*
				if (--countdown <= 0)
				{
					readClockAndInput();
				}
				
				sideToMove = !sideToMove;
				Hash ^= side;
				*/
				// And here's dynamic.
				if (depth <= 3 * onePly)
				{
					value = -qsearch(pos, -beta, -beta + 1);
				}
				else
				{
					value = -alphabetaPVS(pos, ply, (depth - 1 * onePly - (depth > (6 * onePly) ? 3 * onePly : 2 * onePly)), -beta, -beta + 1, false);
				}
				/*
				sideToMove = !sideToMove;
				Hash ^= side;
				*/
				if (searching == false)
				{
					return 0;
				}

				if (value >= beta)
				{
					return value;
				}
			}
		}
	}

	allowNullMove = true;

	// Internal iterative deepening.
	if ((alpha + 1) != beta && ttMove == probeFailed && depth > 2 * onePly && searching)
	{
		value = alphabetaPVS(pos, ply, depth - 2 * onePly, alpha, beta, allowNullMove);
		if (value <= alpha)
		{
			value = alphabetaPVS(pos, ply, depth - 2 * onePly, -infinity, beta, allowNullMove);
		}
		ttProbe(pos, ply, depth, alpha, beta, ttMove, ttAllowNull);
	}

	pvFound = false;
	movesFound = false;

	Move moveStack[256];
	/*
	if (check)
	{
		generatedMoves = generateEvasions(pos, moveStack);
		if (generatedMoves == 1)
		{
			depth += onePly;
		}
	}
	else
	{
		generatedMoves = generateMoves(pos, moveStack);
	}
	*/
	generatedMoves = generateMoves(pos, moveStack);
	orderMoves(pos, moveStack, generatedMoves, ttMove, ply);
	for (int i = 0; i < generatedMoves; i++)
	{
		selectMove(moveStack, generatedMoves, i);
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
		movesFound = true;
		if (pvFound)
		{
			value = -alphabetaPVS(pos, ply + 1, depth - onePly, -alpha - 1, -alpha, allowNullMove); 
			if (value > alpha && value < beta)
			{
				value = -alphabetaPVS(pos, ply + 1, depth - onePly, -beta, -alpha, allowNullMove);
			}
		}
		else
		{
			value = -alphabetaPVS(pos, ply + 1, depth - onePly, -beta, -alpha, allowNullMove); 
		}
		pos.unmakeMove(moveStack[i]);

		if (searching == false)
		{
			return 0;
		}

		if (value > bestscore)
		{
			bestscore = value;
			if (value > alpha)
			{
				// Update the history heuristic when a move which improves alpha is found.
				// Don't update if the move is not a quiet move.
				if ((pos.getPiece(moveStack[i].getTo()) == Empty) && !(moveStack[i].getPromotion() < 5 && moveStack[i].getPromotion() > 0))
				{
					butterfly[pos.getSideToMove()][moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
					if (value >= beta)
					{
						if (moveStack[i].getMove() != killer[0][ply])
						{
							killer[1][ply] = killer[0][ply];
							killer[0][ply] = moveStack[i].getMove();
						}
					}
				}

				if (value < beta)
				{
					alpha = value;
					ttFlag = ttExact;
					bestmove = moveStack[i].getMove();
					pvFound = true;
				}
				else
				{
					ttSave(pos, depth, value, ttBeta, moveStack[i].getMove());
					return value;
				}
			}
		}
	}

	if (!movesFound)
	{
		if (check)
		{
			bestscore = -mateScore + ply;
		}
		else
		{
			bestscore = drawScore;
		}
	}

	ttSave(pos, depth, bestscore, ttFlag, bestmove);

	return bestscore;
}

#endif