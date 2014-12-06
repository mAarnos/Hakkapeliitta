#include "search.hpp"
#include <iostream>
#include <cmath>
#include <sstream>
#include "eval.hpp"
#include "movegen.hpp"
#include "utils\synchronized_ostream.hpp"
#include "utils\exception.hpp"

TranspositionTable Search::transpositionTable;
HistoryTable Search::historyTable;
KillerTable Search::killerTable;

int Search::contemptValue;
std::array<int, 2> Search::contempt;
std::array<HashKey, 128> Search::repetitionHashes;
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
    0, 300, 300, 300
};

const int16_t Search::hashMoveScore = 32767;
const int16_t Search::captureMoveScore = 30767; // hashMoveScore - 2000
const std::array<int16_t, 1 + 4> Search::killerMoveScore = {
    0, 28767, 28766, 28765, 28764 // not used, captureMoveScore - 2000, captureMoveScore - 2001, etc. 
};

int Search::tbHits;
int Search::nodeCount;
int Search::nodesToTimeCheck;
Stopwatch Search::sw;

void Search::initialize()
{
    contemptValue = 0;
    contempt.fill(0);
    searching = false;
    infinite = false;
    pondering = false;
    targetTime = maxTime = 0;
    nodeCount = 0;
	tbHits = 0;
    nodesToTimeCheck = 10000;

    for (auto i = 0; i < 256; ++i)
    {
        lmrReductions[i] = static_cast<int>(std::max(1.0, std::round(std::log(i + 1))));
    }
}

bool Search::repetitionDraw(const Position& pos, int ply)
{
    auto temp = std::max(ply - pos.getFiftyMoveDistance(), 0);

    for (auto i = ply - 2; ply >= temp; ply -= 2)
    {
        if (repetitionHashes[i] == pos.getHashKey())
        {
            return true;
        }
    }

    return false;
}

