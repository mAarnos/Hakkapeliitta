#include "search.hpp"
#include <iostream>
#include <cmath>
#include "eval.hpp"
#include "movegen.hpp"

TranspositionTable Search::transpositionTable;
HistoryTable Search::historyTable;
KillerTable Search::killerTable;

int Search::contemptValue;
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
    0, 300, 300, 300
};

const int16_t Search::hashMoveScore = 32767;
const int16_t Search::captureMoveScore = 30767; // hashMoveScore - 2000
const std::array<int16_t, 1 + 4> Search::killerMoveScore = {
    0, 28767, 28766, 28765, 28764 // not used, captureMoveScore - 2000, captureMoveScore - 2001, etc. 
};

std::array<Move, 32> Search::pv[32];
std::array<int, 32> Search::pvLength;

void Search::initialize()
{
    contemptValue = 0;
    contempt.fill(0);
    searching = false;
    infinite = false;
    pondering = false;
    targetTime = maxTime = 0;

    for (auto i = 0; i < 256; ++i)
    {
        lmrReductions[i] = static_cast<int>(std::max(1.0, std::round(std::log(i + 1))));
    }
}

// Move ordering goes like this:
// 1. Hash move (which can also be the PV-move)
// 2. Good captures and promotions
// 3. Equal captures and promotions
// 4. Killer moves
// 5. Quiet moves sorted by the history heuristic
// 6. Bad captures
void Search::orderMoves(const Position & pos, MoveList & moveList, const Move & ttMove, int ply)
{
    for (auto i = 0; i < moveList.size(); ++i)
    {
        auto & move = moveList[i];
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

void Search::orderCaptures(const Position & pos, MoveList & moveList)
{
    for (auto i = 0; i < moveList.size(); ++i)
    {
        moveList[i].setScore(pos.SEE(moveList[i]));
    }
}

void Search::selectMove(MoveList & moveList, int currentMove)
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

int Search::quiescenceSearch(Position & pos, int ply, int alpha, int beta, bool inCheck)
{
    int bestScore, delta;
    MoveList moveList;
    History history;

    // delete this
    pvLength[ply] = ply;

    if (inCheck)
    {
        bestScore = matedInPly(ply);
        MoveGen::generateLegalEvasions(pos, moveList);
        orderMoves(pos, moveList, Move(0, 0, 0, 0), ply); // TODO: some replacement for constructing a move.
        delta = -infinity;
    }
    else
    {
        bestScore = Evaluation::evaluate(pos);
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
        const auto & move = moveList[i];

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

                // delete this
                pv[ply][ply] = move;
                for (auto i = ply + 1; i < pvLength[ply + 1]; ++i)
                {
                    pv[ply][i] = pv[ply + 1][i];
                }
                pvLength[ply] = pvLength[ply + 1];
            }
            bestScore = score;
        }
    }

    return bestScore;
}

