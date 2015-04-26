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
#include "utils\synchronized_ostream.hpp"
#include "utils\exception.hpp"
#include "utils\clamp.hpp"
#include "movesort.hpp"

const int mateScore = 32767; // mate in 0
const int minMateScore = 32767 - 1000; // mate in 500
const int infinity = mateScore + 1;

const int aspirationWindow = 16;
const int baseNullReduction = 3;
const int futilityDepth = 7;
const int deltaPruningMargin = 50;
const int reverseFutilityDepth = 3;
const int lmrDepthLimit = 3;
const int lmpDepth = 6;
std::array<int, 1 + lmpDepth> lmpMoveCounts;
const int razoringDepth = 3;
const int seePruningDepth = 3;

const int16_t hashMoveScore = 30000;
const int16_t captureMoveScore = hashMoveScore >> 1;
const std::array<int16_t, 1 + 4> killerMoveScore = {
    0, hashMoveScore >> 2, hashMoveScore >> 3, hashMoveScore >> 4, hashMoveScore >> 5
};
const int16_t counterMoveScore = hashMoveScore >> 6;

int matedInPly(const int ply)
{
    return (-mateScore + ply);
}

int mateInPly(const int ply)
{
    return (mateScore - ply);
}

int isWinScore(const int score)
{
    return score >= minMateScore;
}

int isLoseScore(const int score)
{
    return score <= -minMateScore;
}

int isMateScore(const int score)
{
    return isWinScore(score) || isLoseScore(score);
}

bool quietMove(const Position& pos, const Move& move)
{
    return pos.getBoard(move.getTo()) == Piece::Empty && (move.getPromotion() == Piece::Empty || move.getPromotion() == Piece::King);
}

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

int razoringMargin(int depth)
{
    return depth * 50 + 50;
}

int reverseFutilityMargin(int depth)
{
    return 50 * depth + 100;
}

int futilityMargin(int depth)
{
    return 25 * depth + 100;
}

Search::Search()
{
    contempt = {{ 0, 0 }};
    rootPly = 0;
    contempt.fill(0);
    searching = false;
    pondering = false;
    infinite = false;
    targetTime = maxTime = 0;
    nodeCount = 0;
    tbHits = 0;
    nextSendInfo = 1000;
    nodesToTimeCheck = 10000;
    selDepth = 1;
    searchNeedsMoreTime = false;

    for (auto i = 0; i < 64; ++i)
    {
        for (auto j = 0; j < 64; ++j)
        {
            lmrReductions[i][j] = static_cast<int>(std::max(1.0, (std::log(i + 1) * std::log(j + 1)) / 1.70));
        }
    }

    for (auto i = 0; i < 1 + lmpDepth; ++i)
    {
        lmpMoveCounts[i] = static_cast<int>(std::round(2.98484 + std::pow(i, 1.74716)));
    }
}

void Search::orderCaptures(const Position& pos, MoveList& moveList, const Move& ttMove)
{
    for (auto i = 0; i < moveList.size(); ++i)
    {
        auto& move = moveList[i];

        if (move.getMove() == ttMove.getMove())
        {
            move.setScore(hashMoveScore); 
        }
        else if (!quietMove(pos, move))
        {
            auto score = pos.mvvLva(move);
            score += captureMoveScore; 
            move.setScore(score);
        }
        else
        {
            move.setScore(historyTable.getScore(pos, move));
        }
    }
}

void selectMove(MoveList& moveList, const int currentMove)
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

// Move ordering goes like this:
// 1. Hash move (which can also be the PV-move)
// 2. Good captures and promotions
// 3. Equal captures and promotions
// 4. Killer moves
// 5. Quiet moves sorted by the history heuristic
// 6. Bad captures
void Search::orderMoves(const Position& pos, MoveList& moveList, const Move& ttMove, const int ply, const Move& opponentMove) const
{
    const auto counterMove = counterMoveTable.getCounterMove(pos, opponentMove);

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
            const auto killerScore = killerTable.isKiller(move, ply);
            if (killerScore > 0)
            {
                move.setScore(killerMoveScore[killerScore]);
            }
            else if (move.getMove() == counterMove)
            {
                move.setScore(counterMoveScore);
            }
            else
            {
                move.setScore(historyTable.getScore(pos, move));
            }
        }
    }
}

