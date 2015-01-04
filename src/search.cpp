#include "search.hpp"
#include <iostream>
#include <cmath>
#include <sstream>
#include "eval.hpp"
#include "movegen.hpp"
#include "utils/synchronized_ostream.hpp"
#include "utils/exception.hpp"
#include "syzygy/tbprobe.hpp"

TranspositionTable Search::transpositionTable;
HistoryTable Search::historyTable;
KillerTable Search::killerTable;

int Search::rootPly;
std::array<HashKey, 1024> Search::repetitionHashes;

int Search::contemptValue;
int Search::syzygyProbeLimit;
std::array<int, 2> Search::contempt;
bool Search::searching;
bool Search::pondering;
bool Search::infinite;
int Search::targetTime;
int Search::maxTime;

const int Search::aspirationWindow = 50;
const int Search::nullReduction = 3;
const int Search::futilityDepth = 4;
const std::array<int, 1 + 4> Search::futilityMargins = {
    50, 125, 125, 300, 300
};
const int Search::reverseFutilityDepth = 3;
const std::array<int, 1 + 3> Search::reverseFutilityMargins = {
    0, 260, 445, 900
};
const int Search::lmrFullDepthMoves = 4;
const int Search::lmrReductionLimit = 3;
std::array<int, 256> Search::lmrReductions;
const int Search::lmpDepth = 4;
const std::array<int, 1 + 4> Search::lmpMoveCount = {
    0, 4, 8, 16, 32
};
const int Search::razoringDepth = 3;
const std::array<int, 1 + 3> Search::razoringMargins = {
    0, 125, 300, 300
};

const int32_t Search::hashMoveScore = 2147483647;
const int32_t Search::captureMoveScore = hashMoveScore >> 1; 
const std::array<int32_t, 1 + 4> Search::killerMoveScore = {
    0, hashMoveScore >> 2, hashMoveScore >> 3, hashMoveScore >> 4, hashMoveScore >> 5 
};

bool Search::probeTb;
int Search::tbHits;
int Search::nodeCount;
int Search::nodesToTimeCheck;
int Search::selDepth;
Stopwatch Search::sw;

int lastRootScore;
int currentRootScore;

void Search::initialize()
{
    rootPly = 0;
    contemptValue = 0;
    contempt.fill(0);
    syzygyProbeLimit = 0;
    searching = false;
    infinite = false;
    pondering = false;
    targetTime = maxTime = 0;
    nodeCount = 0;
	tbHits = 0;
    probeTb = true;
    nodesToTimeCheck = 10000;
    selDepth = 1;
    lastRootScore = currentRootScore = -mateScore;

    for (auto i = 0; i < 256; ++i)
    {
        lmrReductions[i] = static_cast<int>(std::max(1.0, std::round(std::log(i + 1))));
    }
}

bool Search::repetitionDraw(const Position& pos, int ply)
{
    auto temp = std::max(rootPly + ply - pos.getFiftyMoveDistance(), 0);

    for (auto i = rootPly + ply - 2; i >= temp; i -= 2)
    {
        if (repetitionHashes[i] == pos.getHashKey())
        {
            return true;
        }
    }

    return false;
}

void removeIllegalMoves(Position& pos, MoveList& moveList)
{
    History history;
    auto marker = 0;

    for (auto i = 0; i < moveList.size(); ++i)
    {
        if (!pos.makeMove(moveList[i], history))
        {
            continue;
        }
        moveList[marker++] = moveList[i];
        pos.unmakeMove(moveList[i], history);
    }

    moveList.resize(marker);
}

