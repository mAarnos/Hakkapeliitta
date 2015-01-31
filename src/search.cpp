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
std::array<int, 256> lmrReductions;
const int lmpDepth = 4;
const std::array<int, 1 + 4> lmpMoveCount = {
    0, 4, 8, 16, 32
};
const int razoringDepth = 3;
const std::array<int, 1 + 3> razoringMargins = {
    0, 125, 300, 300
};

const int16_t hashMoveScore = 32767;
const int16_t captureMoveScore = hashMoveScore - 2000;
const std::array<int16_t, 1 + 4> killerMoveScore = {
    0, hashMoveScore - 4000, hashMoveScore - 4100, hashMoveScore - 4200, hashMoveScore - 4300
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

Search::Search(TranspositionTable& transpositionTable, PawnHashTable& pawnHashTable, KillerTable& killerTable, HistoryTable& historyTable) :
transpositionTable(transpositionTable), killerTable(killerTable), historyTable(historyTable), evaluation(pawnHashTable)
{
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
void Search::orderMoves(const Position& pos, MoveList& moveList, const Move& ttMove, const int ply)
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
        orderMoves(pos, moveList, Move(0, 0, 0, 0), ply); // TODO: some replacement for constructing a move.
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

        // Add givesCheck != 2 condition here.
        // SEE pruning. If the move seems to lose material prune it.
        // This kind of pruning is too dangerous when in check so we don't use it then.
        if (!inCheck && move.getScore() < 0)
        {
            break;
        }

        // Delta pruning. If the move seems to have no chance of raising alpha prune it.
        // This too is too dangerous when we are in check.
        /*
        if (!inCheck && delta + move.getScore() <= alpha)
        {
            bestScore = std::max(bestScore, delta + move.getScore());
            break;
        }
        */

        if (!pos.legal(move, inCheck))
        {
            continue;
        }

        Position newPosition(pos);
        newPosition.makeMove(move);
        // ++nodeCount;
        // --nodesToTimeCheck;

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

