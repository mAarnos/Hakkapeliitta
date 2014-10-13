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

array<int, Squares> history[12];
array<int, Squares> butterfly[12];
vector<Move> pv;

int lastRootScore;
int bestRootScore;

std::array<int, 256> reductions;

int searchRoot(Position & pos, int ply, int depth, int alpha, int beta);

void initSearch()
{
    for (auto i = 0; i < 256; ++i)
    {
        reductions[i] = static_cast<int>(std::max(1.0, std::round(std::log(i + 1))));
    }
}

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
            auto relativeHistoryScore = history[pos.getPiece(moveStack[i].getFrom())][moveStack[i].getTo()] * 100 /
                (butterfly[pos.getPiece(moveStack[i].getFrom())][moveStack[i].getTo()] + 1);
            assert(relativeHistoryScore < killerMove4);
			moveStack[i].setScore(relativeHistoryScore);
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
	int alpha, beta, delta;

	tt.startNewSearch();
	
	searching = true;
    for (auto & row : history)
    {
        for (auto & column : row)
        {
            column /= 2;
        }
    }
    for (auto & row : butterfly)
    {
        for (auto & column : row)
        {
            column /= 2;
        }
    }

    root.resetKillers();
	nodeCount = 0;
	tbHits = 0;
	countDown = stopInterval;

	alpha = -infinity;
	beta = infinity;
    delta = aspirationWindow;

	t.reset();
	t.start();

	for (searchDepth = 1;;)
	{
		int score = lastRootScore = searchRoot(root, 0, searchDepth, alpha, beta);
		reconstructPV(root, pv);

		uint64_t searchTime = t.getms();

		if (searching == false || searchDepth >= 64)
		{
			cout << "info " << "time " << searchTime << " nodes " << nodeCount << " nps " << (nodeCount / (searchTime + 1)) * 1000 << " tbhits " << tbHits << endl << "bestmove ";
			displayPV(pv, 1);
			return;
		}

		if (score <= alpha)
		{
			alpha -= delta;
            delta *= 2;
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
			beta += delta;
            delta *= 2;
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
		if (searchDepth >= 4 && !isMateScore(score))
		{
			alpha = score - aspirationWindow;
			beta = score + aspirationWindow;
		}
        else
        {
            alpha = -infinity;
            beta = infinity;
        }

        delta = aspirationWindow;
		searchDepth++;
	}
}

int qsearch(Position & pos, int ply, int alpha, int beta, bool inCheck)
{
    int score, bestScore, delta;
    int generatedMoves;
    Move moveStack[64];

    if (inCheck)
    {
        bestScore = -mateScore + ply;
        generatedMoves = generateEvasions(pos, moveStack);
        orderMoves(pos, moveStack, generatedMoves, ttMoveNone, ply);
    }
    else
    {
        bestScore = eval(pos);
        if (bestScore > alpha)
        {
            if (bestScore >= beta)
            {
                return bestScore;
            }
            alpha = bestScore;
        }
        delta = bestScore + futilityMargin[0];
        generatedMoves = generateCaptures(pos, moveStack);
        orderCaptures(pos, moveStack, generatedMoves);
    }

    for (int i = 0; i < generatedMoves; i++)
    {
        selectMove(moveStack, generatedMoves, i);
        if (!inCheck) // If we are in check try all moves.
        {
            // We only try good captures, so if we have reached the bad captures we can stop.
            if (moveStack[i].getScore() < 0)
            {
                break;
            }
            // Delta pruning. Check if the score is below the delta-pruning safety margin.
            // we can return alpha straight away as all captures following a failure are equal or worse to it - that is they will fail as well
            if (delta + moveStack[i].getScore() < alpha)
            {
                break;
            }
        }

        if (!(pos.makeMove(moveStack[i])))
        {
            continue;
        }

        nodeCount++;
        auto givesCheck = pos.inCheck();
        score = -qsearch(pos, ply + 1, -beta, -alpha, givesCheck);
        pos.unmakeMove(moveStack[i]);

        if (score > bestScore)
        {
            if (score > alpha)
            {
                if (score >= beta)
                {
                    return score;
                }
                alpha = score;
            }
            bestScore = score;
        }
    }

    return bestScore;
}