void Search::think(const Position& root)
{
    auto pos = root; // Copy the position just for us.
	auto alpha = -infinity;
	auto beta = infinity;
	auto delta = aspirationWindow;
    auto score = matedInPly(0);
	auto inCheck = pos.inCheck();
    auto allowNullMove = 0; // Not used for anything.
	MoveList rootMoveList;
	History history;
	Move bestMove(0, 0, 0, 0);

	tbHits = 0;
    probeTb = true;
    nodeCount = 0;
    nodesToTimeCheck = 10000;
    contempt[pos.getSideToMove()] = -contemptValue;
    contempt[!pos.getSideToMove()] = contemptValue;
    lastRootScore = -mateScore;
    selDepth = 1;
    transpositionTable.startNewSearch();
    historyTable.age();
    killerTable.clear();
	sw.reset();
	sw.start();

	inCheck ? MoveGen::generateLegalEvasions(pos, rootMoveList) 
		    : MoveGen::generatePseudoLegalMoves(pos, rootMoveList);
    removeIllegalMoves(pos, rootMoveList);

    if (pos.getTotalPieceCount() <= syzygyProbeLimit)
    {
        tbHits = rootMoveList.size();

        // If the current root position is in the tablebases then RootMoves
        // contains only moves that preserve the draw or win.
        auto rootInTb = Syzygy::rootProbe(pos, rootMoveList, score);

        if (rootInTb)
        {
            probeTb = false;
        }
        else // If DTZ tables are missing, use WDL tables as a fallback
        {
            // Filter out moves that do not preserve a draw or win.
            rootInTb = Syzygy::rootProbeWdl(pos, rootMoveList, score);
            if (rootInTb)
            { 
                probeTb = false;
            }
            else
            {
                tbHits = 0;
            }
        }
    }

    // Get the tt move from a possible previous search.
    auto ttEntry = transpositionTable.probe(pos);
    if (ttEntry)
    {
        bestMove.setMove(ttEntry->getBestMove());
    }

    repetitionHashes[rootPly] = pos.getHashKey();
    for (auto depth = 1;;)
    {
        auto previousAlpha = alpha;
        auto previousBeta = beta;
		auto movesSearched = 0;
		auto lmrNode = (!inCheck && depth >= lmrReductionLimit);
        currentRootScore = -mateScore;

		orderMoves(pos, rootMoveList, bestMove, 0);
		try {
			for (auto i = 0; i < rootMoveList.size(); ++i)
			{
				selectMove(rootMoveList, i);
				const auto& move = rootMoveList[i];
				if (!pos.makeMove(move, history))
				{
					continue;
				}
                ++nodeCount;
                --nodesToTimeCheck;

				if (depth >= 12)
				{
					infoCurrMove(move, depth, i);
				}

				auto givesCheck = pos.inCheck();
				auto extension = givesCheck ? 1 : 0;
				auto newDepth = depth - 1 + extension;
				auto nonCriticalMove = !extension && move.getScore() >= 0 && move.getScore() < killerMoveScore[4];

				if (!movesSearched)
				{
					score = -search<true>(pos, newDepth, 1, -beta, -alpha, 2, givesCheck);
				}
				else
				{
					auto reduction = ((lmrNode && i >= lmrFullDepthMoves && nonCriticalMove)
						? lmrReductions[i - lmrFullDepthMoves] : 0);

					score = -search<false>(pos, newDepth - reduction, 1, -alpha - 1, -alpha, 2, givesCheck);

                    if (score > alpha)
                    {
                        score = -search<true>(pos, newDepth - reduction, 1, -beta, -alpha, 2, givesCheck);
                        if (reduction && score > alpha)
                        {
                            score = -search<true>(pos, newDepth, 1, -beta, -alpha, 2, givesCheck);
                        }
                    }
				}
				pos.unmakeMove(move, history);
				++movesSearched;

                if (score > currentRootScore)
				{
                    currentRootScore = score;
					if (score > alpha)
					{
						bestMove = move;
						alpha = score;
						if (score >= beta)
						{
                            if (!inCheck)
                            {
                                if (pos.getBoard(move.getTo()) == Piece::Empty && (move.getPromotion() == Piece::Empty || move.getPromotion() == Piece::King))
                                {
                                    historyTable.addCutoff(pos, move, depth);
                                    killerTable.addKiller(move, 0);
                                }
                                for (auto j = 0; j < i; ++j)
                                {
                                    const auto& move2 = rootMoveList[j];
                                    if (pos.getBoard(move2.getTo()) == Piece::Empty && (move2.getPromotion() == Piece::Empty || move2.getPromotion() == Piece::King))
                                    {
                                        historyTable.addNotCutoff(pos, move2, depth);
                                    }
                                }
                            }
							break;
						}
                        transpositionTable.save(pos, 0, bestMove, currentRootScore, depth, UpperBoundScore);
						auto pv = transpositionTable.extractPv(pos);
                        infoPv(pv, depth, currentRootScore, ExactScore);
					}
				}
			}
		}
        catch (const StopSearchException&)
		{
            pos = root; // Exception messes up the position, fix it.
		}

        transpositionTable.save(pos, 0, bestMove, currentRootScore, depth, currentRootScore >= beta ? LowerBoundScore : ExactScore);
		auto pv = transpositionTable.extractPv(pos);

        // If this is not an infinite search and the search has returned mate scores two times in a row stop searching.
        if (!infinite && isMateScore(currentRootScore) && isMateScore(lastRootScore) && depth > 6)
        {
            searching = false;
        }

        // If we have reached the max depth stop searching. Should not happen during gameplay, only analysis.
        if (depth >= 127 && currentRootScore > previousAlpha && currentRootScore < previousBeta)
        {
            searching = false;
        }

		if (!searching)
        { 
			auto searchTime = sw.elapsed<std::chrono::milliseconds>();
            sync_cout << "info time " << searchTime
                      << " nodes " << nodeCount
                      << " nps " << (nodeCount / (searchTime + 1)) * 1000
                      << " tbhits " << tbHits << std::endl
                      << "bestmove " << algebraicMove(pv[0]) << std::endl;
            break;
		}

        lastRootScore = currentRootScore;

        // The score is outside the aspiration windows, we need to loosen them up.
        if (currentRootScore <= previousAlpha || currentRootScore >= previousBeta)
		{
            auto lowerBound = currentRootScore >= previousBeta;
            if (isMateScore(currentRootScore)) 
            {
                // We apparently found a mate, in this case we remove aspiration windows completely to not miss a faster mate.
                alpha = -infinity;
                beta = infinity;
            }
            else
            {
                // Loosen the window we breached a bit. 
                if (lowerBound)
                {
                    alpha = previousAlpha;
                    beta = previousBeta + delta;
                }
                else
                {
                    alpha = previousAlpha - delta;
                    beta = previousBeta;
                }
                delta *= 2; // Exponential increase to the amount of widening seems best.
            }
            infoPv(pv, depth, currentRootScore, lowerBound ? LowerBoundScore : UpperBoundScore);
			continue;
		}
        infoPv(pv, depth, currentRootScore, ExactScore);

        // Adjust alpha and beta based on the last score.
        // Don't adjust if depth is low - it's a waste of time.
		// Also don't use aspiration windows when searching for faster mate.
        if (depth >= 4 && !isMateScore(currentRootScore))
        {
            alpha = currentRootScore - aspirationWindow;
            beta = currentRootScore + aspirationWindow;
        }
        else
        {
            alpha = -infinity;
            beta = infinity;
        }
        delta = aspirationWindow;
        ++depth;
    }

	sw.stop();
}