void Search::infoCurrMove(const Move& move, int depth, int nr)
{
    sync_cout << "info depth " << depth
              << " currmove " << moveToUciFormat(move)
              << " currmovenumber " << nr + 1 << std::endl;
} 

std::string movesToUciFormat(const std::vector<Move>& moves)
{
    std::string s;

    for (auto& move : moves)
    {
        s += moveToUciFormat(move) + " ";
    }

    return s;
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
        << " pv " << movesToUciFormat(moves) << std::endl;

    sync_cout << ss.str();
}

void removeIllegalMoves(Position& pos, MoveList& moveList, bool inCheck)
{
    auto marker = 0;

    for (auto i = 0; i < moveList.size(); ++i)
    {
        if (pos.legal(moveList[i], inCheck))
        {
            moveList[marker++] = moveList[i];
        }
    }

    moveList.resize(marker);
}

void Search::think(const Position& root, SearchParameters searchParameters, int newRootPly, std::array<HashKey, 1024> newRepetitionHashKeys, int contemptValue)
{
    auto alpha = -infinity;
    auto beta = infinity;
    auto delta = aspirationWindow;
    auto score = matedInPly(0);
    auto inCheck = root.inCheck();
    Position pos(root);
    MoveList rootMoveList;
    std::vector<Move> pv;
    Move bestMove;

    tbHits = 0;
    nodeCount = 0;
    nodesToTimeCheck = 10000;
    contempt[root.getSideToMove()] = -contemptValue;
    contempt[!root.getSideToMove()] = contemptValue;
    searchNeedsMoreTime = false;
    selDepth = 1;
    nextSendInfo = 1000;
    searching = true;
    pondering = searchParameters.ponder;
    infinite = (searchParameters.infinite || searchParameters.depth > 0 || searchParameters.nodes > 0);
    const auto maxDepth = (searchParameters.depth > 0 ? std::min(searchParameters.depth + 1, 128) : 128);
    maxNodes = (searchParameters.nodes > 0 ? searchParameters.nodes : std::numeric_limits<size_t>::max());
    rootPly = newRootPly;
    repetitionHashes = newRepetitionHashKeys;
    transpositionTable.startNewSearch();
    historyTable.age();
    counterMoveTable.clear();
    killerTable.clear(); 
    sw.reset();
    sw.start();

    // Allocate the time limits.
    if (searchParameters.moveTime)
    {
        targetTime = maxTime = searchParameters.moveTime;
    }
    else
    {
        const auto lagBuffer = 50;
        auto time = searchParameters.time[root.getSideToMove()];
        auto increment = searchParameters.increment[root.getSideToMove()];
        targetTime = clamp(time / std::min(searchParameters.movesToGo, 25) + increment - lagBuffer, 1, time - lagBuffer);
        maxTime = clamp(time / 2 + increment, 1, time - lagBuffer);
    }

    inCheck ? moveGen.generateLegalEvasions(pos, rootMoveList)
            : moveGen.generatePseudoLegalMoves(pos, rootMoveList);
    removeIllegalMoves(pos, rootMoveList, inCheck);

    // Get the tt move from a possible previous search.
    auto ttEntry = transpositionTable.probe(pos.getHashKey());
    if (ttEntry)
    {
        bestMove.setMove(ttEntry->getBestMove());
    }

    std::array<SearchStack, 128 + 1> searchStacks;
    for (auto i = 0; i < 128 + 1; ++i)
    {
        searchStacks[i].clear(i);
    }
    auto ss = &searchStacks[0];

    repetitionHashes[rootPly] = pos.getHashKey();
    for (auto depth = 1; depth < maxDepth;)
    {
        auto previousAlpha = alpha;
        auto previousBeta = beta;
        auto movesSearched = 0;
        auto lmrNode = (!inCheck && depth >= lmrDepthLimit);
        auto bestScore = -mateScore;

        orderMoves(pos, rootMoveList, bestMove, 0, Move());
        try {
            for (auto i = 0; i < rootMoveList.size(); ++i)
            {
                selectMove(rootMoveList, i);
                const auto& move = rootMoveList[i];
                ++nodeCount;
                --nodesToTimeCheck;
                searchNeedsMoreTime = i > 0;

                // Start sending currmove info only after one second has elapsed.
                if (sw.elapsed<std::chrono::milliseconds>() > 1000)
                {
                    infoCurrMove(move, depth, i);
                }

                auto givesCheck = pos.givesCheck(move);
                auto extension = givesCheck ? 1 : 0;
                auto newDepth = depth - 1 + extension;
                auto nonCriticalMove = !extension && move.getScore() >= 0 && move.getScore() < counterMoveScore;

                Position newPosition(pos);
                newPosition.makeMove(move);
                ss->currentMove = move;
                if (!movesSearched)
                {
                    score = newDepth > 0 ? -search<true>(newPosition, newDepth, -beta, -alpha, givesCheck != 0, ss + 1)
                                         : -quiescenceSearch(newPosition, 0, -beta, -alpha, givesCheck != 0, ss + 1);
                }
                else
                {
                    auto reduction = ((lmrNode && nonCriticalMove) ? lmrReductions[std::min(i, 63)][std::min(depth, 63)] : 0);

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
                            if (quietMove(pos, move))
                            {
                                historyTable.addCutoff(pos, move, depth);
                                killerTable.addKiller(move, 0);
                            }
                            for (auto j = 0; j < i; ++j)
                            {
                                const auto& move2 = rootMoveList[j];
                                if (quietMove(pos, move2))
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
                    transpositionTable.save(pos.getHashKey(), bestMove, realScoreToTtScore(score, 0), depth, lowerBound ? LowerBoundScore : UpperBoundScore);
                    pv = transpositionTable.extractPv(pos);
                    infoPv(pv, depth, score, lowerBound ? LowerBoundScore : UpperBoundScore);
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
                        transpositionTable.save(pos.getHashKey(), bestMove, realScoreToTtScore(score, 0), depth, ExactScore);
                        pv = transpositionTable.extractPv(pos);
                        infoPv(pv, depth, score, ExactScore);
                    }
                }
            }
        }
        catch (const StopSearchException&)
        {
            pos = root; // Exception messes up the position, fix it.
        }

        transpositionTable.save(pos.getHashKey(), bestMove, realScoreToTtScore(bestScore, 0), depth, bestScore >= beta ? LowerBoundScore : ExactScore);
        pv = transpositionTable.extractPv(pos);

        // If this is not an infinite search, we have reached sufficient depth and there is only one root move then stop searching.
        if (!infinite && rootMoveList.size() == 1 && depth > 6)
        {
            break;
        }

        if (!searching)
        {
            break;
        }

        infoPv(pv, depth, bestScore, ExactScore);

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

    sw.stop();
    // Make sure that the the flag that we are searching is set to false when we quit.
    // If we somehow reach maximum depth we might not reset the flag otherwise.
    searching = false;
    auto searchTime = sw.elapsed<std::chrono::milliseconds>();
    sync_cout << "info time " << searchTime
              << " nodes " << nodeCount
              << " nps " << (nodeCount / (searchTime + 1)) * 1000
              << " tbhits " << tbHits << std::endl
              << "bestmove " << moveToUciFormat(pv[0]) << std::endl;
}

// Technically we are checking for 2 - fold repetitions instead of 3 - fold, but that is enough for game theoric correctness.
bool Search::repetitionDraw(const Position& pos, int ply) const
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

int Search::quiescenceSearch(const Position& pos, const int depth, int alpha, int beta, const bool inCheck, SearchStack* ss)
{
    int bestScore, delta;
    MoveList moveList;
    Move bestMove;
    auto ttFlag = UpperBoundScore;

    // Don't go over max depth.
    if (ss->ply >= 128)
    {
        return evaluation.evaluate(pos);
    }

    // Check for fifty move draws.
    if (pos.getFiftyMoveDistance() >= 100)
    {
        if (inCheck)
        {
            moveGen.generateLegalEvasions(pos, moveList);
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

    // We use only two depths when saving info to the TT, one for when we search captures+checks and one for when we search just captures.
    // Since when we are in check we search all moves regardless of depth it goes to the first category as well.
    // It seems that when this part was broken then not pruning checks below didn't work either for some reason.
    const auto ttDepth = (inCheck || depth >= 0) ? 0 : -1;

    auto ttEntry = transpositionTable.probe(pos.getHashKey());
    if (ttEntry)
    {
        bestMove.setMove(ttEntry->getBestMove());
        if (ttEntry->getDepth() >= ttDepth)
        {
            auto ttScore = ttScoreToRealScore(ttEntry->getScore(), ss->ply);
            auto ttFlags = ttEntry->getFlags();
            if (ttFlags == ExactScore || (ttFlags == UpperBoundScore && ttScore <= alpha) || (ttFlags == LowerBoundScore && ttScore >= beta))
                return ttScore;
        }
    }

    if (inCheck)
    {
        bestScore = matedInPly(ss->ply);
        delta = -infinity;
        moveGen.generateLegalEvasions(pos, moveList);
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
        depth >= 0 ? moveGen.generatePseudoLegalCapturesAndQuietChecks(pos, moveList) : moveGen.generatePseudoLegalCaptures(pos, moveList, false);
    }

    orderCaptures(pos, moveList, bestMove);
    repetitionHashes[rootPly + ss->ply] = pos.getHashKey();
    for (auto i = 0; i < moveList.size(); ++i)
    {
        selectMove(moveList, i);
        const auto& move = moveList[i];
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
                    transpositionTable.save(pos.getHashKey(), move, realScoreToTtScore(score, ss->ply), ttDepth, LowerBoundScore);
                    return score;
                }
                bestMove = move;
                ttFlag = ExactScore;
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

    auto bestScore = matedInPly(ss->ply), movesSearched = 0, prunedMoves = 0;
    auto ttFlag = UpperBoundScore;
    MoveList quietsSearched;
    Move bestMove;
    uint16_t ttMove = 0;
    int score;

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

    // Small speed optimization, runs fine without it.
    transpositionTable.prefetch(pos.getHashKey());

    // Time check things.
    if (nodesToTimeCheck <= 0)
    {
        nodesToTimeCheck = 10000;
        auto time = static_cast<int64_t>(sw.elapsed<std::chrono::milliseconds>()); // Casting works around a few warnings.

        if (nodeCount >= maxNodes)
        {
            searching = false;
        }

        if (!infinite) // Can't stop search if ordered to run indefinitely
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
            sync_cout << "info nodes " << nodeCount 
                      << " time " << time 
                      << " nps " << (nodeCount / (time + 1)) * 1000 
                      << " tbhits " << tbHits << std::endl;
        }
    }

    // Check for fifty move draws.
    if (pos.getFiftyMoveDistance() >= 100)
    {
        if (inCheck)
        {
            // Might as well use quietsSearched at this point, we are returning anyways.
            moveGen.generateLegalEvasions(pos, quietsSearched);
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
    auto ttEntry = transpositionTable.probe(pos.getHashKey());
    if (ttEntry)
    {
        ttMove = ttEntry->getBestMove();
        if (ttEntry->getDepth() >= depth)
        {
            auto ttScore = ttScoreToRealScore(ttEntry->getScore(), ss->ply);
            auto ttFlags = ttEntry->getFlags();
            if (ttFlags == ExactScore || (ttFlags == UpperBoundScore && ttScore <= alpha) || (ttFlags == LowerBoundScore && ttScore >= beta))
                return ttScore;
        }
    }

    // Get the static evaluation of the position. Not needed in nodes where we are in check.
    auto staticEval = (inCheck ? -infinity : evaluation.evaluate(pos));

    // Reverse futility pruning / static null move pruning.
    // Not useful in PV-nodes as this tries to search for nodes where score >= beta but in PV-nodes score < beta.
    if (!pvNode && !inCheck && pos.nonPawnMaterial(pos.getSideToMove()) && depth <= reverseFutilityDepth && staticEval - reverseFutilityMargin(depth) >= beta)
        return staticEval - reverseFutilityMargin(depth);

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
    if (!pvNode && ss->allowNullMove && !inCheck && staticEval >= beta && pos.nonPawnMaterial(pos.getSideToMove()))
    {
        const auto R = baseNullReduction + depth / 6;
        if (!(ttEntry
            && ttEntry->getFlags() == UpperBoundScore
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
                transpositionTable.save(pos.getHashKey(), Move(ttMove, 0), realScoreToTtScore(score, ss->ply), depth, LowerBoundScore);
                return score;
            }
        }
    }

    // Internal iterative deepening.
    if (!ttMove && (pvNode ? depth > 4 : depth > 7))
    {
        // We can skip nullmove in IID since if it would have worked we wouldn't be here.
        ss->allowNullMove = false;
        score = search<pvNode>(pos, pvNode ? depth - 2 : depth / 2, alpha, beta, inCheck, ss);
        ss->allowNullMove = true;

        // Now probe the TT and get the best move.
        auto tte = transpositionTable.probe(pos.getHashKey());
        if (tte)
        {
            ttMove = tte->getBestMove();
        }
    }

    // Futility pruning is useless at PV-nodes for the same reason as razoring.
    auto futileNode = (!pvNode && !inCheck && depth <= futilityDepth && staticEval + futilityMargin(depth) <= alpha);
    auto lmpNode = (!pvNode && !inCheck && depth <= lmpDepth);
    auto lmrNode = (!inCheck && depth >= lmrDepthLimit);
    auto seePruningNode = !pvNode && !inCheck && depth <= seePruningDepth;

    const auto killerA = killerTable.getKillerA(ss->ply);
    const auto killerB = killerTable.getKillerB(ss->ply);
    const auto counter = counterMoveTable.getCounterMove(pos, (ss - 1)->currentMove);
    MoveSort ms(pos, historyTable, ttMove, killerA, killerB, counter, inCheck);

    repetitionHashes[rootPly + ss->ply] = pos.getHashKey();
    for (auto i = 0;; ++i)
    {
        const auto move = ms.next();
        if (move.empty()) break;

        auto givesCheck = pos.givesCheck(move);
        auto extension = givesCheck ? 1 : 0;
        auto newDepth = depth - 1 + extension;
        auto quietMove = !pos.captureOrPromotion(move);
        if (quietMove) quietsSearched.emplace_back(move);
        auto nonCriticalMove = !extension && quietMove && move.getMove() != ttMove
                                                       && move.getMove() != killerA 
                                                       && move.getMove() != killerB 
                                                       && move.getMove() != counter;
        ++nodeCount;
        --nodesToTimeCheck;

        // Futility pruning and late move pruning. Oh, SEE pruning as well.
        if (nonCriticalMove && !isLoseScore(bestScore))
        {
            if (futileNode)
            {
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
                                 : -quiescenceSearch(newPosition, 0, -beta, -alpha, givesCheck != 0, ss + 1) ;
        }
        else
        {
            auto reduction = ((lmrNode && nonCriticalMove) ? lmrReductions[std::min(i, 63)][std::min(depth, 63)] : 0);

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
                    transpositionTable.save(pos.getHashKey(), move, realScoreToTtScore(score, ss->ply), depth, LowerBoundScore);

                    if (!inCheck)
                    {
                        if (quietMove)
                        {
                            historyTable.addCutoff(pos, move, depth);
                            killerTable.addKiller(move, ss->ply);
                            counterMoveTable.updateCounterMoves(pos, move, (ss - 1)->currentMove);
                        }
                        for (auto j = 0; j < quietsSearched.size() - 1; ++j)
                        {
                            historyTable.addNotCutoff(pos, quietsSearched[j], depth);
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

    transpositionTable.save(pos.getHashKey(), bestMove, realScoreToTtScore(bestScore, ss->ply), depth, ttFlag);

    return bestScore;
}
