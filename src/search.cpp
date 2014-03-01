#ifndef SEARCH_CPP
#define SEARCH_CPP

#include "search.h"
#include "eval.h"
#include <iomanip>
#include "see.h"
#include "hash.h"
#include "time.h"
#include "uci.h"
#include "ttable.h"
#include <windows.h>
#include "tbprobe.h"

U64 nodeCount = 0;

// triangular pv table
Move pv[600][600];
int pvLength[600];
bool followpv;

int lastpvLength;
Move lastpv[600];

// history heuristic
int whiteButterfly[64][64];
int blackButterfly[64][64];

// boolean switch for null move pruning
bool allowNullMove;

// killer heuristic
Move killer[2][600];

U64 searchtime;

int searchdepth;

bool useTB = false;
int tbhits = 0;

bool singleMoveRoot = false;

int searchRoot(int depth, int alpha, int beta);

string numberToNotation[64] = { 
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

string numberToPromotion[16] = {
	"", "", "", "n", "", "b", "r", "q", 
	"", "", "", "n", "", "b", "r", "q", 
};

using namespace std;

int perft(int depth) {
	
	int i;

	if (depth == 0) {
		return 1;
	}	

	if (inCheck(sideToMove))
	{
		generateEvasions();
	}
	else
	{
		generateMoves();	
	}

	// generateMoves();
	for (i = moveList[ply]; i < moveList[ply + 1]; i++) 
	{
		if (!(make(moveStack[i].moveInt))) // if the move is not legal just continue with the loop
		{
			continue;
		}
		nodeCount += perft(depth - 1);
		unmake(moveStack[i].moveInt);	
	}	
    return 0;
}	

// repetitionCount is used to detect twofold repetitions of the current position
int repetitionCount()
{
    for (int i = hply - 2; i >= (hply - fiftyMoveDistance) && i >= 0; i -= 2)   // we can skip every position where sideToMove is different from the current one. Also we don't need to go further than hply-fifty because the position must be different
    {
		if (historyStack[i].Hash == Hash) 
		{
			return true;
		}
    }
	return false;
}

// Orders the movelist from best move to worst move. The order is:
// 1. PV-move, if any
// 2. hash move, if any
// 3. good captures/promotions
// 4. equal captures/promotions
// 5. killer moves
// 6. quiet moves sorted by history heuristic
// 7. bad captures/promotions
void orderMoves(int &ttMove)
{
	int i;

	for (i = moveList[ply]; i < moveList[ply+1]; i++)
	{
		if (followpv && moveStack[i].moveInt == lastpv[ply].moveInt)
		{
			moveStack[i].score = Infinity;
		}
		else if (ttMove != -1 && moveStack[i].moveInt == ttMove)
		{
			moveStack[i].score = HashMove; 
		}
		else if (moveStack[i].isCapture() || moveStack[i].isPromotion())
		{
			moveStack[i].score = SEE(moveStack[i]);
			if (moveStack[i].score >= 0)
				moveStack[i].score += CaptureMove; 
		}
		else if (moveStack[i].moveInt == killer[0][ply].moveInt)
		{
			moveStack[i].score = KillerMove1;
		}
		else if (moveStack[i].moveInt == killer[1][ply].moveInt)
		{
			moveStack[i].score = KillerMove2;
		}
		else if (ply > 1 && moveStack[i].moveInt == killer[0][ply-2].moveInt)
		{
			moveStack[i].score = KillerMove3;
		}
		else if (ply > 1 && moveStack[i].moveInt == killer[1][ply-2].moveInt)
		{
			moveStack[i].score = KillerMove3;
		}
		else 
		{
			if (sideToMove)
				moveStack[i].score = blackButterfly[moveStack[i].getFrom()][moveStack[i].getTo()];
			else
				moveStack[i].score = whiteButterfly[moveStack[i].getFrom()][moveStack[i].getTo()];
		}
	}
}

// only to be used on root with the tablebases
void orderMovesSyzygy()
{
	int i, j = 0;

	for (i = moveList[ply]; i < moveList[ply+1]; i++)
	{
		if (moveStack[i].score != -mateScore)
		{
			moveStack[j].moveInt = moveStack[i].moveInt;
			moveStack[j++].score = moveStack[i].score;
		}
	}
	moveList[ply+1] = j;
}

void selectMove(int &i)
{
	int j;
	int k = i;

	int best = moveStack[i].score;
	for (j = i+1; j < moveList[ply + 1]; j++)
	{
		if (moveStack[j].score > best)
		{
			best = moveStack[j].score;
			k = j;
		}
	}
	if (k > i)
	{
		int temp = moveStack[k].moveInt;
		int temp1 = moveStack[k].score;
		moveStack[k].moveInt = moveStack[i].moveInt;
		moveStack[k].score = moveStack[i].score;
		moveStack[i].moveInt = temp;
		moveStack[i].score = temp1;
	}

	if (best == Infinity)
		followpv = true;
	else
		followpv = false;
}

// selects the capture with the highest SEE score and puts it as the first move to try
void selectcapture(int &i)
{
	int j, k, best, temp, temp1;

	if (followpv)
	{
		followpv = false;
		for (j = i; j < moveList[ply + 1]; j++)
		{
			if (moveStack[j].moveInt == lastpv[ply].moveInt)
			{
				followpv = true;
				temp = moveStack[j].moveInt;
				temp1 = moveStack[j].score;
				moveStack[j].moveInt = moveStack[i].moveInt;
				moveStack[j].score = moveStack[i].score;
				moveStack[i].moveInt = temp;
				moveStack[i].score = temp1;
				return;
			}
		}
	}

	best = moveStack[i].score;
	j = i;
	for (k = i + 1; k < moveList[ply + 1]; k++)
	{
		if (moveStack[k].score > best)
		{
			best = moveStack[k].score;
			j = k;
		}
	}
	if (j > i)
	{
		temp = moveStack[j].moveInt;
		temp1 = moveStack[j].score;
		moveStack[j].moveInt = moveStack[i].moveInt;
		moveStack[j].score = moveStack[i].score;
		moveStack[i].moveInt = temp;
		moveStack[i].score = temp1;
		return;
	}
}

// displays the pv in the format UCI-protocol wants(from, to, if promotion add what promotion)
void displayPV()
{
	int i, from, to, promotion;

	for (i = 0; i < lastpvLength; i++)
	{
		from = lastpv[i].getFrom();
		to = lastpv[i].getTo();

		if (lastpv[i].isPromotion())
		{
			promotion = lastpv[i].getPromotion();
			std::cout << numberToNotation[from] << numberToNotation[to] << numberToPromotion[promotion] << " ";
		}
		else 
		{
			std::cout << numberToNotation[from] << numberToNotation[to] << " ";
		}
	}
}

void think()
{
	int score, alpha, beta;

	// ply is simply the distance from root, so every time we start searching from the root we must reset ply to zero
	ply = 0; 
	// are the following four resets necessary?
	lastpvLength = 0;
    memset(lastpv, 0 , sizeof(lastpv));
	memset(whiteButterfly, 0, sizeof(whiteButterfly));
	memset(blackButterfly, 0, sizeof(blackButterfly));
	memset(killer, 0, sizeof(killer));
	nodeCount = 0;
	tbhits = 0;
	countdown = stopinterval;

	alpha = -(mateScore + 1);
	beta = mateScore + 1;

	t.reset();
	t.start();

	for (searchdepth = 1;;)
	{
		// are the following resets necessary?
		memset(moveList, 0, sizeof(moveList));
        memset(moveStack, 0, sizeof(moveStack));
		memset(pvLength, 0, sizeof(pvLength));
        memset(pv, 0, sizeof(pv));

		followpv = true;
		allowNullMove = true;
		singleMoveRoot = false;

		score = searchRoot(searchdepth, alpha, beta);

		searchtime = t.getms();

		if (timedout || Searching == false)
		{
			cout << "info " << "time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << endl;
			int from = lastpv[0].getFrom();
			int to = lastpv[0].getTo();

			if (lastpv[0].isPromotion())
			{
				int promotion = lastpv[0].getPromotion();
				std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << numberToPromotion[promotion] << endl;
			}
			else 
			{
				std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << endl;
			}
			return;
		}
		// if we've used more than 70% of our time already don't start a new iteration as it most likely won't finish
		else if (t.getms() > (stopfrac * maxtime))
		{
			cout << "info " << "time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << endl;
			int from = lastpv[0].getFrom();
			int to = lastpv[0].getTo();

			if (lastpv[0].isPromotion())
			{
				int promotion = lastpv[0].getPromotion();
				std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << numberToPromotion[promotion] << endl;
			}
			else 
			{
				std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << endl;
			}
			return;
		}

		// if our score is outside the aspiration window do a research with no windows
		if (score <= alpha)
		{
			alpha = -(mateScore + 1);
			if (score >= mateScore - 600)
			{
				int v = ((mateScore - score + 1) >> 1);
				cout << "info " << "depth " << searchdepth << " score mate " << v << " upperbound " << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
				displayPV();
				cout << "" << endl;
			}
			else if (score <= -mateScore + 600)
			{
				int v = ((-score - mateScore) >> 1);
				cout << "info " << "depth " << searchdepth << " score mate " << v << " upperbound " << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
				displayPV();
				cout << "" << endl;
			}
			else
			{
				cout << "info " << "depth " << searchdepth << " score cp " << score << " upperbound " << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
				displayPV();
				cout << "" << endl;
			}
			continue;
		}
		if (score >= beta)
		{
			beta = mateScore + 1;
			if (score >= mateScore - 600)
			{
				int v = ((mateScore - score + 1) >> 1);
				cout << "info " << "depth " << searchdepth << " score mate " << v << " lowerbound " << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
				displayPV();
				cout << "" << endl;
			}
			else if (score <= -mateScore + 600)
			{
				int v = ((-score - mateScore) >> 1);
				cout << "info " << "depth " << searchdepth << " score mate " << v << " lowerbound " << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
				displayPV();
				cout << "" << endl;
			}
			else
			{
				cout << "info " << "depth " << searchdepth << " score cp " << score << " lowerbound " << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
				displayPV();
				cout << "" << endl;
			}
			continue;
		}
	
		// remember the last PV
		lastpvLength = pvLength[0];
		for (int i = 0; i < pvLength[0]; i++)
		{
			lastpv[i].moveInt = pv[0][i].moveInt;
		}
		// instead of (nodeCount / searchtime) we have (nodeCount / (searchtime + 1)) to prevent division by zero. Not fixing this WILL lead to extremely frequent crashes.
		if (score >= mateScore - 600)
		{
			int v = ((mateScore - score + 1) >> 1);
			cout << "info " << "depth " << searchdepth << " score mate " << v << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
			displayPV();
			cout << "" << endl;
		}
		else if (score <= -mateScore + 600)
		{
			int v = ((-score - mateScore) >> 1);
			cout << "info " << "depth " << searchdepth << " score mate " << v << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
			displayPV();
			cout << "" << endl;
		}
		else
		{
			cout << "info " << "depth " << searchdepth << " score cp " << score << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
			displayPV();
			cout << "" << endl;
		}

		if (singleMoveRoot && !Infinite)
		{
			singleMoveRoot = false;

			cout << "info " << "time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000  << " tbhits " << tbhits << endl;
			int from = lastpv[0].getFrom();
			int to = lastpv[0].getTo();
			if (lastpv[0].isPromotion())
			{
				int promotion = lastpv[0].getPromotion();
				std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << numberToPromotion[promotion] << endl;
			}
			else 
			{
				std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << endl;
			}
			return;
		}
		searchdepth++;
		// adjust alpha and beta based on the last score
		// if we found a mate we dont use aspiration windows anymore
		// score-1/score+1 is necessary in case of no faster mate as it will just lead to no pv
		// don't adjust if depth is low - it's a waste of time
		if (searchdepth > 4)
		{
			if (score >= mateScore - 600)
			{
				alpha = score-1;
				beta = mateScore+1;
			}
			else if (score <= -mateScore + 600)
			{
				alpha = -(mateScore + 1);
				beta = score+1;
			}
			else
			{
				alpha = score - aspirationWindow;
				beta = score + aspirationWindow;
			}
		}
		else 
		{
			if (score >= mateScore - 600 || score <= -mateScore + 600)
			{	
				cout << "info " << "time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << endl;
				if (!Infinite)
				{
					cout << "info " << "time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << endl;
					int from = lastpv[0].getFrom();
					int to = lastpv[0].getTo();
					if (lastpv[0].isPromotion())
					{
						int promotion = lastpv[0].getPromotion();
						std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << numberToPromotion[promotion] << endl;
					}
					else 
					{
						std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << endl;
					}
				}
				else 
				{
					while(true)
					{
						Sleep(100);
						readClockAndInput();
						if (!Searching)
							break;
					}

					int from = lastpv[0].getFrom();
					int to = lastpv[0].getTo();
					if (lastpv[0].isPromotion())
					{
						int promotion = lastpv[0].getPromotion();
						std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << numberToPromotion[promotion] << endl;
					}
					else 
					{
						std::cout << "bestmove " << numberToNotation[from] << numberToNotation[to] << endl;
					}
				}
				return;
			}
		}
		
	}

	t.stop();
	return;
}

// quiescence search
 int qsearch(int alpha, int beta)
{
	int i, j, value, bestscore;

	if (timedout || Searching == false)
	{
		return 0;
	}

	pvLength[ply] = ply;

	value = eval();
	if (value >= beta)
	{
		return value;
	}

	// first part of delta pruning: we test if ANY move can improve alpha. If it cannot we may safely exit. 
	// seems like this is making the program play worse (a bit), test
	/*
	if (value + 1500 < alpha)
	{
		return alpha;
	}
	*/

	bestscore = value;
	if (value > alpha) 
	{
		alpha = value;
	}

	generateCaptures();
	for (i = moveList[ply]; i < moveList[ply+1]; i++)
	{
		selectcapture(i);
		// the second part of delta pruning, also the more important part
		// we can return alpha straight away as all captures following a failure are equal or worse to it - that is they will fail as well
		if (value + moveStack[i].score + deltaPruningSafetyMargin < alpha)
		{
			return bestscore;
		}
		
		if (!make(moveStack[i].moveInt))
		{
			continue;
		}
		nodeCount++;
		if (--countdown <= 0) 
		{ 
			readClockAndInput();
		}
		value = -qsearch(-beta, -alpha);
		unmake(moveStack[i].moveInt);
		if (value >= beta) 
		{
			return value;
		}
		if (value > bestscore)
		{
			bestscore = value;
			if (value > alpha)
			{
				alpha = value;
				pv[ply][ply].moveInt = moveStack[i].moveInt;
				for (j = ply + 1; j < pvLength[ply+1]; j++)
				{
					pv[ply][j].moveInt = pv[ply+1][j].moveInt;
				}
				pvLength[ply] = pvLength[ply+1];
			}
		}
	}
	return bestscore;
}

int alphabetaPVS(int depth, int alpha, int beta)
{
	bool movesFound, pvFound;
	U64 check;
	int value;
	int ttMove = Invalid;
	int bestmove = -1;
	unsigned __int8 ttFlag = ttAlpha;
	bool ttAllowNull = true;
	pvLength[ply] = ply;
	int bestscore = -mateScore;

	_mm_prefetch((char *)&tt[Hash % ttSize], _MM_HINT_NTA);

	// mate distance pruning
	alpha = max(alpha, -mateScore + ply);
	beta = min(beta, mateScore - ply - 1);
	if (alpha >= beta)
		return alpha;

	// check extension
	// makes sure that the quiescence search is NEVER started while being in check
	check = inCheck(sideToMove);
	if (check)
	{
		depth++;
	}
	
	if (depth <= 0) 
	{
		return qsearch(alpha, beta);
	}

	if (repetitionCount()) 
	{
		return drawscore;
	}

	if ((value = ttProbe(depth, &alpha, &beta, &ttMove, &ttAllowNull)) != Invalid)
	{
		return value;
	}

	// probe the tablebases 
	if (popcnt(occupiedSquares) <= syzygyProbeLimit && useTB) 
	{
		int success;
		int v = probe_wdl(&success);
		if (success) 
		{
			tbhits++;
			if (v < -1) value = -mateScore + ply + 200;
			else if (v > 1) value = mateScore - ply - 200;
			else value = drawscore + v;
			return value;
		}
    }

	// null move pruning
	if (!followpv && allowNullMove && ttAllowNull)
    {
		if (!check)
        {
			if (phase != 256)
            {
				// static null move pruning
				if (depth <= 3)
				{
					int staticEval = eval();
					if (depth == 1 && staticEval - 260 >= beta)
						return staticEval;
					else if (depth == 2 && staticEval - 445 >= beta)
						return staticEval;
					else if (depth == 3 && staticEval - 900 >= beta)
						depth--;
				}
				allowNullMove = false;
				nodeCount++;
				if (--countdown <= 0) 
				{ 
					readClockAndInput();
				}
				sideToMove = !sideToMove;
				Hash ^= side;
				if (depth <= 3)
					value = -qsearch(-beta, -beta + 1);
				else 
					value = -alphabetaPVS(depth - 1 - (depth > 6 ? 3 : 2), -beta, -beta + 1);
				sideToMove = !sideToMove;
				Hash ^= side;
				if (timedout || Searching == false)
				{
					return 0;
				}
				allowNullMove = true;
				if (value >= beta) 
				{
					/*
					if (depth >= 6)
					{
						int v = -alphabetaPVS(depth - 5, -beta-1, -beta);
						if (v >= beta)
							return value;
					}
					else
					{
						return value;
					}
					*/
					return value;
				}
			 }
		}
	}
	
	// internal iterative deepening
	if (!followpv && (alpha + 1) != beta && ttMove == Invalid && depth > 2 && Searching)
	{
		value = alphabetaPVS(depth-2, alpha, beta);
		if (value <= alpha)
		{
			value = alphabetaPVS(depth-2, -(mateScore + 1), beta);
		}
		ttProbe(depth, &alpha, &beta, &ttMove, &ttAllowNull);
	}
	
	allowNullMove = true;
	
	pvFound = false;
	movesFound = false;
	
	if (check)
	{
		generateEvasions();
		// singular reply extension
		if (moveList[ply + 1] - moveList[ply] == 1)
		{
			depth++;
		}
	}
	else 
	{
		generateMoves();
	}

	orderMoves(ttMove);
	for (int i = moveList[ply]; i < moveList[ply + 1]; i++)
	{
		selectMove(i);
		if (!(make(moveStack[i].moveInt))) 
		{
			continue;
		}
		nodeCount++;
		if (--countdown <= 0) 
		{ 
			readClockAndInput();
		}
		movesFound = true;
		if (pvFound)
		{
			value = -alphabetaPVS(depth-1, -alpha-1, -alpha); // zero window search 
			if (value > alpha && value < beta) 
			{
				value = -alphabetaPVS(depth-1, -beta, -alpha); // in case of failure research with a normal window
			}
		}
        else
		{
			value = -alphabetaPVS(depth-1, -beta, -alpha); // normal alpha beta, used until a pv is found
		}
        unmake(moveStack[i].moveInt);
		if (timedout || Searching == false)
		{
			return 0;
		}
        if (value >= beta) 
		{
			// update the history heuristic when a move which causes a beta-cutoff occurs
			if (!moveStack[i].isCapture() && !moveStack[i].isPromotion())
			{
				if (sideToMove)
				{
					blackButterfly[moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
				}
				else
				{
					whiteButterfly[moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
				}

				if (moveStack[i].moveInt != killer[0][ply].moveInt)
				{
					killer[1][ply] = killer[0][ply];
					killer[0][ply] = moveStack[i];
				}
			}
			ttSave(depth, value, ttBeta, moveStack[i].moveInt);
			return value;
		}
		if (value > bestscore)
		{
			bestscore = value;
			if (value > alpha)
			{
				alpha = value;
				ttFlag = ttExact;
				bestmove = moveStack[i].moveInt;
				pvFound = true;
				pv[ply][ply].moveInt = moveStack[i].moveInt;                 
				for (int j = ply + 1; j < pvLength[ply + 1]; ++j)
				{
					pv[ply][j].moveInt = pv[ply+1][j].moveInt;   
				}
				pvLength[ply] = pvLength[ply + 1];

				// update the history heuristic when a move which improves alpha is found
				if (!moveStack[i].isCapture() && !moveStack[i].isPromotion())
				{
					if (sideToMove)
					{
						blackButterfly[moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
					}
					else
					{
						whiteButterfly[moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
					}
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
			bestscore = drawscore;
		}
	}
	if (fiftyMoveDistance >= 100)
	{
		bestscore = drawscore;
	}

	ttSave(depth, bestscore, ttFlag, bestmove);

	return bestscore;
}

int searchRoot(int depth, int alpha, int beta)
{
	int value;
	U64 check;
	int legalmoves = 0;
	bool pvFound = false;
	int ttMove = -1;
	int bestscore = -mateScore;

	pvLength[ply] = ply;

	check = inCheck(sideToMove);
	if (check)
	{
		depth++;
		generateEvasions();
	}
	else
	{
		generateMoves();
	}

	// if we are within the syzygy tablebases only search moves which preserve the correct wdl result
	// also do not probe inside the search
	if (popcnt(occupiedSquares) <= syzygyProbeLimit && dtzPathGiven)
	{
		if (root_probe())
		{
			orderMovesSyzygy();
			useTB = false;
		}
	}
	orderMoves(ttMove);
	for (int i = moveList[ply]; i < moveList[ply + 1]; i++)
	{
		selectMove(i);
		if (!(make(moveStack[i].moveInt))) 
		{
			continue;
		}
		nodeCount++;
		legalmoves++;

		if (--countdown <= 0) 
		{ 
			readClockAndInput();
		}

		if (pvFound)
		{
			value = -alphabetaPVS(depth-1, -alpha-1, -alpha); // zero window search 
			if (value > alpha && value < beta) 
			{
				value = -alphabetaPVS(depth-1, -beta, -alpha); // in case of failure research with a normal window
			}
		}
        else
		{
			value = -alphabetaPVS(depth-1, -beta, -alpha); // normal alpha beta, used until a pv is found
		}

        unmake(moveStack[i].moveInt);

		if (timedout || Searching == false)
		{
			return 0;
		}

        if (value >= beta) 
		{
			// update the history heuristic when a move which causes a beta-cutoff occurs
			if (!moveStack[i].isCapture() && !moveStack[i].isPromotion())
			{
				if (sideToMove)
				{
					blackButterfly[moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
				}
				else
				{
					whiteButterfly[moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
				}

				if (moveStack[i].moveInt != killer[0][ply].moveInt)
				{
					killer[1][ply] = killer[0][ply];
					killer[0][ply] = moveStack[i];
				}
			}
			return value;
		}
		if (value > bestscore)
		{
			bestscore = value;
			if (value > alpha)
			{
				alpha = value;
				pvFound = true;
				pv[ply][ply].moveInt = moveStack[i].moveInt;                 
				for (int j = ply + 1; j < pvLength[ply + 1]; ++j)
				{
					pv[ply][j].moveInt = pv[ply+1][j].moveInt;   
				}
				pvLength[ply] = pvLength[ply + 1];

				// update the history heuristic when a move which improves alpha is found
				if (!moveStack[i].isCapture() && !moveStack[i].isPromotion())
				{
					if (sideToMove)
					{
						blackButterfly[moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
					}
					else
					{
						whiteButterfly[moveStack[i].getFrom()][moveStack[i].getTo()] += depth*depth;
					}
				}

				// remember the last PV
				lastpvLength = pvLength[0];
				for (int i = 0; i < pvLength[0]; i++)
				{
					lastpv[i] = pv[0][i];
				}
				searchtime = t.getms();
				if (alpha >= mateScore - 600)
				{
					int v = ((mateScore - alpha + 1) >> 1);
					cout << "info " << "depth " << searchdepth << " score mate " << v << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
					displayPV();
					cout << "" << endl;
				}
				else if (alpha <= -mateScore + 600)
				{
					int v = ((-alpha - mateScore) >> 1);
					cout << "info " << "depth " << searchdepth << " score mate " << v << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
					displayPV();
					cout << "" << endl;
				}
				else
				{
					cout << "info " << "depth " << searchdepth << " score cp " << alpha << " time " << searchtime << " nodes " << nodeCount << " nps " << (nodeCount / (searchtime + 1)) * 1000 << " tbhits " << tbhits << " pv ";
					displayPV();
					cout << "" << endl;
				}
			}
		}
	}

	if (legalmoves == 1)
		singleMoveRoot = true;

	return bestscore;
}

#endif