std::string Search::algebraicMove(const Move& move)
{
	static std::array<std::string, 64> squareToNotation = {
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	};

	static std::array<std::string, 64> promotionToNotation = {
		"p", "n", "b", "r", "q", "k"
	};

	std::string s;
	auto from = move.getFrom();
	auto to = move.getTo();
	auto promotion = move.getPromotion();

	s += squareToNotation[from] + squareToNotation[to];
	if (promotion != Piece::Empty && promotion != Piece::King && promotion != Piece::Pawn)
	{
		s += promotionToNotation[promotion];
	}

	return s;
}

std::string Search::algebraicMoves(const std::vector<Move>& moves)
{
	std::string s;

	for (auto& move : moves)
	{
		s += algebraicMove(move) + " ";
	}

	return s;
}

void Search::infoCurrMove(const Move& move, int depth, int nr)
{
	sync_cout << "info depth " << depth 
		      << " currmove " << algebraicMove(move) 
			  << " currmovenumber " << nr + 1 << std::endl;
}

void Search::infoPv(const std::vector<Move>& moves, int depth, int score, int flags)
{
	std::stringstream ss;

	ss << "info depth " << depth << " seldepth " << selDepth;
	if (isMateScore(score))
	{
		score = (score > 0 ? ((mateScore - score + 1) >> 1) : ((-score - mateScore) >> 1));
		ss << " score mate " << score;
	}
	else
	{
		ss << " score cp " << score;
	}

	if (flags == LowerBoundScore)
	{
		ss << " lowerbound ";
	}
	else if (flags == UpperBoundScore)
	{
		ss << " upperbound ";
	}

	auto searchTime = sw.elapsed<std::chrono::milliseconds>();
    ss << " time " << searchTime
       << " nodes " << nodeCount
       << " nps " << (nodeCount / (searchTime + 1)) * 1000
       << " tbhits " << tbHits
       << " pv " << algebraicMoves(moves) << std::endl;

	sync_cout << ss.str(); 
}