void Search::think(Position& pos)
{
	auto alpha = -infinity;
	auto beta = infinity;
	auto delta = aspirationWindow;
	auto inCheck = pos.inCheck();
	MoveList rootMoveList;
	History history;
	Move bestMove(0, 0, 0, 0);

	tbHits = 0;
    nodeCount = 0;
    nodesToTimeCheck = 10000;
    contempt[pos.getSideToMove()] = -contemptValue;
    contempt[!pos.getSideToMove()] = contemptValue;
    historyTable.age();
    killerTable.clear();
	sw.reset();
	sw.start();

	inCheck ? MoveGen::generateLegalEvasions(pos, rootMoveList) 
		    : MoveGen::generatePseudoLegalMoves(pos, rootMoveList);
	// Remove all illegal moves.
	auto marker = 0;
	for (auto i = 0; i < rootMoveList.size(); ++i)
	{
		if (!pos.makeMove(rootMoveList[i], history))
		{
			continue;
		}
		rootMoveList[marker++] = rootMoveList[i];
		pos.unmakeMove(rootMoveList[i], history);
	}

    for (auto depth = 1;;)
    {
		auto movesSearched = 0;
		auto bestScore = matedInPly(0), score = matedInPly(0);
		auto lmrNode = (!inCheck && depth >= lmrReductionLimit);

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
#ifdef INFO_CURR_MOVE
				if (depth >= 10)
				{
					infoCurrMove(move, depth, i);
				}
#endif
				++nodeCount;

				auto givesCheck = pos.inCheck();
				auto extension = givesCheck ? 1 : 0;
				auto newDepth = depth - 1 + extension;
				auto nonCriticalMove = !extension && move.getScore() >= 0 && move.getScore() < killerMoveScore[4];

				if (!movesSearched)
				{
					score = (newDepth > 0 ? -search<true>(pos, newDepth, 1, -beta, -alpha, 2, givesCheck)
						                  : -quiescenceSearch(pos, 1, -beta, -alpha, givesCheck));
				}
				else
				{
					auto reduction = ((lmrNode && movesSearched >= lmrFullDepthMoves && nonCriticalMove)
						? lmrReductions[movesSearched - lmrFullDepthMoves] : 0);

					score = (newDepth - reduction > 0 ? -search<false>(pos, newDepth - reduction, 1, -alpha - 1, -alpha, 2, givesCheck)
						                              : -quiescenceSearch(pos, 1, -alpha - 1, -alpha, givesCheck));

					if (reduction && score > alpha) // Research in case reduced move is above alpha.
					{
						score = (newDepth > 0 ? -search<false>(pos, newDepth, 1, -alpha - 1, -alpha, 2, givesCheck)
							                  : -quiescenceSearch(pos, 1, -alpha - 1, -alpha, givesCheck));
					}

					if (score > alpha && score < beta) // Full-window research in case a new pv is found. Can only happen in PV-nodes.
					{
						score = (newDepth > 0 ? -search<true>(pos, newDepth, 1, -beta, -alpha, 2, givesCheck)
							                  : -quiescenceSearch(pos, 1, -beta, -alpha, givesCheck));
					}
				}
				pos.unmakeMove(move, history);
				++movesSearched;

				if (score > bestScore)
				{
					bestScore = score;
					if (score > alpha)
					{
						bestMove = move;
						alpha = score;
						if (score >= beta)
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
							break;
						}
						transpositionTable.save(pos, 0, bestMove, bestScore, depth, UpperBoundScore);
						auto pv = transpositionTable.extractPv(pos);
						infoPv(pv, depth, bestScore, ExactScore);
					}
				}
			}
			transpositionTable.save(pos, 0, bestMove, bestScore, depth, bestScore >= beta ? LowerBoundScore : ExactScore);
		}
		catch (const HakkapeliittaException&)
		{
		}

		auto pv = transpositionTable.extractPv(pos);
		assert(pv.size() >= 1);

		if (!searching)
		{
			auto searchTime = sw.elapsed<std::chrono::milliseconds>();
			sync_cout << "info time " << searchTime
				      << " nodes " << nodeCount 
					  << " nps " << (nodeCount / searchTime + 1) * 1000
					  << " tbhits " << tbHits << std::endl 
					  << " bestmove " << algebraicMove(pv[0]);
			break;
		}

		if (bestScore <= alpha)
		{
			alpha -= delta;
			delta *= 2;
			infoPv(pv, depth, bestScore, UpperBoundScore);
			continue;
		}
		if (bestScore >= beta)
		{
			beta += delta;
			delta *= 2;
			infoPv(pv, depth, bestScore, LowerBoundScore);
			continue;
		}
		infoPv(pv, depth, bestScore, ExactScore);

        // Adjust alpha and beta based on the last score.
        // Don't adjust if depth is low - it's a waste of time.
		// Also reset the window completely if a mate is found so that we don't miss shorter mates.
        if (depth >= 4 && !isMateScore(bestScore))
        {
			alpha = bestScore - aspirationWindow;
			beta = bestScore + aspirationWindow;
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

	ss << "info depth " << depth;
	if (isMateScore(score))
	{
		score = (score > 0 ? ((mateInPly(score + 1)) >> 1) : ((-score - mateScore) >> 1));
		ss << " score mate" << score;
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
	   << " nps " << (nodeCount / searchTime + 1) * 1000
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

    // Maybe add checking for draw here?

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
            // Bad capture pruning + delta pruning. Assumes that the moves are sorted from highest SEE value to lowest.
            if (move.getScore() < 0 || (delta + move.getScore() < alpha))
            {
                break;
            }
        }

        if (!pos.makeMove(move, history))
        {
            continue;
        }

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
#pragma warning (disable : 4127) // shuts up warnings about conditional branches always being true/false
#endif
template <bool pvNode>
int Search::search(Position& pos, int depth, int ply, int alpha, int beta, int allowNullMove, bool inCheck)
{
    assert(alpha < beta);

    auto bestScore = matedInPly(ply), movesSearched = 0, prunedMoves = 0;
    MoveList moveList;
    History history;
    Move ttMove, bestMove(0, 0, 0, 0);
    TTFlags ttFlag = UpperBoundScore;
    int score;
    auto zugzwangLikely = false;

	// Small speed optimization, runs fine without it.
    transpositionTable.prefetch(pos.getHashKey());

    if (--nodesToTimeCheck <= 0)
    {
        nodesToTimeCheck = 10000;
        // FIXME! Check for timeout here.
        if (!searching)
        {
            throw HakkapeliittaException("Search: stop");
        }
    }

    if (pos.getFiftyMoveDistance() >= 100)
    {
        if (inCheck) 
        {
            MoveGen::generateLegalEvasions(pos, moveList);
            if (moveList.empty())
            {
                return matedInPly(ply); // Can't claim draw on fifty move if mated.
            }
        }
        return contempt[pos.getSideToMove()];
    }

    if (repetitionDraw(pos, ply))
    {
        return contempt[pos.getSideToMove()];
    }

    // Mate distance pruning, safe at all types of nodes.
    alpha = std::max(matedInPly(ply), alpha);
    beta = std::min(mateInPly(ply - 1), beta);
    if (alpha >= beta)
        return alpha;

    // Transposition table probe.
    if (transpositionTable.probe(pos, ply, ttMove, score, depth, alpha, beta))
    {
        return score;
    }

    // Probe tablebases here.

    // Get the static evaluation of the position. Not needed in nodes where we are in check.
    auto staticEval = (inCheck ? -infinity : Evaluation::evaluate(pos, zugzwangLikely));

    // Reverse futility pruning / static null move pruning.
    if (!pvNode && !inCheck && !zugzwangLikely && depth <= reverseFutilityDepth && staticEval - reverseFutilityMargins[depth] >= beta)
        return staticEval - reverseFutilityMargins[depth];

    // Razoring.
    if (!pvNode && !inCheck && depth <= razoringDepth && staticEval <= alpha - razoringMargins[depth])
    {
        auto razoringAlpha = alpha - razoringMargins[depth];
        score = quiescenceSearch(pos, ply, razoringAlpha, razoringAlpha + 1, false);
        if (score <= razoringAlpha)
            return score;
    }

    // Double null move pruning.
    // Not used when in a PV-node because we should _never_ fail high at a PV-node so doing this is a waste of time.
    if (!pvNode && allowNullMove && !inCheck && !zugzwangLikely)
    {
        pos.makeNullMove(history);
        ++nodeCount;
        score = (depth - 1 - nullReduction <= 0 ? -quiescenceSearch(pos, ply + 1, alpha, beta, false)
                                                : -search<false>(pos, depth - 1 - nullReduction, ply + 1, -beta, -beta + 1, allowNullMove - 1, false));
        pos.unmakeNullMove(history);
        if (score >= beta)
        {
            transpositionTable.save(pos, ply, ttMove, score, depth, LowerBoundScore);
            return score;
        }
    }

    // Internal iterative deepening.
    // Only done at PV-nodes due to the cost involved.
    if (pvNode && ttMove.empty() && depth > 4)
    {
        // We can skip nullmove in IID since if it would have worked we wouldn't be here.
        score = search<true>(pos, depth - 2, ply, alpha, beta, 0, inCheck);
        transpositionTable.probe(pos, ply, ttMove, score, depth, alpha, beta);
    }

    // Generate moves and order them. In nodes where we are in check we use a special evasion move generator.
    inCheck ? MoveGen::generateLegalEvasions(pos, moveList) : MoveGen::generatePseudoLegalMoves(pos, moveList);
    orderMoves(pos, moveList, ttMove, ply);

    // Set flags for certain kinds of nodes.
    auto futileNode = (!inCheck && depth <= futilityDepth && staticEval + futilityMargins[depth] <= alpha);
    auto lmpNode = (!pvNode && !inCheck && depth <= lmpDepth);
    auto lmrNode = (!inCheck && depth >= lmrReductionLimit);
    auto oneReply = (moveList.size() == 1);

    for (auto i = 0; i < moveList.size(); ++i)
    {
        selectMove(moveList, i);
        const auto& move = moveList[i];
        if (!pos.makeMove(move, history))
        {
            continue;
        }
        ++nodeCount;

        auto givesCheck = pos.inCheck();
        auto extension = (givesCheck || oneReply) ? 1 : 0;
        auto newDepth = depth - 1 + extension;
        auto nonCriticalMove = !extension && move.getScore() >= 0 && move.getScore() < killerMoveScore[4];

        // Futility pruning and late move pruning / move count based pruning.
        if (nonCriticalMove && (futileNode || (lmpNode && movesSearched >= lmpMoveCount[depth])))
        {
            pos.unmakeMove(move, history);
            ++prunedMoves;
            continue;
        }

        if (!movesSearched)
        {
            score = (newDepth > 0 ? -search<pvNode>(pos, newDepth, ply + 1, -beta, -alpha, 2, givesCheck)
                                  : -quiescenceSearch(pos, ply + 1, -beta, -alpha, givesCheck));
        }
        else
        {
            auto reduction = ((lmrNode && movesSearched >= lmrFullDepthMoves && nonCriticalMove)
                           ? lmrReductions[movesSearched - lmrFullDepthMoves] : 0);

            score = (newDepth - reduction > 0 ? -search<false>(pos, newDepth - reduction, ply + 1, -alpha - 1, -alpha, 2, givesCheck) 
                                              : -quiescenceSearch(pos, ply + 1, -alpha - 1, -alpha, givesCheck));

            if (reduction && score > alpha) // Research in case reduced move is above alpha.
            {
                score = (newDepth > 0 ? -search<false>(pos, newDepth, ply + 1, -alpha - 1, -alpha, 2, givesCheck)
                                      : -quiescenceSearch(pos, ply + 1, -alpha - 1, -alpha, givesCheck));
            }

            if (score > alpha && score < beta) // Full-window research in case a new pv is found. Can only happen in PV-nodes.
            {
                score = (newDepth > 0 ? -search<pvNode>(pos, newDepth, ply + 1, -beta, -alpha, 2, givesCheck)
                                      : -quiescenceSearch(pos, ply + 1, -beta, -alpha, givesCheck));
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
            return (inCheck ? matedInPly(ply) : contempt[pos.getSideToMove()]);
        }
        return staticEval; // Looks like we pruned all moves away. Return some approximation of the score. Just alpha is fine too.
    }

    transpositionTable.save(pos, ply, bestMove, bestScore, depth, ttFlag);

    return bestScore;
}
