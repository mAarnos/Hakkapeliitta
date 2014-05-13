#include "search.hpp"
#include "movegen.hpp"
#include "ttable.hpp"
#include "eval.hpp"
#include "uci.hpp"
#include "movegen.hpp"
#include "time.hpp"
#include "tbprobe.hpp"

uint64_t nodeCount = 0;
uint64_t tbHits = 0;
int searchDepth;
int syzygyProbeLimit = 0;
bool probeTB;

array<int, Squares> butterfly[Colours][Squares];
vector<Move> pv;

int searchRoot(Position & pos, int ply, int depth, int alpha, int beta);

uint64_t perft(Position & pos, int depth)
{
	uint64_t generatedMoves, nodes = 0;

	if (depth == 0)
	{
		return 1;
	}

	Move moveStack[256];
	if (pos.inCheck())
	{
		generatedMoves = generateEvasions(pos, moveStack);
	}
	else
	{
		generatedMoves = generateMoves(pos, moveStack);
	}
	for (int i = 0; i < generatedMoves; i++)
	{
		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}
		nodes += perft(pos, depth - 1);
		pos.unmakeMove(moveStack[i]);
	}

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
		else if (pos.getPiece(moveStack[i].getTo()) != Empty || ((moveStack[i].getPromotion() != Empty) && (moveStack[i].getPromotion() != King)))
		{
			int score = pos.SEE(moveStack[i]);
			// If the capture is good, order it way higher than anything else with the exception of the hash move.
			if (score >= 0)
			{
				score += captureMove;
			}
			moveStack[i].setScore(score);
		}
		else if (moveStack[i].getMove() == pos.getKiller(0, ply))
		{
			moveStack[i].setScore(killerMove1);
		}
		else if (moveStack[i].getMove() == pos.getKiller(1, ply))
		{
			moveStack[i].setScore(killerMove2);
		}
		else if (ply > 1 && moveStack[i].getMove() == pos.getKiller(0, ply - 2))
		{
			moveStack[i].setScore(killerMove3);
		}
		else if (ply > 1 && moveStack[i].getMove() == pos.getKiller(1, ply - 2))
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

// Clean this function up.
void reconstructPV(Position pos, vector<Move> & pv)
{
	Move m;
	pv.clear();

	int ply = 0;
	int entry;
	while (ply < 63)
	{
		ttEntry * hashEntry = &tt.getEntry(pos.getHash() % tt.getSize());
		for (entry = 0; entry < 4; entry++)
		{
			if ((hashEntry->getHash(entry) ^ hashEntry->getData(entry)) == pos.getHash())
			{
				break;
			}
		}

		if ((entry > 3)
		|| (hashEntry->getFlags(entry) != ttExact && ply >= 2)
		|| (pos.repetitionDraw() && ply >= 2)
		|| (hashEntry->getBestMove(entry) == ttMoveNone))
		{
			break;
		}
		m.setMove(hashEntry->getBestMove(entry));
		pv.push_back(m);
		pos.makeMove(m);
		ply++;
	}
}

// Displays the pv in the format UCI-protocol wants(from, to, if promotion add what promotion).
void displayPV(vector<Move> pv, int length)
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
		"p", "n", "b", "r", "q", "k"
	};

	for (int i = 0; i < length; i++)
	{
		int from = pv[i].getFrom();
		int to = pv[i].getTo();
		int promotion = pv[i].getPromotion();

		cout << squareToNotation[from] << squareToNotation[to];

		if (promotion != Empty && promotion != King && promotion != Pawn)
		{
			cout << promotionToNotation[promotion];
		}
		cout << " ";
	}
	cout << endl;
}