// Move ordering goes like this:
// 1. Hash move (which can also be the PV-move)
// 2. Good captures and promotions
// 3. Equal captures and promotions
// 4. Killer moves
// 5. Quiet moves sorted by the history heuristic
// 6. Bad captures
void Search::orderMoves(const Position& pos, MoveList& moveList, const Move& ttMove, int ply)
{
    for (auto i = 0; i < moveList.size(); ++i)
    {
        auto& move = moveList[i];

        if (move.getMove() == ttMove.getMove()) // Move from transposition table
        {
            move.setScore(hashMoveScore);
        }
        else if (pos.getBoard(move.getTo()) != Piece::Empty 
             || (move.getPromotion() != Piece::Empty && move.getPromotion() != Piece::King))
        {
            auto score = pos.SEE(move);
            if (score >= 0) // Order good captures and promotions after ttMove
            {
                score += captureMoveScore;
            }
            move.setScore(score);
        }
        else
        {
            auto killerScore = killerTable.isKiller(move, ply);
            if (killerScore > 0)
            {
                move.setScore(killerMoveScore[killerScore]);
            }
            else
            {
                move.setScore(historyTable.getScore(pos, move));
            }
        }
    }
}

void Search::orderCaptures(const Position& pos, MoveList& moveList)
{
    for (auto i = 0; i < moveList.size(); ++i)
    {
        moveList[i].setScore(pos.SEE(moveList[i]));
    }
}

void Search::selectMove(MoveList& moveList, int currentMove)
{
    auto bestMove = currentMove;
    auto bestScore = moveList[currentMove].getScore();

    for (auto i = currentMove + 1; i < moveList.size(); ++i)
    {
        if (moveList[i].getScore() > bestScore)
        {
            bestScore = moveList[i].getScore();
            bestMove = i;
        }
    }

    if (bestMove > currentMove)
    {
        std::swap(moveList[currentMove], moveList[bestMove]);
    }
}

