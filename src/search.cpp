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
#include "movegen.hpp"
#include "movesort.hpp"
#include "utils/clamp.hpp"
#include "utils/exception.hpp"
#include "syzygy/tbprobe.hpp"

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

// Removes illegal moves from a moveList.
// Remove after getting MoveSort working everywhere.
void removeIllegalMoves(const Position& pos, MoveList& moveList, bool inCheck)
{
    auto marker = 0;

    for (auto i = 0; i < moveList.size(); ++i)
    {
        if (pos.legal(moveList.getMove(i), inCheck))
        {
            moveList.setMove(marker++, moveList.getMove(i));
        }
    }

    moveList.resize(marker);
}

void Search::orderRootMoves(const Position& pos, MoveList& moveList, const Move& ttMove) const
{
    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto move = moveList.getMove(i);

        if (move == ttMove) // Move from transposition table
        {
            moveList.setScore(i, hashMoveScore);
        }
        else if (pos.captureOrPromotion(move))
        {
            auto score = pos.SEE(move);
            if (score >= 0) // Order good captures and promotions after ttMove
            {
                score += captureMoveScore;
            }
            moveList.setScore(i, score);
        }
        else
        {
            const auto killers = killerTable.getKillers(0);
            if (move == killers.first)
            {
                moveList.setScore(i, killerMoveScore[1]);
            }
            else if (move == killers.second)
            {
                moveList.setScore(i, killerMoveScore[2]);
            }
            else
            {
                moveList.setScore(i, historyTable.getScore(pos, move));
            }
        }
    }
}

Search::Search(SearchListener& sl):
    tp(1), listener(sl), searchNeedsMoreTime(false), nodesToTimeCheck(10000), nextSendInfo(1000), 
    targetTime(1000), maxTime(10000), maxNodes(std::numeric_limits<size_t>::max()),
    tbHits(0), nodeCount(0), selDepth(0), searching(false), pondering(false), infinite(false), 
    cardinality(6), probeDepth(1), use50(true), rootPly(0), repetitionHashes({}), contempt({})
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

std::vector<Move> Search::extractPv(const Position& pos) const
{
    Position root(pos);
    std::vector<Move> pv;
    std::unordered_set<HashKey> previousHashes;

    for (auto ply = 0; ply < 128; ++ply)
    {
        const auto entry = transpositionTable.probe(root.getHashKey());

        // No entry found -> end of PV
        if (!entry)
            break;

        // No move found in the entry, so we cannot add to the PV
        if (entry->getBestMove().empty())
            break;

        // Repetition draw -> end of PV
        if (previousHashes.count(root.getHashKey()) > 0)
            break;

        // No exact score hash entry to use -> end of PV 
        // If we are very near the root we accept all flags as we absolutely need the move 
        // to be played and a ponder move is very important as well.
        if (entry->getFlags() != TranspositionTable::Flags::ExactScore && ply >= 2)
            break;

        const auto m = entry->getBestMove();
        pv.push_back(m);
        previousHashes.insert(root.getHashKey());
        root.makeMove(m);
    }

    return pv;
}

void Search::go(const Position& root, const SearchParameters& sp)
{
    std::unique_lock<std::mutex> waitLock(waitMutex);
    tp.addJob(&Search::think, this, root, sp);
    // Wait here until the search function has started.
    // Think of a chain of commands "go", "stop", "go", "stop" sent within 1 or 2 milliseconds.
    // If this part isn't here, some commands could get lost.
    waitCv.wait(waitLock);
}

