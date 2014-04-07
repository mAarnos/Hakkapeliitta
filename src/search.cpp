#ifndef SEARCH_CPP
#define SEARCH_CPP

#include "search.h"
#include "movegen.h"
#include "ttable.h"
#include "eval.h"
#include "uci.h"
#include "movegen.h"
#include "time.h"

uint64_t nodeCount = 0;

array<int, maxGameLength> killer[2];
array<int, Squares> butterfly[Colours][Squares];

int searchRoot(Position & pos, int ply, int depth, int alpha, int beta);

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

void orderMoves(Position & pos, Move * moveStack, int moveAmount, int ttMove, int ply)
{
	for (int i = 0; i < moveAmount; i++)
	{
		if (ttMove == moveStack[i].getMove())
		{
			moveStack[i].setScore(hashMove);
		}
		else if (pos.getPiece(moveStack[i].getTo()) != Empty || moveStack[i].getPromotion() != Empty || moveStack[i].getEnPassant())
		{
			int score = pos.SEE(moveStack[i]);
			// If the capture is good, order it way higher than bad captures.
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
		else if (ply > 1 && moveStack[i].getMove() == killer[0][ply - 2])
		{
			moveStack[i].setScore(killerMove3);
		}
		else if (ply > 1 && moveStack[i].getMove() == killer[1][ply - 2])
		{
			moveStack[i].setScore(killerMove4);
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

void reconstructPV(Position pos, vector<Move> & pv)
{
	ttEntry * hashEntry;
	Move m;
	pv.clear();

	int ply = 0;
	while (ply < 63)
	{
		hashEntry = &tt.getEntry(pos.getHash() % tt.getSize());
		if ((hashEntry->hash ^ hashEntry->data) != pos.getHash()
        || ((hashEntry->data >> 56) != ttExact)
		|| ((int)hashEntry->data) == ttMoveNone)
		{
			break;
		}
		m.setMove((int)hashEntry->data);
		pv.push_back(m);
		pos.makeMove(m);
		ply++;
	}
}

// Displays the pv in the format UCI-protocol wants(from, to, if promotion add what promotion).
void displayPV(vector<Move> pv)
{
	static array<string, Squares> squareToNotation = {
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	};

	static array<string, Pieces> promotionToNotation = {
		"P", "N", "B", "R", "Q", "K"
	};

	for (int i = 0; i < pv.size(); i++)
	{
		int from = pv[i].getFrom();
		int to = pv[i].getTo();

		cout << squareToNotation[from] << squareToNotation[to];

		if (pv[i].getPromotion() != Empty)
		{
			int promotion = pv[i].getPromotion();
			cout << promotionToNotation[promotion];
		}
		cout << " ";
	}
	cout << endl;
}

void displayBestMove(Move m)
{
	static array<string, Squares> squareToNotation = {
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	};

	static array<string, Pieces> promotionToNotation = {
		"P", "N", "B", "R", "Q", "K"
	};

	int from = m.getFrom();
	int to = m.getTo();

	cout << squareToNotation[from] << squareToNotation[to];
	if (m.getPromotion() != Empty)
	{
		int promotion = m.getPromotion();
		cout << promotionToNotation[promotion];
	}
	cout << " ";
}

void think()
{
	int score, alpha, beta;
	int searchDepth, searchTime;
	vector<Move> pv;

	searching = true;
	memset(butterfly, 0, sizeof(butterfly));
	memset(killer, 0, sizeof(killer));
	nodeCount = 0;
	countDown = stopInterval;

	alpha = -infinity;
	beta = infinity;

	t.reset();
	t.start();

	for (searchDepth = 1;;)
	{
		score = searchRoot(root, 0, searchDepth * onePly, alpha, beta);
		reconstructPV(root, pv);

		searchTime = (int)t.getms();

		// If more than 70% of our has been used or we have been ordered to stop searching return the best move.
		// Also stop searching if there is only one root move or if we have searched too far.
		if (searching == false || t.getms() > (stopFraction * targetTime) || searchDepth >= 64)
		{
			cout << "info " << "time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << endl;
			cout << "bestmove ";
			displayBestMove(pv[0]);
			cout << endl;

			return;
		}

		// if our score is outside the aspiration window do a research with no windows
		if (score <= alpha)
		{
			alpha = -infinity;
			if (isMateScore(score))
			{
				int v;
				score > 0 ? v = ((mateScore - score + 1) >> 1) : v = ((-score - mateScore) >> 1);
				cout << "info " << "depth " << searchDepth << " score mate " << v << " upperbound " << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " pv ";
				displayPV(pv);
			}
			else
			{
				cout << "info " << "depth " << searchDepth << " score cp " << score << " upperbound " << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " pv ";
				displayPV(pv);
			}
			continue;
		}
		if (score >= beta)
		{
			beta = infinity;
			if (isMateScore(score))
			{
				int v;
				score > 0 ? v = ((mateScore - score + 1) >> 1) : v = ((-score - mateScore) >> 1);
				cout << "info " << "depth " << searchDepth << " score mate " << v << " lowerbound " << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " pv ";
				displayPV(pv);
			}
			else
			{
				cout << "info " << "depth " << searchDepth << " score cp " << score << " lowerbound " << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " pv ";
				displayPV(pv);
			}
			continue;
		}

		if (isMateScore(score))
		{
			int v;
			score > 0 ? v = ((mateScore - score + 1) >> 1) : v = ((-score - mateScore) >> 1);
			cout << "info " << "depth " << searchDepth << " score mate " << v << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " pv ";
			displayPV(pv);
		}
		else
		{
			cout << "info " << "depth " << searchDepth << " score cp " << score << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " pv ";
			displayPV(pv);
		}

		// Adjust alpha and beta based on the last score.
		// Don't adjust if depth is low - it's a waste of time.
		if (searchDepth >= 4)
		{
			alpha = score - aspirationWindow;
			beta = score + aspirationWindow;
		}

		searchDepth++;
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
		if (value > beta)
		{
			return value;
		}
		alpha = value;
	}

	Move moveStack[64];
	int generatedMoves = generateCaptures(pos, moveStack);
	orderCaptures(pos, moveStack, generatedMoves);
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

		if (countDown-- <= 0)
		{
			readClockAndInput();
		}
		
		value = -qsearch(pos, -beta, -alpha);
		pos.unmakeMove(moveStack[i]);

		if (value > bestscore)
		{
			bestscore = value;
			if (value > alpha)
			{
				if (value > beta)
				{
					return value;
				}
				alpha = value;
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
	int ttMove = ttMoveNone;
	int bestmove = ttMoveNone;
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

				if (countDown-- <= 0)
				{
					readClockAndInput();
				}
				
				// And here's dynamic.
				pos.makeNullMove();
				if (depth <= 3 * onePly)
				{
					value = -qsearch(pos, -beta, -beta + 1);
				}
				else
				{
					value = -alphabetaPVS(pos, ply, (depth - 1 * onePly - (depth > (6 * onePly) ? 3 * onePly : 2 * onePly)), -beta, -beta + 1, false);
				}
				pos.unmakeNullMove();

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
	
	// Internal iterative deepening
	if ((alpha + 1) != beta && ttMove == ttMoveNone && depth > 2 * onePly && searching)
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

		if (countDown-- <= 0)
		{
			readClockAndInput();
		}

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
			bestmove = moveStack[i].getMove();
			if (value > alpha)
			{
				// Update the history heuristic when a move which improves alpha is found.
				// Don't update if the move is not a quiet move.
				if ((pos.getPiece(moveStack[i].getTo()) == Empty) && (moveStack[i].getPromotion() == Empty))
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
					pvFound = true;
				}
				else
				{
					ttSave(pos, depth, value, ttBeta, bestmove);
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

int searchRoot(Position & pos, int ply, int depth, int alpha, int beta)
{
	int value, generatedMoves;
	bool check;
	bool pvFound = false;
	int ttMove = ttMoveNone;
	int bestmove = ttMoveNone;
	int bestscore = -mateScore;

	check = pos.inCheck(pos.getSideToMove());
	if (check)
	{
		depth += onePly;
	}

	Move moveStack[256];
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

		if (countDown-- <= 0)
		{
			readClockAndInput();
		}
		if (pvFound)
		{
			value = -alphabetaPVS(pos, ply + 1, depth - onePly, -alpha - 1, -alpha, true);
			if (value > alpha && value < beta)
			{
				value = -alphabetaPVS(pos, ply + 1, depth - onePly, -beta, -alpha, true);
			}
		}
		else
		{
			value = -alphabetaPVS(pos, ply + 1, depth - onePly, -beta, -alpha, true);
		}

		pos.unmakeMove(moveStack[i]);

		if (searching == false)
		{
			return 0;
		}

		if (value > bestscore)
		{
			bestscore = value;
			bestmove = moveStack[i].getMove();
			if (value > alpha)
			{
				// Update the history heuristic when a move which improves alpha is found.
				// Don't update if the move is not a quiet move.
				if ((pos.getPiece(moveStack[i].getTo()) == Empty) && (moveStack[i].getPromotion() == Empty))
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
					ttSave(pos, depth, value, ttAlpha, bestmove);
					pvFound = true;
				}
				else
				{
					ttSave(pos, depth, value, ttBeta, bestmove);
					return value;
				}
			}
		}
	}

	ttSave(pos, depth, bestscore, ttExact, bestmove);

	return bestscore;
}

#endif