int alphabetaPVS(Position & pos, int ply, int depth, int alpha, int beta, int allowNullMove, bool inCheck)
{
	bool pvNode = ((alpha + 1) != beta), futileNode = false, lmpNode = false;
	int score, generatedMoves, staticEval, ttFlag = ttAlpha, bestScore = -mateScore, movesSearched = 0, prunedMoves = 0;
	uint16_t ttMove = ttMoveNone, bestMove = ttMoveNone;
    auto oneReply = false;

    prefetch(pos.getHash());

	// Check if we have overstepped the time limit or if the user has given a new order.
	if (countDown-- <= 0)
	{
		checkTimeAndInput();
	}

	// Check for aborted search(either due to going beyond the allocated time or a new command) and repetition+fifty move draws.
	if (!searching || pos.repetitionDraw())
	{
		return contempt(pos.getSideToMove());
	}

	// Mate distance pruning.
	alpha = max(-mateScore + ply, alpha);
	beta = min(mateScore - ply - 1, beta);
	if (alpha >= beta)
		return alpha;

	// If we have gone as far as we wanted to go drop into quiescence search.
	if (depth <= 0)
	{
		return qsearch(pos, ply, alpha, beta, inCheck);
	}

	// Probe the transposition table.
	if ((score = ttProbe(pos, ply, depth, alpha, beta, ttMove, allowNullMove)) != probeFailed)
	{
		return score;
	}

	// Probe the Syzygy tablebases.
	if (probeTB && popcnt(pos.getOccupiedSquares()) <= syzygyProbeLimit)
	{
		int found, v = probe_wdl(pos, &found);
		if (found)
		{
			tbHits++;
			if (v < -1) score = -mateScore + ply + 200;
			else if (v > 1) score = mateScore - ply - 200;
			else score = contempt(pos.getSideToMove()) + v;
			ttSave(pos, ply, depth + 4, score, ttExact, ttMoveNone);
			return score;
		}
	}

	// Get the static evaluation of the position.
    staticEval = (inCheck ? 0 : eval(pos));

    // Reverse futility pruning.
    if (!pvNode && !inCheck && pos.getRawPhase() != totalPhase 
        && depth <= staticNullMoveDepth && staticEval - staticNullMoveMargin[depth] >= beta)
        return staticEval - staticNullMoveMargin[depth];

	// Double null move pruning.
    if (!pvNode && allowNullMove && !inCheck && pos.getRawPhase() != totalPhase)
	{
		pos.makeNullMove();
		nodeCount++;
		if (depth <= 4)
		{
			score = -qsearch(pos, ply + 1, -beta, -beta + 1, false);
		}
		else
		{
			score = -alphabetaPVS(pos, ply + 1, depth - 1 - nullReduction, -beta, -beta + 1, allowNullMove - 1, false);
		}
		pos.unmakeNullMove();

		if (!searching)
		{
			return 0;
		}

		if (score >= beta)
		{
			ttSave(pos, ply, depth, score, ttBeta, ttMoveNone);
			return score;
		}
	}

    // Razoring.
    if (!inCheck && depth <= razoringDepth && staticEval <= alpha - razoringMargin[depth])
    {
        auto razoringAlpha = alpha - razoringMargin[depth];
        score = qsearch(pos, ply, razoringAlpha, razoringAlpha + 1, false);
        if (score <= razoringAlpha)
            return score;
    }

	// Internal iterative deepening.
	if (pvNode && ttMove == ttMoveNone && depth > 2)
	{
        // We can safely skip null move in IID since if it would have worked we wouldn't be here.
		score = alphabetaPVS(pos, ply, depth - 2, alpha, beta, 0, inCheck);
		if (score <= alpha)
		{
			score = alphabetaPVS(pos, ply, depth - 2, -infinity, beta, 0, inCheck);
		}
		ttProbe(pos, ply, depth, alpha, beta, ttMove, allowNullMove);
	}

	if (!inCheck && depth <= futilityDepth && staticEval + futilityMargin[depth] <= alpha)
	{
		futileNode = true;
	}

    if (!pvNode && !inCheck && depth <= lmpDepth)
    {
        lmpNode = true;
    }
	
	Move moveStack[256];
	if (inCheck)
	{
		generatedMoves = generateEvasions(pos, moveStack);
        if (generatedMoves == 1)
            oneReply = true;
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

		int newDepth = depth - 1;
		bool givesCheck = pos.inCheck();
        auto extension = (givesCheck || oneReply) ? 1 : 0;
        newDepth += extension;

        if (!extension && moveStack[i].getScore() < killerMove4 && moveStack[i].getScore() >= 0)
		{
            if (futileNode || (lmpNode && movesSearched >= lmpMoveCount[depth]))
            {
                pos.unmakeMove(moveStack[i]);
                prunedMoves++;
                continue;
            }
		}

		if (!movesSearched)
		{
			score = -alphabetaPVS(pos, ply + 1, newDepth, -beta, -alpha, 2, givesCheck);
		}
		else
		{
			if (movesSearched >= fullDepthMoves && depth >= reductionLimit
                && !inCheck && !extension && moveStack[i].getScore() < killerMove4 && moveStack[i].getScore() >= 0)
			{
                // Progressively reduce later moves more and more.
                auto reduction = reductions[movesSearched - fullDepthMoves];
                score = -alphabetaPVS(pos, ply + 1, newDepth - reduction, -alpha - 1, -alpha, 2, givesCheck);
			}
			else
			{
				score = alpha + 1; // Hack to ensure that full-depth search is done.
			}

			if (score > alpha)
			{
                score = -alphabetaPVS(pos, ply + 1, newDepth, -alpha - 1, -alpha, 2, givesCheck);
				if (score > alpha && score < beta)
				{
                    score = -alphabetaPVS(pos, ply + 1, newDepth, -beta, -alpha, 2, givesCheck);
				}
			}
		}
		pos.unmakeMove(moveStack[i]);
		movesSearched++;

		if (!searching)
		{
			return 0;
		}

		if (score > bestScore)
		{
			bestScore = score;
			if (score > alpha)
			{
                bestMove = moveStack[i].getMove();
				if (score >= beta)
				{
					ttSave(pos, ply, depth, score, ttBeta, bestMove);
                    // Update the history heuristic when a move which causes a cutoff.
                    // Don't update if the move is not a quiet move.
                    if ((pos.getPiece(moveStack[i].getTo()) == Empty) && ((moveStack[i].getPromotion() == Empty) || (moveStack[i].getPromotion() == King)))
                    {
                        history[pos.getPiece(moveStack[i].getFrom())][moveStack[i].getTo()] += depth * depth;
                        if (moveStack[i].getMove() != pos.getKiller(0, ply))
                        {
                            pos.setKiller(1, ply, pos.getKiller(0, ply));
                            pos.setKiller(0, ply, moveStack[i].getMove());
                        }
                    }
                    for (auto j = 0; j < i; ++j)
                    {
                        if ((pos.getPiece(moveStack[j].getTo()) == Empty) &&
                            ((moveStack[j].getPromotion() == Empty) || (moveStack[j].getPromotion() == King)))
                        {
                            butterfly[pos.getPiece(moveStack[j].getFrom())][moveStack[j].getTo()] += depth * depth;
                        }
                    }
					return score;
				}
				alpha = score;
				ttFlag = ttExact;
			}
		}
	}

	if (!movesSearched)
	{
		if (!prunedMoves)
		{
			return (inCheck ? -mateScore + ply : contempt(pos.getSideToMove()));
		}
		else
		{
			return staticEval;
		}
	}

	ttSave(pos, ply, depth, bestScore, ttFlag, bestMove);

	return bestScore;
}