void think()
{
	int alpha, beta;

	tt.startNewSearch();
	
	searching = true;
	memset(butterfly, 0, sizeof(butterfly));
	nodeCount = 0;
	tbHits = 0;
	countDown = stopInterval;

	alpha = -infinity;
	beta = infinity;

	t.reset();
	t.start();

	for (searchDepth = 1;;)
	{
		int score = searchRoot(root, 0, searchDepth * onePly, alpha, beta);
		reconstructPV(root, pv);

		uint64_t searchTime = t.getms();

		// If more than 70% of our has been used or we have been ordered to stop searching return the best move.
		// Also stop searching if there is only one root move or if we have searched too far.
		if (searching == false || t.getms() > (stopFraction * targetTime) || searchDepth >= 64)
		{
			cout << "info " << "time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << endl << "bestmove ";
			displayPV(pv, 1);
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
				cout << "info " << "depth " << searchDepth << " score mate " << v << " upperbound " << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << " pv ";
				displayPV(pv, (int)pv.size());
			}
			else
			{
				cout << "info " << "depth " << searchDepth << " score cp " << score << " upperbound " << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << " pv ";
				displayPV(pv, (int)pv.size());
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
				cout << "info " << "depth " << searchDepth << " score mate " << v << " lowerbound " << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << " pv ";
				displayPV(pv, (int)pv.size());
			}
			else
			{
				cout << "info " << "depth " << searchDepth << " score cp " << score << " lowerbound " << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << " pv ";
				displayPV(pv, (int)pv.size());
			}
			continue;
		}

		if (isMateScore(score))
		{
			int v;
			score > 0 ? v = ((mateScore - score + 1) >> 1) : v = ((-score - mateScore) >> 1);
			cout << "info " << "depth " << searchDepth << " score mate " << v << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << " pv ";
			displayPV(pv, (int)pv.size());
		}
		else
		{
			cout << "info " << "depth " << searchDepth << " score cp " << score << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << " pv ";
			displayPV(pv, (int)pv.size());
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
	int value, bestScore, delta;

	value = eval(pos);
	if (value > alpha)
	{
		if (value >= beta)
		{
			return value;
		}
		alpha = value;
	}
	else if (value + 1000 < alpha) // If we are doing extremely badly, so badly that even capturing a hanging queen can't help us just return alpha.
	{
		return alpha;
	}
	bestScore = value;
	delta = value + futilityMargin[0];

	Move moveStack[64];
	int generatedMoves = generateCaptures(pos, moveStack);
	orderCaptures(pos, moveStack, generatedMoves);
	for (int i = 0; i < generatedMoves; i++)
	{
		selectMove(moveStack, generatedMoves, i);
		// We only try good captures, so if we have reached the bad captures we can stop.
		if (moveStack[i].getScore() < 0)
		{
			break;
		}
		// Delta pruning. Check if the score is below the delta-pruning safety margin.
		// we can return alpha straight away as all captures following a failure are equal or worse to it - that is they will fail as well
		if (delta + moveStack[i].getScore() < alpha)
		{
			return bestScore;
		}

		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}

		nodeCount++;
		value = -qsearch(pos, -beta, -alpha);
		pos.unmakeMove(moveStack[i]);

		if (value > bestScore)
		{
			bestScore = value;
			if (value > alpha)
			{
				if (value >= beta)
				{
					return value;
				}
				alpha = value;
			}
		}
	}

	return bestScore;
}

