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

const int mateScore = 32767; // mate in 0
const int minMateScore = 32767 - 1000; // mate in 500
const int infinity = mateScore + 1;

const int aspirationWindow = 50;
const int nullReduction = 3;
const int futilityDepth = 4;
const std::array<int, 1 + 4> futilityMargins = {
    50, 125, 125, 300, 300
};
const int reverseFutilityDepth = 3;
const std::array<int, 1 + 3> reverseFutilityMargins = {
    0, 260, 445, 900
};
const int lmrFullDepthMoves = 4;
const int lmrReductionLimit = 3;
const int lmpDepth = 4;
const std::array<int, 1 + 4> lmpMoveCount = {
    0, 4, 8, 16, 32
};
const int razoringDepth = 3;
const std::array<int, 1 + 3> razoringMargins = {
    0, 125, 300, 300
};

const int32_t hashMoveScore = 2147483647;
const int32_t captureMoveScore = hashMoveScore >> 1;
const std::array<int32_t, 1 + 4> killerMoveScore = {
    0, hashMoveScore >> 2, hashMoveScore >> 3, hashMoveScore >> 4, hashMoveScore >> 5
};

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

Search::Search(TranspositionTable& transpositionTable, PawnHashTable& pawnHashTable, KillerTable& killerTable, HistoryTable& historyTable) :
transpositionTable(transpositionTable), killerTable(killerTable), historyTable(historyTable), evaluation(pawnHashTable)
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
    nodesToTimeCheck = 10000;
    selDepth = 1;
    lastRootScore = currentRootScore = -mateScore;

    for (auto i = 0; i < 256; ++i)
    {
        lmrReductions[i] = static_cast<int>(std::max(1.0, std::round(std::log(i + 1))));
    }
}

