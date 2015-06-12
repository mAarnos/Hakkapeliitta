/*
    Hakkapeliitta - A UCI chess engine. Copyright (C) 2013-2015 Mikko Aarnos.

    Hakkapeliitta is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Hakkapeliitta is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Hakkapeliitta. If not, see <http://www.gnu.org/licenses/>.
*/

#include "search.hpp"
#include "movelist.hpp"
#include "movegen.hpp"
#include "movesort.hpp"

// Returns a score corresponding to getting mated in a given number of plies.
int matedInPly(int ply)
{
    return (-mateScore + ply);
}

// Returns a score corresponding to a mate in a given number of plies.
int mateInPly(int ply)
{
    return (mateScore - ply);
}

// Returns true if a given score is "mate in X".
bool isWinScore(int score)
{
    return score >= minMateScore;
}

// Returns true if a given score is "mated in X".
bool isLoseScore(int score)
{
    return score <= -minMateScore;
}

// Returns true if the score is some kind of a mate score.
bool isMateScore(int score)
{
    return isWinScore(score) || isLoseScore(score);
}

// TT-scores are adjusted to avoid some well-known problems. This adjusts a score back to normal.
int ttScoreToRealScore(int score, int ply)
{
    if (isLoseScore(score))
    {
        score += ply;
    }
    else if (isWinScore(score))
    {
        score -= ply;
    }

    return score;
}

// Converse of the previous function.
int realScoreToTtScore(int score, int ply)
{
    if (isLoseScore(score))
    {
        score -= ply;
    }
    else if (isWinScore(score))
    {
        score += ply;
    }

    return score;
}

// Used for ordering moves during the quiescence search.
// Delete as soon as MoveSort works everywhere.
void Search::orderCaptures(const Position& pos, MoveList& moveList, const Move& ttMove) const
{
    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto move = moveList.getMove(i);

        if (move == ttMove)
        {
            moveList.setScore(i, hashMoveScore);
        }
        else if (pos.captureOrPromotion(move))
        {
            moveList.setScore(i, pos.mvvLva(move) + captureMoveScore);
        }
        else
        {
            moveList.setScore(i, historyTable.getScore(pos, move));
        }
    }
}

// Select the best move from a move list with selection sort.
// Delete as soon as MoveSort works everywhere.
Move selectMove(MoveList& moveList, int currentMove)
{
    auto bestMove = currentMove;
    auto bestScore = moveList.getScore(currentMove);

    for (auto i = currentMove + 1; i < moveList.size(); ++i)
    {
        if (moveList.getScore(i) > bestScore)
        {
            bestScore = moveList.getScore(i);
            bestMove = i;
        }
    }

    if (bestMove > currentMove)
    {
        // Swap the values at bestLocation and currentMove.
        const auto m = moveList.getMove(currentMove);
        const auto s = moveList.getScore(currentMove);
        moveList.setMove(currentMove, moveList.getMove(bestMove));
        moveList.setScore(currentMove, moveList.getScore(bestMove));
        moveList.setMove(bestMove, m);
        moveList.setScore(bestMove, s);
    }

    return moveList.getMove(currentMove);
}

// Calculates the razoring margin for a given depth.
int razoringMargin(int depth)
{
    return 50 * depth + 50;
}

// Calculates the reverse futility margin for a given depth.
int reverseFutilityMargin(int depth)
{
    return 50 * depth + 100;
}

// Calculates the futility margin for a given depth.
int futilityMargin(int depth)
{
    return 25 * depth + 100;
}

Search::Search():
    searchNeedsMoreTime(false), nodesToTimeCheck(10000), nextSendInfo(1000), 
    targetTime(1000), maxTime(10000), maxNodes(std::numeric_limits<size_t>::max()),
    tbHits(0), nodeCount(0), selDepth(0), rootPly(0), repetitionHashes({}), contempt({})
{
    for (auto i = 0; i < 64; ++i)
    {
        for (auto j = 0; j < 64; ++j)
        {
            lmrReductions[i][j] = static_cast<int>(std::max(1.0, (std::log1p(i) * std::log1p(j)) / 1.70));
        }
    }

    for (auto d = 0; d < 1 + lmpDepth; ++d)
    {
        lmpMoveCounts[d] = static_cast<int>(std::round(2.98484 + std::pow(d, 1.74716)));
    }
}