int alphabetaPVS(Position & pos, int ply, int depth, int alpha, int beta, bool allowNullMove)
{
	bool pvNode = ((alpha + 1) != beta), futileNode = false, check;
	int value, generatedMoves, ttFlag = ttAlpha, bestScore = -mateScore, movesSearched = 0;
	uint16_t ttMove = ttMoveNone, bestMove = ttMoveNone;

	// Check if we have overstepped the time limit or if the user has given a new order.
	if (countDown-- <= 0)
	{
		checkTimeAndInput();
	}

	// Check for aborted search(either due to going beyond the allocated time or a new command) and repetition+fifty move draws.
	if (!searching || pos.repetitionDraw())
	{
		return drawScore;
	}

	check = pos.getIsInCheck(ply);

	// If we have gone as far as we wanted to go drop into quiescence search.
	if (depth <= 0)
	{
		return qsearch(pos, alpha, beta);
	}

	// Probe the transposition table.
	if ((value = ttProbe(pos, ply, depth, alpha, beta, ttMove)) != probeFailed)
	{
		return value;
	}

	// Probe the Syzygy tablebases.
	if (probeTB && popcnt(pos.getOccupiedSquares()) <= syzygyProbeLimit)
	{
		int found, v = probe_wdl(pos, &found);
		if (found)
		{
			tbHits++;
			if (v < -1) value = -mateScore + ply + 200;
			else if (v > 1) value = mateScore - ply - 200;
			else value = drawScore + v;
			ttSave(pos, ply, depth + 4 * onePly, value, ttExact, ttMoveNone);
			return value;
		}
	}

	// Get the static evaluation of the position.
	int staticEval = eval(pos);

	// Null move pruning, both static and dynamic.
	if (!pvNode && allowNullMove && !check && pos.calculateGamePhase() != 256)
	{
		// Here's static.
		if (depth <= 3 * onePly)
		{
			if (depth <= onePly && staticEval - 260 >= beta)
			{
				return staticEval;
			}
			else if (depth <= 2 * onePly && staticEval - 445 >= beta)
			{
				return staticEval;
			}
			else if (staticEval - 900 >= beta)
			{
				depth -= onePly;
			}
		}

		// And here's dynamic.
		pos.makeNullMove();
		nodeCount++;
		if (depth <= 3 * onePly)
		{
			value = -qsearch(pos, -beta, -beta + 1);
		}
		else
		{
			value = -alphabetaPVS(pos, ply, (depth - onePly - (depth > (6 * onePly) ? 3 * onePly : 2 * onePly)), -beta, -beta + 1, false);
		}
		pos.unmakeNullMove();

		if (!searching)
		{
			return drawScore;
		}

		if (value >= beta)
		{
			return value;
		}
	}

	// Internal iterative deepening
	if (pvNode && ttMove == ttMoveNone && depth > 2 * onePly)
	{
		value = alphabetaPVS(pos, ply, depth - 2 * onePly, alpha, beta, true);
		if (value <= alpha)
		{
			value = alphabetaPVS(pos, ply, depth - 2 * onePly, -infinity, beta, true);
		}
		ttProbe(pos, ply, depth, alpha, beta, ttMove);
	}

	if (!pvNode && !check && depth <= futilityDepth && staticEval + futilityMargin[depth] <= alpha)
	{
		futileNode = true;
	}
	
	Move moveStack[256];
	if (check)
	{
		generatedMoves = generateEvasions(pos, moveStack);
		/*
		if (generatedMoves == 1)
		{
			depth += onePly;
		}
		*/
	}
	else
	{
		generatedMoves = generateMoves(pos, moveStack);
	}
	orderMoves(pos, moveStack, generatedMoves, ttMove, ply);
	for (int i = 0; i < generatedMoves; i++)
	{
		selectMove(moveStack, generatedMoves, i);
		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}
		nodeCount++;

		int newDepth = depth - onePly;
		bool givesCheck = pos.inCheck();
		pos.setIsInCheck(ply + 1, givesCheck);
		if (givesCheck)
		{
			newDepth += onePly;
		}

		if (futileNode && moveStack[i].getScore() < captureMove && !givesCheck && movesSearched)
		{
			pos.unmakeMove(moveStack[i]);
			continue;
		}

		if (!movesSearched)
		{
			value = -alphabetaPVS(pos, ply + 1, newDepth, -beta, -alpha, true);
		}
		else
		{
			value = -alphabetaPVS(pos, ply + 1, newDepth, -alpha - 1, -alpha, true);
			if (value > alpha && value < beta)
			{
				value = -alphabetaPVS(pos, ply + 1, newDepth, -beta, -alpha, true);
			}
		}
		pos.unmakeMove(moveStack[i]);
		movesSearched++;

		if (!searching)
		{
			return drawScore;
		}

		if (value > bestScore)
		{
			bestScore = value;
			bestMove = moveStack[i].getMove();
			if (value > alpha)
			{
				// Update the history heuristic when a move which improves alpha is found.
				// Don't update if the move is not a quiet move.
				if ((pos.getPiece(moveStack[i].getTo()) == Empty) && ((moveStack[i].getPromotion() == Empty) || (moveStack[i].getPromotion() == King)))
				{
					butterfly[pos.getSideToMove()][moveStack[i].getFrom()][moveStack[i].getTo()] += depth * depth;
					if (value >= beta)
					{
						if (moveStack[i].getMove() != pos.getKiller(0, ply))
						{
							pos.setKiller(1, ply, pos.getKiller(0, ply));
							pos.setKiller(0, ply, moveStack[i].getMove());
						}
					}
				}

				if (value >= beta)
				{
					ttSave(pos, ply, depth, value, ttBeta, bestMove);
					return value;
				}
				alpha = value;
				ttFlag = ttExact;
			}
		}
	}

	if (!movesSearched)
	{
		if (check)
		{
			return -mateScore + ply;
		}
		else
		{
			return drawScore;
		}
	}

	ttSave(pos, ply, depth, bestScore, ttFlag, bestMove);

	return bestScore;
}