void orderCaptures(const Position& pos, MoveList& moveList)
{
    for (auto i = 0; i < moveList.size(); ++i)
    {
        moveList[i].setScore(pos.SEE(moveList[i]));
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
void Search::orderMoves(const Position& pos, MoveList& moveList, const Move& ttMove, const int ply) const
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
            const auto killerScore = killerTable.isKiller(move, ply);
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

void Search::think(const Position& root, SearchParameters searchParameters, int newRootPly, std::array<HashKey, 1024> newRepetitionHashKeys)
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
    contempt[root.getSideToMove()] = 0; // -contemptValue;
    contempt[!root.getSideToMove()] = 0; // contemptValue;
    lastRootScore = -mateScore;
    selDepth = 1;
    searching = true;
    pondering = searchParameters.ponder;
    infinite = searchParameters.infinite;
    rootPly = newRootPly;
    repetitionHashes = newRepetitionHashKeys;
    transpositionTable.startNewSearch();
    transpositionTable.clear();
    historyTable.clear();
    // historyTable.age();
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
        targetTime = clamp(time / searchParameters.movesToGo + increment - lagBuffer, 1, time - lagBuffer);
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

    repetitionHashes[rootPly] = pos.getHashKey();
    for (auto depth = 1; depth < 128;)
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
                ++nodeCount;
                --nodesToTimeCheck;

                if (depth >= 12)
                {
                    infoCurrMove(move, depth, i);
                }

                auto givesCheck = pos.givesCheck(move);
                auto extension = givesCheck ? 1 : 0;
                auto newDepth = depth - 1 + extension;
                auto nonCriticalMove = !extension && move.getScore() >= 0 && move.getScore() < killerMoveScore[4];

                Position newPosition(pos);
                newPosition.makeMove(move);
                if (!movesSearched)
                {
                    score = -search<true>(newPosition, newDepth, 1, -beta, -alpha, true, givesCheck != 0);
                }
                else
                {
                    auto reduction = ((lmrNode && i >= lmrFullDepthMoves && nonCriticalMove)
                        ? lmrReductions[i - lmrFullDepthMoves] : 0);

                    score = -search<false>(newPosition, newDepth - reduction, 1, -alpha - 1, -alpha, true, givesCheck != 0);

                    if (reduction && score > alpha)
                    {
                        score = -search<false>(newPosition, newDepth, 1, -alpha - 1, -alpha, true, givesCheck != 0);
                    }

                    if (score > alpha && score < beta)
                    {
                        score = -search<true>(newPosition, newDepth, 1, -beta, -alpha, true, givesCheck != 0);
                    }
                }
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
                            break;
                        }
                        transpositionTable.save(pos.getHashKey(), 0, bestMove, realScoreToTtScore(currentRootScore, 0), depth, UpperBoundScore);
                        pv = transpositionTable.extractPv(pos);
                        infoPv(pv, depth, currentRootScore, ExactScore);
                    }
                }
            }
        }
        catch (const StopSearchException&)
        {
            pos = root; // Exception messes up the position, fix it.
        }

        transpositionTable.save(pos.getHashKey(), 0, bestMove, realScoreToTtScore(currentRootScore, 0), depth, currentRootScore >= beta ? LowerBoundScore : ExactScore);
        pv = transpositionTable.extractPv(pos);

        // If this is not an infinite search and the search has returned mate scores two times in a row stop searching.
        if (!infinite && isMateScore(currentRootScore) && isMateScore(lastRootScore) && depth > 6)
        {
            break;
        }

        if (!searching)
        {
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

int Search::quiescenceSearch(const Position& pos, const int depth, const int ply, int alpha, int beta, const bool inCheck)
{
    int bestScore, delta;
    bool zugzwangLikely;
    MoveList moveList;

    if (inCheck)
    {
        bestScore = matedInPly(ply);
        delta = -infinity;
        moveGen.generateLegalEvasions(pos, moveList);
        orderMoves(pos, moveList, Move(), ply); // TODO: some replacement for constructing a move.
    }
    else
    {
        bestScore = evaluation.evaluate(pos, zugzwangLikely);
        if (bestScore > alpha)
        {
            if (bestScore >= beta)
            {
                return bestScore;
            }
            alpha = bestScore;
        }
        delta = bestScore + futilityMargins[0];
        moveGen.generatePseudoLegalCaptures(pos, moveList);
        orderCaptures(pos, moveList);
    }

    for (auto i = 0; i < moveList.size(); ++i)
    {
        selectMove(moveList, i);
        const auto& move = moveList[i];
        const auto givesCheck = pos.givesCheck(move);
        ++nodeCount;
        --nodesToTimeCheck;

        // Add givesCheck != 2 condition here.
        // SEE pruning. If the move seems to lose material prune it.
        // This kind of pruning is too dangerous when in check so we don't use it then.
        if (!inCheck && move.getScore() < 0)
        {
            break;
        }

        // Delta pruning. If the move seems to have no chance of raising alpha prune it.
        // This too is too dangerous when we are in check.
        if (!inCheck && delta + move.getScore() <= alpha)
        {
            bestScore = std::max(bestScore, delta + move.getScore());
            break;
        }

        if (!pos.legal(move, inCheck))
        {
            continue;
        }

        Position newPosition(pos);
        newPosition.makeMove(move);

        const auto score = -quiescenceSearch(newPosition, depth - 1, ply + 1, -beta, -alpha, givesCheck != 0);

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
int Search::search(const Position& pos, int depth, int ply, int alpha, int beta, bool allowNullMove, bool inCheck)
{
    assert(alpha < beta);

    auto bestScore = matedInPly(ply), movesSearched = 0, prunedMoves = 0;
    auto ttFlag = UpperBoundScore;
    auto zugzwangLikely = false; // Initialization needed only to shut up warnings.
    auto mateThreat = false;
    MoveList moveList;
    Move ttMove, bestMove;
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
            moveGen.generateLegalEvasions(pos, moveList);
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
        return quiescenceSearch(pos, 0, ply, alpha, beta, inCheck);
    }

    // Probe the transposition table. 
    auto ttEntry = transpositionTable.probe(pos.getHashKey());
    if (ttEntry)
    {
        ttMove.setMove(ttEntry->getBestMove());
        if (ttEntry->getDepth() >= depth)
        {
            auto ttScore = ttScoreToRealScore(ttEntry->getScore(), ply);
            auto ttFlags = ttEntry->getFlags();

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

    // Get the static evaluation of the position. Not needed in nodes where we are in check.
    auto staticEval = (inCheck ? -infinity : evaluation.evaluate(pos, zugzwangLikely));

    // Reverse futility pruning / static null move pruning.
    // Not useful in PV-nodes as this tries to search for nodes where score >= beta but in PV-nodes score < beta.
    if (!pvNode && !inCheck && !zugzwangLikely && depth <= reverseFutilityDepth && staticEval - reverseFutilityMargins[depth] >= beta)
        return staticEval - reverseFutilityMargins[depth];

    // Razoring.
    // Not useful in PV-nodes as this tries to search for nodes where score <= alpha but in PV-nodes score > alpha.
    if (!pvNode && !inCheck && depth <= razoringDepth && staticEval <= alpha - razoringMargins[depth])
    {
        auto razoringAlpha = alpha - razoringMargins[depth];
        score = quiescenceSearch(pos, 0, ply, razoringAlpha, razoringAlpha + 1, false);
        if (score <= razoringAlpha)
        {
            return score;
        }
    }

    // Null move pruning.
    // Not used when in a PV-node because we should _never_ fail high at a PV-node so doing this is a waste of time.
    // I don't really like the staticEval >= beta condition but the gain in elo is significant so...
    if (!pvNode && allowNullMove && !inCheck && staticEval >= beta && !zugzwangLikely)
    {
        if (!(ttEntry
            && ttEntry->getFlags() == UpperBoundScore
            && ttEntry->getDepth() >= depth - 1 - nullReduction
            && ttEntry->getScore() <= alpha))
        {
            repetitionHashes[rootPly + ply] = pos.getHashKey();
            Position newPosition(pos);
            newPosition.makeNullMove();
            ++nodeCount;
            --nodesToTimeCheck;
            score = -search<false>(newPosition, depth - 1 - nullReduction, ply + 1, -beta, -beta + 1, false, false);
            if (score >= beta)
            {
                // Don't return unproven mate scores as they cause some instability.
                if (isMateScore(score))
                    score = beta;
                transpositionTable.save(pos.getHashKey(), ply, ttMove, realScoreToTtScore(score, ply), depth, LowerBoundScore);
                return score;
            }
            else if (score == matedInPly(ply + 2))
            {
                mateThreat = true;
            }
        }
    }

    // Internal iterative deepening.
    // Only done at PV-nodes due to the cost involved.
    if (pvNode && ttMove.empty() && depth > 4)
    {
        // We can skip nullmove in IID since if it would have worked we wouldn't be here.
        score = search<true>(pos, depth - 2, ply, alpha, beta, false, inCheck);

        // Now probe the TT and get the best move.
        auto tte = transpositionTable.probe(pos.getHashKey());
        if (tte)
        {
            ttMove.setMove(tte->getBestMove());
        }
    }

    // Generate moves and order them. In nodes where we are in check we use a special evasion move generator.
    inCheck ? moveGen.generateLegalEvasions(pos, moveList) : moveGen.generatePseudoLegalMoves(pos, moveList);
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

        auto givesCheck = pos.givesCheck(move);
        auto extension = (givesCheck || mateThreat || oneReply) ? 1 : 0;
        auto newDepth = depth - 1 + extension;
        auto nonCriticalMove = !extension && move.getScore() >= 0 && move.getScore() < killerMoveScore[4];
        ++nodeCount;
        --nodesToTimeCheck;

        // Futility pruning and late move pruning.
        if (nonCriticalMove && !isLoseScore(bestScore))
        {
            if (futileNode)
            {
                ++prunedMoves;
                continue;
            }

            if (lmpNode && i >= lmpMoveCount[depth])
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
        if (!movesSearched)
        {
            score = -search<pvNode>(newPosition, newDepth, ply + 1, -beta, -alpha, true, givesCheck != 0);
        }
        else
        {
            auto reduction = ((lmrNode && i >= lmrFullDepthMoves && nonCriticalMove)
                ? lmrReductions[i - lmrFullDepthMoves] : 0);

            score = -search<false>(newPosition, newDepth - reduction, ply + 1, -alpha - 1, -alpha, true, givesCheck != 0);

            // The LMR'd move didn't fail low, drop the reduction because that most likely caused the fail high.
            // If we are in a PV-node the alternative is to open the window first. The more unstable the search the better doing that is.
            // Before the tuned evaluation opening the window was better, after the tuned eval it is worse. Why?
            if (reduction && score > alpha)
            {
                score = -search<false>(newPosition, newDepth, ply + 1, -alpha - 1, -alpha, true, givesCheck != 0);
            }

            // If we are in a PV-node this is used to get the exact score for a new PV.
            // Since we used null window on the previous searches the score is only a bound, and this won't do for a PV.
            if (score > alpha && score < beta)
            {
                score = -search<true>(newPosition, newDepth, ply + 1, -beta, -alpha, true, givesCheck != 0);
            }
        }
        ++movesSearched;

        if (score > bestScore)
        {
            if (score > alpha)
            {
                if (score >= beta)
                {
                    transpositionTable.save(pos.getHashKey(), ply, move, realScoreToTtScore(score, ply), depth, LowerBoundScore);

                    if (!inCheck)
                    {
                        if (quietMove(pos, move))
                        {
                            historyTable.addCutoff(pos, move, depth);
                            killerTable.addKiller(move, ply);
                        }
                        for (auto j = 0; j < i; ++j)
                        {
                            const auto& move2 = moveList[j];
                            if (quietMove(pos, move2))
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

    transpositionTable.save(pos.getHashKey(), ply, bestMove, realScoreToTtScore(bestScore, ply), depth, ttFlag);

    return bestScore;
}