int Search::quiescenceSearch(Position& pos, int ply, int alpha, int beta, bool inCheck)
{
    int bestScore, delta;
    bool zugzwangLikely;
    MoveList moveList;
    History history;

    if (inCheck)
    {
        bestScore = matedInPly(ply);
		delta = -infinity;
        MoveGen::generateLegalEvasions(pos, moveList);
        orderMoves(pos, moveList, Move(0, 0, 0, 0), ply); // TODO: some replacement for constructing a move.
    }
    else
    {
        bestScore = Evaluation::evaluate(pos, zugzwangLikely);
        if (bestScore > alpha)
        {
            if (bestScore >= beta)
            {
                return bestScore;
            }
            alpha = bestScore;
        }
        delta = bestScore + futilityMargins[0];
        MoveGen::generatePseudoLegalCaptureMoves(pos, moveList);
        orderCaptures(pos, moveList);
    }

    for (auto i = 0; i < moveList.size(); ++i)
    {
        selectMove(moveList, i);
        const auto& move = moveList[i];

        if (!inCheck) // don't do any pruning if in check
        {
            // Bad capture pruning. Assumes that the moves are sorted from highest SEE value to lowest.
            if (move.getScore() < 0)
            {
                break;
            }

            // Delta pruning. Same assumption as previous part.
            if (delta + move.getScore() <= alpha)
            {
                bestScore = std::max(bestScore, delta + move.getScore());
                break;
            }
        }

        if (!pos.makeMove(move, history))
        {
            continue;
        }
        ++nodeCount;
        --nodesToTimeCheck;

        auto score = -quiescenceSearch(pos, ply + 1, -beta, -alpha, pos.inCheck());
        pos.unmakeMove(move, history);

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

#ifdef _WIN32
#pragma warning (disable : 4127) // Shuts up warnings about conditional branches always being true/false.
#endif
template <bool pvNode>
int Search::search(Position& pos, int depth, int ply, int alpha, int beta, int allowNullMove, bool inCheck)
{
    assert(alpha < beta);

    auto bestScore = matedInPly(ply), movesSearched = 0, prunedMoves = 0;
    auto ttFlag = UpperBoundScore;
    auto zugzwangLikely = false; // Initialization needed only to shut up warnings.
    MoveList moveList;
    History history;
    Move ttMove(0, 0, 0, 0), bestMove(0, 0, 0, 0);
    int score;

    // Used for sending seldepth info.
    if (ply > selDepth)
    {
        selDepth = ply;
    }

	// Small speed optimization, runs fine without it.
    transpositionTable.prefetch(pos.getHashKey());

    // Time check things.
    if (nodesToTimeCheck <= 0)
    {
        nodesToTimeCheck = 10000;
        if (!infinite) // Can't stop search if ordered to run indefinitely
        {
            // Casting works around a few warnings.
            auto time = static_cast<int64_t>(sw.elapsed<std::chrono::milliseconds>());
            if (time > maxTime) // Hard cutoff for search time, if we don't stop we risk running out of time later.
            {
                searching = false;
            }
            else if (time > targetTime)
            {
                if (currentRootScore == -mateScore) // No score for root -> new iteration -> most likely takes too long to complete -> stop
                {
                    searching = false;
                }
                else if (currentRootScore < lastRootScore) // Score dropping, extend search time up to 5x.
                {
                    if (time > 5 * targetTime) 
                    {
                        searching = false;
                    }
                }
            }
            else
            {
                // TODO: Add easy move here.
            }
        }

        if (!searching)
        {
            throw StopSearchException("allocated time has run out");
        }
    }

    // Check for fifty move draws.
    if (pos.getFiftyMoveDistance() >= 100)
    {
        if (inCheck) 
        {
            MoveGen::generateLegalEvasions(pos, moveList);
            if (moveList.empty())
            {
                return bestScore; // Can't claim draw on fifty move if mated.
            }
        }
        return contempt[pos.getSideToMove()];
    }
     
    // Check for repetition draws. Technically we are checking for 2-fold repetitions instead of 3-fold, but that is enough for game theoric correctness.
    if (repetitionDraw(pos, ply))
    {
        return contempt[pos.getSideToMove()];
    }

    // Mate distance pruning, safe at all types of nodes.
    alpha = std::max(matedInPly(ply), alpha);
    beta = std::min(mateInPly(ply + 1), beta);
    if (alpha >= beta)
        return alpha;

    // If the depth is too low drop into quiescense search.
    if (depth <= 0)
    {
        return quiescenceSearch(pos, ply, alpha, beta, inCheck);
    }

    // Probe the transposition table. 
    auto ttEntry = transpositionTable.probe(pos);
    if (ttEntry)
    {
        ttMove.setMove(ttEntry->getBestMove());
        if (ttEntry->getDepth() >= depth)
        {
            auto ttScore = ttEntry->getScore();
            auto ttFlags = ttEntry->getFlags();

            // Correct mate scores back to normal form.
            if (isMateScore(ttScore))
            {
                ttScore += static_cast<int16_t>(ttScore > 0 ? -ply : ply);
            }

            if (ttFlags == ExactScore || (ttFlags == UpperBoundScore && ttScore <= alpha) || (ttFlags == LowerBoundScore && ttScore >= beta))
                return ttScore;

            if (ttFlags == UpperBoundScore && ttScore < beta)
            {
                beta = ttScore;
            }
            else if (ttFlags == LowerBoundScore && ttScore > alpha)
            {
                alpha = ttScore;
            }
        }
    }

    // Probe the endgame tablebases.
    if (probeTb && pos.getTotalPieceCount() <= syzygyProbeLimit)
    {
        int success;
        score = Syzygy::probeWdl(pos, success);
        if (success)
        {
            ++tbHits;
            if (score < -1) score = matedInPly(ply + 200);
            else if (score > 1) score = mateInPly(ply + 200);
            else score += contempt[pos.getSideToMove()];
            return score;
        }
    }

    // Get the static evaluation of the position. Not needed in nodes where we are in check.
    auto staticEval = (inCheck ? -infinity : Evaluation::evaluate(pos, zugzwangLikely));

    // Reverse futility pruning / static null move pruning.
    // Not useful in PV-nodes as this tries to search for nodes where score >= beta but in PV-nodes score < beta.
    if (!pvNode && !inCheck && !zugzwangLikely && depth <= reverseFutilityDepth && staticEval - reverseFutilityMargins[depth] >= beta)
        return staticEval - reverseFutilityMargins[depth];

    // Razoring.
    // Not useful in PV-nodes as this tries to search for nodes where score <= alpha but in PV-nodes score > alpha.
    if (!pvNode && !inCheck && depth <= razoringDepth && staticEval <= alpha - razoringMargins[depth])
    {
        auto razoringAlpha = alpha - razoringMargins[depth];
        score = quiescenceSearch(pos, ply, razoringAlpha, razoringAlpha + 1, false);
        if (score <= razoringAlpha)
        {
            transpositionTable.save(pos, ply, ttMove, score, depth, UpperBoundScore);
            return score;
        }
    }

    // Null move pruning.
    // Not used when in a PV-node because we should _never_ fail high at a PV-node so doing this is a waste of time.
    if (!pvNode && allowNullMove && !inCheck && !zugzwangLikely)
    {
        if (!(ttEntry 
           && ttEntry->getFlags() == UpperBoundScore 
           && ttEntry->getDepth() >= depth - 1 - nullReduction
           && ttEntry->getScore() <= alpha))
        {
            repetitionHashes[rootPly + ply] = pos.getHashKey();
            pos.makeNullMove(history);
            ++nodeCount;
            --nodesToTimeCheck;
            score = -search<false>(pos, depth - 1 - nullReduction, ply + 1, -beta, -beta + 1, allowNullMove - 2, false);
            pos.unmakeNullMove(history);
            if (score >= beta)
            {
                transpositionTable.save(pos, ply, ttMove, score, depth, LowerBoundScore);
                return score;
            }
        }
    }

    // Internal iterative deepening.
    // Only done at PV-nodes due to the cost involved.
    if (pvNode && ttMove.empty() && depth > 4)
    {
        // We can skip nullmove in IID since if it would have worked we wouldn't be here.
        score = search<true>(pos, depth - 2, ply, alpha, beta, 0, inCheck);

        // Now probe the TT and get the best move.
        auto tte = transpositionTable.probe(pos);
        if (tte)
        {
            ttMove.setMove(tte->getBestMove());
        }
    }

    // Generate moves and order them. In nodes where we are in check we use a special evasion move generator.
    inCheck ? MoveGen::generateLegalEvasions(pos, moveList) : MoveGen::generatePseudoLegalMoves(pos, moveList);
    orderMoves(pos, moveList, ttMove, ply);

    // Futility pruning is useless at PV-nodes for the same reason as razoring.
    auto futileNode = (!pvNode && !inCheck && depth <= futilityDepth && staticEval + futilityMargins[depth] <= alpha);
    auto lmpNode = (!pvNode && !inCheck && depth <= lmpDepth);
    auto lmrNode = (!inCheck && depth >= lmrReductionLimit);
    auto oneReply = (moveList.size() == 1);

    repetitionHashes[rootPly + ply] = pos.getHashKey();
    for (auto i = 0; i < moveList.size(); ++i)
    {
        selectMove(moveList, i);
        const auto& move = moveList[i];
        if (!pos.makeMove(move, history))
        {
            continue;
        }
        ++nodeCount;
        --nodesToTimeCheck;

        auto givesCheck = pos.inCheck();
        auto extension = (givesCheck || oneReply) ? 1 : 0;
        auto newDepth = depth - 1 + extension;
        auto nonCriticalMove = !extension && move.getScore() >= 0 && move.getScore() < killerMoveScore[4];

        // Futility pruning and late move pruning / move count based pruning.
        if (nonCriticalMove && bestScore > -maxMateScore && (futileNode || (lmpNode && i >= lmpMoveCount[depth])))
        {
            pos.unmakeMove(move, history);
            ++prunedMoves;
            continue;
        }

        if (!movesSearched)
        {
            score = -search<pvNode>(pos, newDepth, ply + 1, -beta, -alpha, 2, givesCheck);
        }
        else
        {
            auto reduction = ((lmrNode && i >= lmrFullDepthMoves && nonCriticalMove)
                           ? lmrReductions[i - lmrFullDepthMoves] : 0);

            score = -search<false>(pos, newDepth - reduction, ply + 1, -alpha - 1, -alpha, 2, givesCheck);

            // If a reduced search doesn't fail low in a PV-node we first open the window and search with a reduced depth.
            // If that doesn't fail low either only then do we stop reducing the move.
            // Test the other idea sometime again.
            // In non-PV-nodes if a reduced move doesn't fail low all we can do is remove the reduction.
            if (pvNode && score > alpha)
            {
                score = -search<true>(pos, newDepth - reduction, ply + 1, -beta, -alpha, 2, givesCheck);
                if (reduction && score > alpha)
                {
                    score = -search<true>(pos, newDepth, ply + 1, -beta, -alpha, 2, givesCheck);
                }
            }
            else if (reduction && score > alpha) 
            {
                score = -search<false>(pos, newDepth, ply + 1, -alpha - 1, -alpha, 2, givesCheck);
            }
        }
        pos.unmakeMove(move, history);
        ++movesSearched;

        if (score > bestScore)
        {
            if (score > alpha)
            {
                if (score >= beta)
                {
                    transpositionTable.save(pos, ply, move, score, depth, LowerBoundScore);

                    if (!inCheck)
                    {
                        if (pos.getBoard(move.getTo()) == Piece::Empty && (move.getPromotion() == Piece::Empty || move.getPromotion() == Piece::King))
                        {
                            historyTable.addCutoff(pos, move, depth);
                            killerTable.addKiller(move, ply);
                        }
                        for (auto j = 0; j < i; ++j)
                        {
                            const auto& move2 = moveList[j];
                            if (pos.getBoard(move2.getTo()) == Piece::Empty && (move2.getPromotion() == Piece::Empty || move2.getPromotion() == Piece::King))
                            {
                                historyTable.addNotCutoff(pos, move2, depth);
                            }
                        }
                    }

                    return score;
                }
                bestMove = move;
                alpha = score;
                ttFlag = ExactScore;
            }
            bestScore = score;
        }
    }

    if (!movesSearched)
    {
        if (!prunedMoves)
        {
            return (inCheck ? bestScore : contempt[pos.getSideToMove()]);
        }
        return staticEval; // Looks like we pruned all moves away. Return some approximation of the score. Just alpha is fine too.
    }

    transpositionTable.save(pos, ply, bestMove, bestScore, depth, ttFlag);

    return bestScore;
}