int searchRoot(Position & pos, int ply, int depth, int alpha, int beta)
{
	int value, generatedMoves, newDepth, bestScore = -mateScore, movesSearched = 0;
	uint16_t ttMove = ttMoveNone, bestMove = ttMoveNone;

	// Probe the transposition table to get the PV-Move, if any.
	ttProbe(pos, ply, depth, alpha, beta, ttMove);

	Move moveStack[256];
	generatedMoves = generateMoves(pos, moveStack);

	// Check if the root position is in tablebases, and if it is remove moves which do not maintain the correct result.
	probeTB = true;
	if (popcnt(pos.getOccupiedSquares()) <= syzygyProbeLimit)
	{
		bool rootInTB = root_probe(pos, value, moveStack, generatedMoves);
		if (rootInTB)
		{
			tbHits += generatedMoves;
			probeTB = false; // Do not probe tablebases during the search
		}
		else // If DTZ tables are missing, use WDL tables as a fallback
		{
			// Filter out moves that do not preserve a draw or win
			rootInTB = root_probe_wdl(pos, value, moveStack, generatedMoves);
			if (rootInTB)
			{
				tbHits += generatedMoves;
				probeTB = false; // Do not probe tablebases during the search
			}
		}
	}

	orderMoves(pos, moveStack, generatedMoves, ttMove, ply);
	for (int i = 0; i < generatedMoves; i++)
	{
		selectMove(moveStack, generatedMoves, i);
		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}
		nodeCount++;

		newDepth = depth - onePly;
		pos.setIsInCheck(ply + 1, pos.inCheck());
		if (pos.getIsInCheck(ply + 1))
		{
			newDepth += onePly;
		}

		if (!movesSearched)
		{
			value = -alphabetaPVS(pos, ply + 1, newDepth, -beta, -alpha, true);
		}
		else
		{
			value = -alphabetaPVS(pos, ply + 1, newDepth, -alpha - 1, -alpha, true);
			if (value > alpha && value < beta)
			{
				value = -alphabetaPVS(pos, ply + 1, newDepth, -beta, -alpha, true);
			}
		}
		pos.unmakeMove(moveStack[i]);
		movesSearched++;

		if (!searching)
		{
			return drawScore;
		}

		if (value > bestScore)
		{
			bestScore = value;
			bestMove = moveStack[i].getMove();
			if (value > alpha)
			{
				// Update the history heuristic when a move which improves alpha is found.
				// Don't update if the move is not a quiet move.
				if ((pos.getPiece(moveStack[i].getTo()) == Empty) && ((moveStack[i].getPromotion() == Empty) || (moveStack[i].getPromotion() == King)))
				{
					butterfly[pos.getSideToMove()][moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
					if (value >= beta)
					{
						if (moveStack[i].getMove() != pos.getKiller(0, ply))
						{
							pos.setKiller(1, ply, pos.getKiller(0, ply));
							pos.setKiller(0, ply, moveStack[i].getMove());
						}
					}
				}

				if (value >= beta)
				{
					ttSave(pos, ply, depth, value, ttBeta, bestMove);
					return value;
				}

				alpha = value;
				ttSave(pos, ply, depth, value, ttAlpha, bestMove);

				int searchTime = (int)t.getms();
				reconstructPV(pos, pv);
				if (isMateScore(alpha))
				{
					int v;
					alpha > 0 ? v = ((mateScore - alpha + 1) >> 1) : v = ((-alpha - mateScore) >> 1);
					cout << "info " << "depth " << searchDepth << " score mate " << v << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << " pv ";
					displayPV(pv, (int)pv.size());
				}
				else
				{
					cout << "info " << "depth " << searchDepth << " score cp " << alpha << " time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << " pv ";
					displayPV(pv, (int)pv.size());
				}
			}
		}
	}

	ttSave(pos, ply, depth, bestScore, ttExact, bestMove);

	return bestScore;
}