int searchRoot(Position & pos, int ply, int depth, int alpha, int beta)
{
	int score, generatedMoves, newDepth, movesSearched = 0;
	uint16_t ttMove = ttMoveNone, bestMove = ttMoveNone; 
	bool givesCheck, inCheck = pos.inCheck();
    int allowNullMove; // not used for anything
    bestRootScore = -mateScore;

	// Probe the transposition table to get the PV-Move, if any.
	ttProbe(pos, ply, depth, alpha, beta, ttMove, allowNullMove);

	Move moveStack[256];
	generatedMoves = generateMoves(pos, moveStack);

	// Check if the root position is in tablebases, and if it is remove moves which do not maintain the correct result.
	probeTB = true;
	if (popcnt(pos.getOccupiedSquares()) <= syzygyProbeLimit)
	{
		bool rootInTB = root_probe(pos, score, moveStack, generatedMoves);
		if (rootInTB)
		{
			tbHits += generatedMoves;
			probeTB = false; // Do not probe tablebases during the search
		}
		else // If DTZ tables are missing, use WDL tables as a fallback
		{
			// Filter out moves that do not preserve a draw or win
			rootInTB = root_probe_wdl(pos, score, moveStack, generatedMoves);
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

		newDepth = depth - 1;
		givesCheck = pos.inCheck();
		if (givesCheck)
		{
			newDepth++;
		}

		if (!movesSearched)
		{
            score = -alphabetaPVS(pos, ply + 1, newDepth, -beta, -alpha, 2, givesCheck);
		}
		else
		{
            if (movesSearched >= fullDepthMoves && depth >= reductionLimit
                && !inCheck && !givesCheck && moveStack[i].getScore() < killerMove4 && moveStack[i].getScore() >= 0)
            {
                // Progressively reduce later moves more and more.
                auto reduction = reductions[movesSearched - fullDepthMoves];
                score = -alphabetaPVS(pos, ply + 1, newDepth - reduction, -alpha - 1, -alpha, 2, givesCheck);
            }
			else
			{
				score = alpha + 1; // Hack to ensure that full-depth search is done.
			}

			if (score > alpha)
			{
                score = -alphabetaPVS(pos, ply + 1, newDepth, -alpha - 1, -alpha, 2, givesCheck);
				if (score > alpha && score < beta)
				{
                    score = -alphabetaPVS(pos, ply + 1, newDepth, -beta, -alpha, 2, givesCheck);
				}
			}
		}
		pos.unmakeMove(moveStack[i]);
		movesSearched++;

		if (!searching)
		{
			return 0;
		}

        if (score > bestRootScore)
		{
            bestRootScore = score;
			if (score > alpha)
			{
                bestMove = moveStack[i].getMove();
				if (score >= beta)
				{
					ttSave(pos, ply, depth, score, ttBeta, bestMove);
                    // Update the history heuristic when a move which improves alpha is found.
                    // Don't update if the move is not a quiet move.
                    if ((pos.getPiece(moveStack[i].getTo()) == Empty) && ((moveStack[i].getPromotion() == Empty) || (moveStack[i].getPromotion() == King)))
                    {
                        history[pos.getPiece(moveStack[i].getFrom())][moveStack[i].getTo()] += depth * depth;
                        if (moveStack[i].getMove() != pos.getKiller(0, ply))
                        {
                            pos.setKiller(1, ply, pos.getKiller(0, ply));
                            pos.setKiller(0, ply, moveStack[i].getMove());
                        }
                    }
                    for (auto j = 0; j < i; ++j)
                    {
                        if ((pos.getPiece(moveStack[j].getTo()) == Empty) &&
                            ((moveStack[j].getPromotion() == Empty) || (moveStack[j].getPromotion() == King)))
                        {
                            butterfly[pos.getPiece(moveStack[j].getFrom())][moveStack[j].getTo()] += depth * depth;
                        }
                    }
					return score;
				}

				alpha = score;
				ttSave(pos, ply, depth, score, ttAlpha, bestMove);

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

    ttSave(pos, ply, depth, bestRootScore, ttExact, bestMove);

    return bestRootScore;
}