int Search::search(Position & pos, int depth, int ply, int alpha, int beta, int allowNullMove, bool inCheck)
{
    assert(alpha < beta);

    auto pvNode = ((alpha + 1) != beta);
    auto bestScore = matedInPly(ply), movesSearched = 0, prunedMoves = 0;
    MoveList moveList;
    Move ttMove;
    History history;
    int score;
    bool futileNode, lmpNode, oneReply;

    transpositionTable.prefetch(pos.getHashKey());

    // Check time and input here

    // Check for repetition draws here

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
    auto staticEval = (inCheck ? -infinity : Evaluation::evaluate(pos));

    // Reverse futility pruning / static null move pruning.
    // Not used when in a pawn endgame - TODO: test
    // Also not used when in a PV-node - TODO: test that too, no reason why it shouldn't work there as well.
    if (!pvNode && !inCheck && pos.getGamePhase() != 64
        && depth <= reverseFutilityDepth && staticEval - reverseFutilityMargins[depth] >= beta)
        return staticEval - reverseFutilityMargins[depth];

    // Double null move pruning.
    // Not used when in a PV-node because we should _never_ fail high at a PV-node so doing this is a waste of time.
    if (!pvNode && allowNullMove && !inCheck && pos.getGamePhase() != 64)
    {
        pos.makeNullMove(history);
        // add ++nodeCount here
        if (depth <= 4)
        {
            score = -quiescenceSearch(pos, ply + 1, alpha, beta, false);
        }
        else
        {
            score = -search(pos, depth - 1 - nullReduction, ply + 1, -beta, -beta + 1, allowNullMove - 1, false);
        }
        pos.unmakeNullMove(history);

        if (score >= beta)
        {
            transpositionTable.save(pos, ply, ttMove, score, depth, 2);
            return score;
        }
    }

    // Razoring.
    if (!inCheck && depth <= razoringDepth && staticEval <= alpha - razoringMargins[depth])
    {
        auto razoringAlpha = alpha - razoringMargins[depth];
        score = quiescenceSearch(pos, ply, razoringAlpha, razoringAlpha + 1, false);
        if (score <= razoringAlpha)
            return score;
    }

    // Internal iterative deepening.
    // Only done at PV-nodes due to the cost involved.
    if (pvNode && ttMove.empty() && depth > 2)
    {
        // We can skip nullmove in IID since if it would have worked we wouldn't be here.
        score = search(pos, depth - 2, ply, alpha, beta, 0, inCheck);
        if (score <= alpha) // Research in case of a fail low.
        {
            score = search(pos, depth - 2, ply, -infinity, beta, 0, inCheck);
        }
        transpositionTable.probe(pos, ply, ttMove, score, depth, alpha, beta);
    }
    
    // Set flags for certain kinds of nodes.
    futileNode = (!inCheck && depth <= futilityDepth && staticEval + futilityMargins[depth] <= alpha);
    lmpNode = (!pvNode && !inCheck && depth <= lmpDepth);

    // Generate moves and order them. In nodes where we are in check we use a special evasion move generator.
    inCheck ? MoveGen::generateLegalEvasions(pos, moveList) : MoveGen::generatePseudoLegalMoves(pos, moveList);
    orderMoves(pos, moveList, ttMove, ply);
    // If we have only one move available set a flag.
    oneReply = (moveList.size() == 1);

    for (auto i = 0; i < moveList.size(); ++i)
    {
        selectMove(moveList, i);
        const auto & move = moveList[i];
        if (!pos.makeMove(move, history))
        {
            continue;
        }
        // add ++nodeCount here

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
            score = -search(pos, newDepth, ply + 1, -beta, -alpha, 2, givesCheck);
        }
        else
        {
            auto reduction = ((!inCheck && movesSearched >= lmrFullDepthMoves && depth >= lmrReductionLimit && nonCriticalMove)
                           ? lmrReductions[movesSearched - lmrFullDepthMoves] : 0);

            score = -search(pos, newDepth - reduction, ply + 1, -alpha - 1, -alpha, 2, givesCheck);

            if (reduction && score > alpha) // Research in case reduced move is above alpha(which shouldn't have happened).
            {
                score = -search(pos, newDepth, ply + 1, -alpha - 1, -alpha, 2, givesCheck);
            }

            if (score > alpha && score < beta) // Full-window research in case a new pv is found. Can only happen in PV-nodes.
            {
                score = -search(pos, newDepth, ply + 1, -beta, -alpha, 2, givesCheck);
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
                    transpositionTable.save(pos, ply, move, score, depth, 2);
                    // FIX ME! only allow for quiet moves
                    /*
                    historyTable.addCutoff(pos, move, depth);
                    for (auto j = 0; j < i; ++j)
                    {
                        historyTable.addNotCutoff(pos, moveList[i], depth);
                    }
                    */
                    return score;
                }
                alpha = score;
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

    // FIX ME! add best move and flags for the move.
    // transpositionTable.save(pos, ply, bestMove, bestScore, depth, 0);

    return bestScore;
}