bool Search::repetitionDraw(const Position& pos, int ply) const
{
    const auto limit = std::max(rootPly + ply - pos.getFiftyMoveDistance(), 0);

    for (auto i = rootPly + ply - 2; i >= limit; i -= 2)
    {
        if (repetitionHashes[i] == pos.getHashKey())
        {
            return true;
        }
    }

    return false;
}

int Search::quiescenceSearch(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss)
{
    assert(alpha < beta);
    assert(depth <= 0);
    assert(inCheck == pos.inCheck());

    search<true>(pos, depth, alpha, beta, inCheck, ss);

    int bestScore, delta;
    MoveList moveList;
    Move bestMove;
    auto ttFlag = TranspositionTable::Flags::UpperBoundScore;

    // Small speed optimization, runs fine without it.
    transpositionTable.prefetch(pos.getHashKey());

    // Don't go over max ply.
    if (ss->ply >= 128)
    {
        return evaluation.evaluate(pos);
    }

    // Check for fifty move draws.
    if (pos.getFiftyMoveDistance() >= 100)
    {
        if (inCheck)
        {
            MoveGen::generateLegalEvasions(pos, moveList);
            if (moveList.empty())
            {
                return matedInPly(ss->ply); // Can't claim draw on fifty move if mated.
            }
        }
        return contempt[pos.getSideToMove()];
    }

    // Check for repetition draws. 
    if (repetitionDraw(pos, ss->ply))
    {
        return contempt[pos.getSideToMove()];
    }

    // Mate distance pruning, safe at all types of nodes.
    alpha = std::max(matedInPly(ss->ply), alpha);
    beta = std::min(mateInPly(ss->ply + 1), beta);
    if (alpha >= beta)
        return alpha;

    // We use only two depths when saving info to the TT, one for when we search captures+checks and one for when we search just captures.
    // Since when we are in check we search all moves regardless of depth it goes to the first category as well.
    // It seems that when this part was broken then not pruning checks below didn't work either for some reason.
    const auto ttDepth = (inCheck || depth >= 0) ? 0 : -1;

    const auto ttEntry = transpositionTable.probe(pos.getHashKey());
    if (ttEntry)
    {
        bestMove = ttEntry->getBestMove();
        if (ttEntry->getDepth() >= ttDepth)
        {
            const auto ttScore = ttScoreToRealScore(ttEntry->getScore(), ss->ply);
            const auto ttFlags = ttEntry->getFlags();
            if (ttFlags == TranspositionTable::Flags::ExactScore
            || (ttFlags == TranspositionTable::Flags::UpperBoundScore && ttScore <= alpha)
            || (ttFlags == TranspositionTable::Flags::LowerBoundScore && ttScore >= beta))
            {
                return ttScore;
            }
        }
    }

    if (inCheck)
    {
        bestScore = matedInPly(ss->ply);
        delta = -infinity;
        MoveGen::generateLegalEvasions(pos, moveList);
        if (moveList.empty())
        {
            return bestScore;
        }
    }
    else
    {
        bestScore = evaluation.evaluate(pos);
        if (bestScore > alpha)
        {
            if (bestScore >= beta)
            {
                return bestScore;
            }
            alpha = bestScore;
        }
        delta = bestScore + deltaPruningMargin;
        depth >= 0 ? MoveGen::generatePseudoLegalCapturesAndQuietChecks(pos, moveList) 
                   : MoveGen::generatePseudoLegalCaptures(pos, moveList, false);
    }

    orderCaptures(pos, moveList, bestMove);
    repetitionHashes[rootPly + ss->ply] = pos.getHashKey();
    for (auto i = 0; i < moveList.size(); ++i)
    {
        selectMove(moveList, i);
        const auto move = moveList.getMove(i);
        const auto givesCheck = pos.givesCheck(move);
        ++nodeCount;
        --nodesToTimeCheck;

        // Only prune moves in quiescence search if we are not in check.
        if (!inCheck)
        {
            const auto seeScore = pos.SEE(move);

            // SEE pruning. If the move seems to lose material prune it.
            // Since the SEE score is meaningless for discovered checks we don't prune them.
            if (seeScore < 0 && givesCheck != 2)
            {
                continue;
            }

            // Delta pruning. If the move seems to have no chance of raising alpha prune it.
            // Pruning checks here is too dangerous.
            if (delta + seeScore <= alpha && !givesCheck)
            {
                bestScore = std::max(bestScore, delta + seeScore);
                continue;
            }
        }

        if (!pos.legal(move, inCheck))
        {
            continue;
        }

        Position newPosition(pos);
        newPosition.makeMove(move);
        const auto score = -quiescenceSearch(newPosition, depth - 1, -beta, -alpha, givesCheck != 0, ss + 1);

        if (score > bestScore)
        {
            if (score > alpha)
            {
                if (score >= beta)
                {
                    transpositionTable.save(pos.getHashKey(), 
                                            move, 
                                            realScoreToTtScore(score, ss->ply), 
                                            ttDepth, 
                                            TranspositionTable::Flags::LowerBoundScore);
                    return score;
                }
                bestMove = move;
                ttFlag = TranspositionTable::Flags::ExactScore;
                alpha = score;
            }
            bestScore = score;
        }
    }

    transpositionTable.save(pos.getHashKey(), bestMove, realScoreToTtScore(bestScore, ss->ply), ttDepth, ttFlag);

    return bestScore;
}