void Search::think(const Position& root, SearchParameters sp)
{
    const auto inCheck = root.inCheck();
    auto alpha = -infinity;
    auto beta = infinity;
    auto delta = aspirationWindow;
    auto score = matedInPly(0);
    Position pos(root);
    MoveList rootMoveList;
    std::vector<Move> pv;
    Move bestMove;

    tbHits = 0;
    nodeCount = 0;
    nodesToTimeCheck = 10000;
    contempt[root.getSideToMove()] = -sp.mContempt;
    contempt[!root.getSideToMove()] = sp.mContempt;
    searchNeedsMoreTime = false;
    selDepth = 1;
    nextSendInfo = 1000;
    searching = true;
    pondering = sp.mPonder;
    infinite = (sp.mInfinite || sp.mDepth > 0 || sp.mNodes > 0);
    const auto maxDepth = (sp.mDepth > 0 ? std::min(sp.mDepth + 1, 128) : 128);
    maxNodes = (sp.mNodes > 0 ? sp.mNodes : std::numeric_limits<size_t>::max());
    rootPly = sp.mRootPly;
    repetitionHashes = sp.mHashKeys;
    cardinality = sp.mSyzygyProbeLimit;
    probeDepth = sp.mSyzygyProbeDepth;
    use50 = sp.mSyzygy50MoveRule;
    transpositionTable.startNewSearch();
    historyTable.age();
    counterMoveTable.clear();
    killerTable.clear();

    {
        // Notify the thread which is waiting in "go" that the search has started.
        // The lock is necessary to make sure that the thread in "go" is actually waiting on a condition variable.
        // Without the lock in that case the thread blocks forever and we are done.
        // No really, that actually happens relatively often. I had fun debugging it too.
        std::unique_lock<std::mutex> waitLock(waitMutex);
        waitCv.notify_one();
    }

    sw.reset();
    sw.start();

    // Allocate the time limits.
    if (sp.mMoveTime)
    {
        targetTime = maxTime = sp.mMoveTime;
    }
    else
    {
        const auto lagBuffer = 50;
        const auto time = sp.mTime[root.getSideToMove()];
        const auto increment = sp.mIncrement[root.getSideToMove()];
        targetTime = clamp(time / std::min(sp.mMovesToGo, 25) + increment - lagBuffer, 1, time - lagBuffer);
        maxTime = clamp(time / 2 + increment, 1, time - lagBuffer);
        if (sp.mPonderOption)
        {
            targetTime += targetTime / 3;
            targetTime = clamp(targetTime, static_cast<uint64_t>(1), maxTime);
        }
    }

    inCheck ? MoveGen::generateLegalEvasions(pos, rootMoveList)
            : MoveGen::generatePseudoLegalMoves(pos, rootMoveList);
    removeIllegalMoves(pos, rootMoveList, inCheck);

    // Skip TB probing when no TB found: !maxCardinality -> !cardinality
    if (cardinality > Syzygy::maxCardinality)
    {
        cardinality = Syzygy::maxCardinality;
        probeDepth = 0;
    }

    if (cardinality >= pos.getTotalPieceCount())
    {
        score = 0;
        auto rootInTb = Syzygy::rootProbe(pos, rootMoveList, score);

        // Do not probe tablebases during the search if the root position is in the tablebases.
        if (rootInTb)
        {
            cardinality = 0;
        }
        else
        {
            // If DTZ tables are missing, use WDL tables as a fallback
            rootInTb = Syzygy::rootProbeWdl(pos, rootMoveList, score);

            // Only probe during search if winning.
            if (rootInTb && score <= 0)
            {
                cardinality = 0;
            }
        }

        if (rootInTb)
        {
            tbHits = rootMoveList.size();
        }
    }

    // Get the tt move from a possible previous search.
    const auto ttEntry = transpositionTable.probe(pos.getHashKey());
    if (ttEntry)
    {
        bestMove = ttEntry->getBestMove();
    }

    std::vector<SearchStack> searchStack;
    for (auto i = 0; i < 128 + 1; ++i)
    {
        searchStack.emplace_back(i);
    }
    auto ss = &searchStack[0];

    for (auto depth = 1; depth < maxDepth;)
    {
        const auto previousAlpha = alpha;
        const auto previousBeta = beta;
        const auto lmrNode = (!inCheck && depth >= lmrDepthLimit);
        const auto killers = killerTable.getKillers(0);
        auto movesSearched = 0;
        auto bestScore = -mateScore;

        orderRootMoves(pos, rootMoveList, bestMove);
        try {
            for (auto i = 0; i < rootMoveList.size(); ++i)
            {
                const auto move = selectMove(rootMoveList, i);
                ++nodeCount;
                --nodesToTimeCheck;
                searchNeedsMoreTime = i > 0;

                // Start sending currmove info only after one second has elapsed.
                if (sw.elapsed<std::chrono::milliseconds>() > 1000)
                {
                    listener.infoCurrMove(move, depth, i);
                }

                const auto givesCheck = pos.givesCheck(move);
                const auto newDepth = depth - 1;
                const auto quietMove = !pos.captureOrPromotion(move);
                const auto nonCriticalMove = !givesCheck && quietMove && move != bestMove
                                                                      && move != killers.first
                                                                      && move != killers.second;

                Position newPosition(pos);
                newPosition.makeMove(move);
                ss->mCurrentMove = move;
                repetitionHashes[rootPly] = pos.getHashKey();
                // Clear the ply + 2 killer slot to make sure you cannot inherit any killer from a cousin.
                killerTable.clear(2);

                if (!movesSearched)
                {
                    score = newDepth > 0 ? -search<true>(newPosition, newDepth, -beta, -alpha, givesCheck != 0, ss + 1)
                                         : -quiescenceSearch(newPosition, 0, -beta, -alpha, givesCheck != 0, ss + 1);
                }
                else
                {
                    const auto reduction = ((lmrNode && nonCriticalMove) ? lmrReductions[std::min(i, 63)][std::min(depth, 63)] : 0);

                    score = newDepth - reduction > 0 ? -search<false>(newPosition, newDepth - reduction, -alpha - 1, -alpha, givesCheck != 0, ss + 1)
                                                     : -quiescenceSearch(newPosition, 0, -alpha - 1, -alpha, givesCheck != 0, ss + 1);

                    if (reduction && score > alpha)
                    {
                        score = newDepth > 0 ? -search<false>(newPosition, newDepth, -alpha - 1, -alpha, givesCheck != 0, ss + 1)
                                             : -quiescenceSearch(newPosition, 0, -alpha - 1, -alpha, givesCheck != 0, ss + 1);
                    }
                    if (score > alpha && score < beta)
                    {
                        score = newDepth > 0 ? -search<true>(newPosition, newDepth, -beta, -alpha, givesCheck != 0, ss + 1)
                                             : -quiescenceSearch(newPosition, 0, -beta, -alpha, givesCheck != 0, ss + 1);
                    }
                }
                ++movesSearched;

                while (score >= beta || ((movesSearched == 1) && score <= alpha))
                {
                    const auto lowerBound = score >= beta;
                    if (lowerBound)
                    {
                        searchNeedsMoreTime = i > 0;
                        bestMove = move;
                        if (isWinScore(score))
                        {
                            beta = infinity;
                        }
                        else
                        {
                            beta = std::min(infinity, previousBeta + delta);
                        }
                        // Don't forget to update history and killer tables.
                        if (!inCheck)
                        {
                            if (quietMove)
                            {
                                historyTable.addCutoff(pos, move, depth);
                                killerTable.update(move, 0);
                            }
                            for (auto j = 0; j < i; ++j)
                            {
                                const auto move2 = rootMoveList.getMove(j);
                                if (!pos.captureOrPromotion(move2))
                                {
                                    historyTable.addNotCutoff(pos, move2, depth);
                                }
                            }
                        }
                    }
                    else
                    {
                        searchNeedsMoreTime = true;
                        if (isLoseScore(score))
                        {
                            alpha = -infinity;
                        }
                        else
                        {
                            alpha = std::max(-infinity, previousAlpha - delta);
                        }
                    }
                    delta *= 2;
                    transpositionTable.save(pos.getHashKey(), 
                                            bestMove, 
                                            realScoreToTtScore(score, 0), 
                                            depth, 
                                            lowerBound ? TranspositionTable::Flags::LowerBoundScore 
                                                       : TranspositionTable::Flags::UpperBoundScore);
                    pv = extractPv(pos);
                    listener.infoPv(pv, 
                                    sw.elapsed<std::chrono::milliseconds>(), 
                                    nodeCount,
                                    tbHits,    
                                    depth, 
                                    score, 
                                    lowerBound ? TranspositionTable::Flags::LowerBoundScore
                                               : TranspositionTable::Flags::UpperBoundScore,
                                    selDepth,
                                    transpositionTable.hashFull());
                    score = newDepth > 0 ? -search<true>(newPosition, newDepth, -beta, -alpha, givesCheck != 0, ss + 1)
                                         : -quiescenceSearch(newPosition, 0, -beta, -alpha, givesCheck != 0, ss + 1);
                }

                if (score > bestScore)
                {
                    bestScore = score;
                    if (score > alpha) // No need to handle the case score >= beta, that is done slightly above
                    {
                        bestMove = move;
                        alpha = score;
                        transpositionTable.save(pos.getHashKey(), 
                                                bestMove, 
                                                realScoreToTtScore(score, 0), 
                                                depth, 
                                                TranspositionTable::Flags::ExactScore);

                        pv = extractPv(pos);
                        listener.infoPv(pv,
                                        sw.elapsed<std::chrono::milliseconds>(),
                                        nodeCount,
                                        tbHits,
                                        depth,
                                        score,
                                        TranspositionTable::Flags::ExactScore, 
                                        selDepth,
                                        transpositionTable.hashFull());
                    }
                }
            }
        }
        catch (const StopSearchException&)
        {
            pos = root; // Exception might mess up the position, fix it. 
        }

        transpositionTable.save(pos.getHashKey(), 
                                bestMove, 
                                realScoreToTtScore(bestScore, 0), 
                                depth, 
                                TranspositionTable::Flags::ExactScore);

        pv = extractPv(pos);

        // If there is only one root move then stop searching.
        // Not done if we are in an infinite search or pondering, since we must search for ever in those cases.
        // depth > 6 is there to make sure we have something to ponder on.
        if (!infinite && !pondering && rootMoveList.size() == 1 && depth > 6)
        {
            break;
        }

        if (!searching)
        {
            break;
        }

        listener.infoPv(pv,
                        sw.elapsed<std::chrono::milliseconds>(),
                        nodeCount,
                        tbHits,
                        depth,
                        bestScore,
                        TranspositionTable::Flags::ExactScore,
                        selDepth,
                        transpositionTable.hashFull());

        // Adjust alpha and beta based on the last score.
        // Don't adjust if depth is low - it's a waste of time.
        // Also don't use aspiration windows when searching for faster mate.
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

    // If we are in an infinite search (or pondering) and we reach the max amount of iterations possible loop here until stopped.
    // This is done because returning is against the UCI-protocol.
    std::chrono::milliseconds dura(5);
    while (searching && (sp.mInfinite || pondering))
    {
        std::this_thread::sleep_for(dura);
    }

    sw.stop();
    // Make sure that the the flag that we are searching is set to false when we quit.
    // If we somehow reach maximum depth we might not reset the flag otherwise.
    searching = false;
    const auto searchTime = sw.elapsed<std::chrono::milliseconds>();
    listener.infoBestMove(pv,
                          searchTime,
                          nodeCount,
                          tbHits, 
                          transpositionTable.hashFull());
}

#ifdef _MSC_VER
#pragma warning (disable : 4127) // Shuts up warnings about conditional branches always being true/false.
#endif

template <bool pvNode>
int Search::search(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss)
{
    assert(alpha < beta);
    assert(depth > 0);
    assert(inCheck == pos.inCheck());

    const auto hashKey = ss->mExcludedMove.empty() ? pos.getHashKey() : pos.getHashKey() ^ Zobrist::manglingHashKey();
    auto bestScore = matedInPly(ss->mPly), movesSearched = 0, prunedMoves = 0;
    auto ttFlag = TranspositionTable::Flags::UpperBoundScore;
    int ttFlags = ttFlag;
    MoveList quietsSearched;
    Move bestMove, ttMove;
    int score, ttScore;

    // Small speed optimization, runs fine without it.
    transpositionTable.prefetch(hashKey);

    // Used for sending seldepth info.
    if (ss->mPly > selDepth)
    {
        selDepth = ss->mPly;
    }

    // Don't go over max ply.
    if (ss->mPly >= maxPly)
    {
        return evaluation.evaluate(pos);
    }

    // Time check things.
    if (nodesToTimeCheck <= 0)
    {
        nodesToTimeCheck = 10000;
        const auto time = sw.elapsed<std::chrono::milliseconds>();

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
            throw StopSearchException("allocated time has run out");
        }

        if (time >= nextSendInfo)
        {
            nextSendInfo += 1000;
            listener.infoRegular(nodeCount, tbHits, time, transpositionTable.hashFull());
        }
    }

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
    if (repetitionDraw(pos, ss->mPly))
    {
        return contempt[pos.getSideToMove()];
    }

    // Mate distance pruning, safe at all types of nodes.
    alpha = std::max(matedInPly(ss->mPly), alpha);
    beta = std::min(mateInPly(ss->mPly + 1), beta);
    if (alpha >= beta)
        return alpha;

    // Probe the transposition table. 
    auto ttEntry = transpositionTable.probe(hashKey);
    if (ttEntry)
    {
        ttMove = ttEntry->getBestMove();
        ttScore = ttScoreToRealScore(ttEntry->getScore(), ss->mPly);
        ttFlags = ttEntry->getFlags();
        if (ttEntry->getDepth() >= depth)
        {
            if (ttFlags == TranspositionTable::Flags::ExactScore
            || (ttFlags == TranspositionTable::Flags::UpperBoundScore && ttScore <= alpha)
            || (ttFlags == TranspositionTable::Flags::LowerBoundScore && ttScore >= beta))
            {
                return ttScore;
            }
        }
    }

    // Probe the syzygy tablebases.
    if (pos.getTotalPieceCount() <= cardinality
        && (pos.getTotalPieceCount() < cardinality || depth >= probeDepth)
        && pos.getFiftyMoveDistance() == 0)
    {
        int found;
        score = Syzygy::probeWdl(pos, found);

        if (found)
        {
            ++tbHits;
            const auto drawScore = use50 ? 1 : 0;
            score = score < -drawScore ? -minMateScore + ss->mPly
                  : score > drawScore ? minMateScore - ss->mPly
                  : score;
            return score;
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
    // If we have a TT move this node is most likely not a ALL-node as it wasn't so previously.
    if (!pvNode && !inCheck && ttMove.empty() && depth <= razoringDepth && staticEval + razoringMargin(depth) <= alpha)
    {
        const auto razoringAlpha = alpha - razoringMargin(depth);
        score = quiescenceSearch(pos, 0, razoringAlpha, razoringAlpha + 1, false, ss);
        if (score <= razoringAlpha)
        {
            return score;
        }
    }

    // Null move pruning.
    // Not used when in a PV-node because we should _never_ fail high at a PV-node so doing this is a waste of time.
    // I don't really like the staticEval >= beta condition but the gain in elo is significant so...
    if (!pvNode && ss->mAllowNullMove && !inCheck && depth > 1 && staticEval >= beta && pos.getNonPawnPieceCount(pos.getSideToMove()))
    {
        const auto R = baseNullReduction + depth / 6;
        const auto likelyFailLow = ttEntry && ttEntry->getFlags() == TranspositionTable::Flags::UpperBoundScore
                                && ttEntry->getDepth() >= depth - 1 - R && ttEntry->getScore() <= alpha;
        if (!likelyFailLow)
        {
            repetitionHashes[rootPly + ss->mPly] = hashKey;
            ss->mCurrentMove = Move();
            Position newPosition(pos);
            newPosition.makeNullMove();
            ++nodeCount;
            --nodesToTimeCheck;
            (ss + 1)->mAllowNullMove = false;
            score = depth - 1 - R > 0 ? -search<false>(newPosition, depth - 1 - R, -beta, -beta + 1, false, ss + 1)
                : -quiescenceSearch(newPosition, 0, -beta, -beta + 1, false, ss + 1);
            (ss + 1)->mAllowNullMove = true;
            if (score >= beta)
            {
                // Don't return unproven mate scores as they cause some instability.
                if (isMateScore(score))
                    score = beta;
                transpositionTable.save(hashKey, 
                                        ttMove, 
                                        realScoreToTtScore(score, ss->mPly), 
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
        ss->mAllowNullMove = false;
        score = search<pvNode>(pos, pvNode ? depth - 2 : depth / 2, alpha, beta, inCheck, ss);
        ss->mAllowNullMove = true;

        // Now probe the TT and get the best move.
        ttEntry = transpositionTable.probe(hashKey);
        if (ttEntry)
        {
            ttMove = ttEntry->getBestMove();
            ttScore = ttScoreToRealScore(ttEntry->getScore(), ss->mPly);
            ttFlags = ttEntry->getFlags();
        }
    }

    // Futility pruning is useless at PV-nodes for the same reason as razoring.
    const auto futileNode = (!pvNode && !inCheck && depth <= futilityDepth && staticEval + futilityMargin(depth) <= alpha);
    const auto lmpNode = (!pvNode && !inCheck && depth <= lmpDepth);
    const auto lmrNode = (!inCheck && depth >= lmrDepthLimit);
    const auto seePruningNode = !pvNode && !inCheck && depth <= seePruningDepth;
    const auto singularNode = (depth >= 8
                            && !ttMove.empty()
                            && ttFlags != TranspositionTable::Flags::UpperBoundScore
                            && ss->mExcludedMove.empty()
                            && ttEntry->getDepth() >= depth - 3);
    const auto killers = killerTable.getKillers(ss->mPly);
    const auto counter = counterMoveTable.getCounterMove(pos, (ss - 1)->mCurrentMove);

    MoveSort ms(pos, historyTable, ttMove, killers.first, killers.second, counter, inCheck);

    for (auto i = 0;; ++i)
    {
        const auto move = ms.next();
        if (move.empty()) break;
        if (move == ss->mExcludedMove) continue;

        auto newDepth = depth - 1;
        if (singularNode && move == ttMove && pos.legal(move, inCheck))
        {
            const auto newBeta = ttScore - 25;
            ss->mExcludedMove = move;
            ss->mAllowNullMove = false;
            score = search<false>(pos, depth / 2, newBeta - 1, newBeta, inCheck, ss);
            ss->mAllowNullMove = true;
            ss->mExcludedMove = Move();
            if (score < newBeta)
            {
                newDepth += 1;
            }
        }

        const auto givesCheck = pos.givesCheck(move);
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
        repetitionHashes[rootPly + ss->mPly] = hashKey;
        ss->mCurrentMove = move;

        // Clear the ply + 2 killer slot to make sure you cannot inherit any killer from a cousin.
        if (ss->mPly + 2 < maxPly)
        {
            killerTable.clear(ss->mPly + 2);
        }

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
                    transpositionTable.save(hashKey, 
                                            move, 
                                            realScoreToTtScore(score, ss->mPly), 
                                            depth, 
                                            TranspositionTable::Flags::LowerBoundScore);

                    // Updating move ordering heuristics while in check is not good, pollutes tables.
                    if (!inCheck)
                    {
                        if (quietMove)
                        {
                            historyTable.addCutoff(pos, move, depth);
                            killerTable.update(move, ss->mPly);
                            counterMoveTable.update(pos, move, (ss - 1)->mCurrentMove);
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

    transpositionTable.save(hashKey, bestMove, realScoreToTtScore(bestScore, ss->mPly), depth, ttFlag);

    return bestScore;
}

int Search::quiescenceSearch(const Position& pos, int depth, int alpha, int beta, bool inCheck, SearchStack* ss)
{
    assert(alpha < beta);
    assert(depth <= 0);
    assert(inCheck == pos.inCheck());

    int bestScore, delta;
    MoveList moveList;
    Move ttMove, bestMove;
    auto ttFlag = TranspositionTable::Flags::UpperBoundScore;

    // Small speed optimization, runs fine without it.
    transpositionTable.prefetch(pos.getHashKey());

    // Don't go over max ply.
    if (ss->mPly >= maxPly)
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
                return matedInPly(ss->mPly); // Can't claim draw on fifty move if mated.
            }
        }
        return contempt[pos.getSideToMove()];
    }

    // Check for repetition draws. 
    if (repetitionDraw(pos, ss->mPly))
    {
        return contempt[pos.getSideToMove()];
    }

    // Mate distance pruning, safe at all types of nodes.
    alpha = std::max(matedInPly(ss->mPly), alpha);
    beta = std::min(mateInPly(ss->mPly + 1), beta);
    if (alpha >= beta)
        return alpha;

    // We use only two depths when saving info to the TT, one for when we search captures+checks and one for when we search just captures.
    // Since when we are in check we search all moves regardless of depth it goes to the first category as well.
    // It seems that when this part was broken then not pruning checks below didn't work either for some reason.
    const auto ttDepth = (inCheck || depth >= 0) ? 0 : -1;

    const auto ttEntry = transpositionTable.probe(pos.getHashKey());
    if (ttEntry)
    {
        ttMove = ttEntry->getBestMove();
        if (ttEntry->getDepth() >= ttDepth)
        {
            const auto ttScore = ttScoreToRealScore(ttEntry->getScore(), ss->mPly);
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
        bestScore = matedInPly(ss->mPly);
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
    repetitionHashes[rootPly + ss->mPly] = pos.getHashKey();
    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto move = selectMove(moveList, i);
        const auto givesCheck = pos.givesCheck(move);
        ++nodeCount;
        --nodesToTimeCheck;

        // Only prune moves in quiescence search if we are not in check.
        if (!inCheck && move != ttMove)
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
                                            realScoreToTtScore(score, ss->mPly),
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

    transpositionTable.save(pos.getHashKey(), 
                            bestMove, 
                            realScoreToTtScore(bestScore, ss->mPly), 
                            ttDepth, 
                            ttFlag);

    return bestScore;
}