#ifdef _WIN32
#pragma warning (disable : 4127) // Shuts up warnings about conditional branches always being true/false.
#endif

template <bool pvNode>
int Search::search(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss)
{
    assert(alpha < beta);
    assert(depth > 0);
    assert(inCheck == pos.inCheck());

    auto bestScore = matedInPly(ss->ply), movesSearched = 0, prunedMoves = 0;
    auto ttFlag = TranspositionTable::Flags::UpperBoundScore;
    MoveList quietsSearched;
    Move bestMove, ttMove;
    int score;

    // Small speed optimization, runs fine without it.
    transpositionTable.prefetch(pos.getHashKey());

    // Used for sending seldepth info.
    if (ss->ply > selDepth)
    {
        selDepth = ss->ply;
    }

    // Don't go over max depth.
    if (ss->ply >= 128)
    {
        return evaluation.evaluate(pos);
    }

    // Time check things.
    // TODO: finish this
    /*
    if (nodesToTimeCheck <= 0)
    {
        nodesToTimeCheck = 10000;
        const auto time = static_cast<int64_t>(sw.elapsed<std::chrono::milliseconds>());

        // Check if we have gone over the node limit.
        if (nodeCount >= maxNodes)
        {
            searching = false;
        }

        if (!infinite && !pondering) // Can't stop search if ordered to run indefinitely
        {
            // First check hard cutoff, then check soft cutoff which depends on the current search situation.
            if (time > maxTime || time > (searchNeedsMoreTime ? 5 * targetTime : targetTime))
            {
                searching = false;
            }
            else
            {
                // TODO: Add easy move here.
            }
        }

        if (!searching)
        {
            // TODO: custom exception
            throw std::exception("allocated time has run out");
        }

        if (time >= nextSendInfo)
        {
            nextSendInfo += 1000;
            
            sync_cout << "info nodes " << nodeCount
                      << " time " << time
                      << " nps " << (nodeCount / (time + 1)) * 1000
                      << " tbhits " << tbHits << std::endl;
        }
    }
    */

    // Check for fifty move draws.
    if (pos.getFiftyMoveDistance() >= 100)
    {
        if (inCheck)
        {
            // Might as well use quietsSearched at this point, we are returning anyways.
            MoveGen::generateLegalEvasions(pos, quietsSearched);
            if (quietsSearched.empty())
            {
                return bestScore; // Can't claim draw on fifty move if mated.
            }
        }
        return contempt[pos.getSideToMove()];
    }

    // Check for repetition draws. Technically we are checking for 2-fold repetitions instead of 3-fold, but that is enough for game theoric correctness.
    if (repetitionDraw(pos, ss->ply))
    {
        return contempt[pos.getSideToMove()];
    }

    // Mate distance pruning, safe at all types of nodes.
    alpha = std::max(matedInPly(ss->ply), alpha);
    beta = std::min(mateInPly(ss->ply + 1), beta);
    if (alpha >= beta)
        return alpha;

    // Probe the transposition table. 
    const auto ttEntry = transpositionTable.probe(pos.getHashKey());
    if (ttEntry)
    {
        ttMove = ttEntry->getBestMove();
        if (ttEntry->getDepth() >= depth)
        {
            const auto ttScore = ttScoreToRealScore(ttEntry->getScore(), ss->ply);
            const auto ttFlags = ttEntry->getFlags();
            if (ttFlags == TranspositionTable::Flags::ExactScore
            || (ttFlags == TranspositionTable::Flags::UpperBoundScore && ttScore <= alpha)
            || (ttFlags == TranspositionTable::Flags::LowerBoundScore && ttScore >= beta))
            {
                return ttScore;
            }
        }
    }

    // Get the static evaluation of the position. Not needed in nodes where we are in check.
    const auto staticEval = (inCheck ? -infinity : evaluation.evaluate(pos));

    // Reverse futility pruning / static null move pruning.
    // Not useful in PV-nodes as this tries to search for nodes where score >= beta but in PV-nodes score < beta.
    if (!pvNode && !inCheck && pos.getNonPawnPieceCount(pos.getSideToMove()) && depth <= reverseFutilityDepth && staticEval - reverseFutilityMargin(depth) >= beta)
    {
        return staticEval - reverseFutilityMargin(depth);
    }

    // Razoring.
    // Not useful in PV-nodes as this tries to search for nodes where score <= alpha but in PV-nodes score > alpha.
    if (!pvNode && !inCheck && depth <= razoringDepth && staticEval + razoringMargin(depth) <= alpha)
    {
        auto razoringAlpha = alpha - razoringMargin(depth);
        score = quiescenceSearch(pos, 0, razoringAlpha, razoringAlpha + 1, false, ss);
        if (score <= razoringAlpha)
        {
            return score;
        }
    }

    // Null move pruning.
    // Not used when in a PV-node because we should _never_ fail high at a PV-node so doing this is a waste of time.
    // I don't really like the staticEval >= beta condition but the gain in elo is significant so...
    if (!pvNode && ss->allowNullMove && !inCheck && depth > 1 && staticEval >= beta && pos.getNonPawnPieceCount(pos.getSideToMove()))
    {
        const auto R = baseNullReduction + depth / 6;
        if (!(ttEntry
            && ttEntry->getFlags() == TranspositionTable::Flags::UpperBoundScore
            && ttEntry->getDepth() >= depth - 1 - R
            && ttEntry->getScore() <= alpha))
        {
            repetitionHashes[rootPly + ss->ply] = pos.getHashKey();
            ss->currentMove = Move();
            Position newPosition(pos);
            newPosition.makeNullMove();
            ++nodeCount;
            --nodesToTimeCheck;
            (ss + 1)->allowNullMove = false;
            score = depth - 1 - R > 0 ? -search<false>(newPosition, depth - 1 - R, -beta, -beta + 1, false, ss + 1)
                : -quiescenceSearch(newPosition, 0, -beta, -beta + 1, false, ss + 1);
            (ss + 1)->allowNullMove = true;
            if (score >= beta)
            {
                // Don't return unproven mate scores as they cause some instability.
                if (isMateScore(score))
                    score = beta;
                transpositionTable.save(pos.getHashKey(), 
                                        ttMove, 
                                        realScoreToTtScore(score, ss->ply), 
                                        depth, 
                                        TranspositionTable::Flags::LowerBoundScore);
                return score;
            }
        }
    }

    // Internal iterative deepening.
    if (ttMove.empty() && (pvNode ? depth > 4 : depth > 7))
    {
        // We can skip nullmove in IID since if it would have worked we wouldn't be here.
        ss->allowNullMove = false;
        score = search<pvNode>(pos, pvNode ? depth - 2 : depth / 2, alpha, beta, inCheck, ss);
        ss->allowNullMove = true;

        // Now probe the TT and get the best move.
        const auto tte = transpositionTable.probe(pos.getHashKey());
        if (tte)
        {
            ttMove = tte->getBestMove();
        }
    }

    // Futility pruning is useless at PV-nodes for the same reason as razoring.
    const auto futileNode = (!pvNode && !inCheck && depth <= futilityDepth && staticEval + futilityMargin(depth) <= alpha);
    const auto lmpNode = (!pvNode && !inCheck && depth <= lmpDepth);
    const auto lmrNode = (!inCheck && depth >= lmrDepthLimit);
    const auto seePruningNode = !pvNode && !inCheck && depth <= seePruningDepth;
    const auto killers = killerTable.getKillers(ss->ply);
    const auto counter = counterMoveTable.getCounterMove(pos, (ss - 1)->currentMove);

    MoveSort ms(pos, historyTable, ttMove, killers.first, killers.second, counter, inCheck);

    repetitionHashes[rootPly + ss->ply] = pos.getHashKey();
    for (auto i = 0;; ++i)
    {
        const auto move = ms.next();
        if (move.empty()) break;

        const auto givesCheck = pos.givesCheck(move);
        const auto newDepth = depth - 1;
        const auto quietMove = !pos.captureOrPromotion(move);
        if (quietMove) quietsSearched.emplace_back(move);
        const auto nonCriticalMove = !givesCheck && quietMove && move != ttMove
                                                              && move != killers.first
                                                              && move != killers.second
                                                              && move != counter;
        ++nodeCount;
        --nodesToTimeCheck;

        // Futility pruning and late move pruning. Oh, SEE pruning as well.
        if (nonCriticalMove)
        {
            if (futileNode)
            {
                bestScore = std::max(bestScore, staticEval + futilityMargin(depth));
                ++prunedMoves;
                continue;
            }

            if (lmpNode && i >= lmpMoveCounts[depth])
            {
                ++prunedMoves;
                continue;
            }

            if (seePruningNode && pos.SEE(move) < 0)
            {
                ++prunedMoves;
                continue;
            }
        }

        if (!pos.legal(move, inCheck))
        {
            continue;
        }

        Position newPosition(pos);
        newPosition.makeMove(move);
        ss->currentMove = move;
        if (!movesSearched)
        {
            score = newDepth > 0 ? -search<pvNode>(newPosition, newDepth, -beta, -alpha, givesCheck != 0, ss + 1)
                : -quiescenceSearch(newPosition, 0, -beta, -alpha, givesCheck != 0, ss + 1);
        }
        else
        {
            const auto reduction = ((lmrNode && nonCriticalMove) ? lmrReductions[std::min(i, 63)][std::min(depth, 63)] : 0);

            score = newDepth - reduction > 0 ? -search<false>(newPosition, newDepth - reduction, -alpha - 1, -alpha, givesCheck != 0, ss + 1)
                                             : -quiescenceSearch(newPosition, 0, -alpha - 1, -alpha, givesCheck != 0, ss + 1);

            // The LMR'd move didn't fail low, drop the reduction because that most likely caused the fail high.
            // If we are in a PV-node the alternative is to open the window first. The more unstable the search the better doing that is.
            // Before the tuned evaluation opening the window was better, after the tuned eval it is worse. Why?
            if (reduction && score > alpha)
            {
                score = newDepth > 0 ? -search<false>(newPosition, newDepth, -alpha - 1, -alpha, givesCheck != 0, ss + 1)
                                     : -quiescenceSearch(newPosition, 0, -alpha - 1, -alpha, givesCheck != 0, ss + 1);
            }

            // If we are in a PV-node this is used to get the exact score for a new PV.
            // Since we used null window on the previous searches the score is only a bound, and this won't do for a PV.
            if (score > alpha && score < beta)
            {
                score = newDepth > 0 ? -search<true>(newPosition, newDepth, -beta, -alpha, givesCheck != 0, ss + 1)
                                     : -quiescenceSearch(newPosition, 0, -beta, -alpha, givesCheck != 0, ss + 1);
            }
        }
        ++movesSearched;

        if (score > bestScore)
        {
            if (score > alpha)
            {
                if (score >= beta)
                {
                    transpositionTable.save(pos.getHashKey(), 
                                            move, 
                                            realScoreToTtScore(score, ss->ply), 
                                            depth, 
                                            TranspositionTable::Flags::LowerBoundScore);

                    // Updating move ordering heuristics while in check is not good, pollutes tables.
                    if (!inCheck)
                    {
                        if (quietMove)
                        {
                            historyTable.addCutoff(pos, move, depth);
                            killerTable.update(move, ss->ply);
                            counterMoveTable.update(pos, move, (ss - 1)->currentMove);
                        }
                        for (auto j = 0; j < quietsSearched.size() - 1; ++j)
                        {
                            historyTable.addNotCutoff(pos, quietsSearched.getMove(j), depth);
                        }
                    }

                    return score;
                }
                bestMove = move;
                alpha = score;
                ttFlag = TranspositionTable::Flags::ExactScore;
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
        // Looks like we pruned all moves away. Return some approximation of the score. Just alpha is fine too.
        // Not used currently.
        return staticEval; 
    }

    transpositionTable.save(pos.getHashKey(), bestMove, realScoreToTtScore(bestScore, ss->ply), depth, ttFlag);

    return bestScore;
}

