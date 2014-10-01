#include "search.hpp"
#include "eval.hpp"
#include "movegen.hpp"
#include <iostream>

const int Search::aspirationWindow = 50;
const int Search::nullReduction = 3;
const int Search::futilityDepth = 4;
const std::array<int, 1 + 4> Search::futilityMargins = {
    50, 125, 125, 300, 300
};
const int Search::lmrFullDepthMoves = 4;
const int Search::lmrReductionLimit = 3;

std::array<Move, 32> Search::pv[32];
std::array<int, 32> Search::pvLength;

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

int Search::qSearch(Position & pos, int ply, int alpha, int beta)
{
    pvLength[ply] = ply;

    auto score = Evaluation::evaluate(pos);
    if (score > alpha)
    {
        if (score >= beta)
        {
            return score;
        }
        alpha = score;
    }
    else if (score + 1000 < alpha)
    {
        // If we are doing so badly that even capturing a queen for free won't help just return alpha.
        return alpha;
    }
    auto bestScore = score;
    auto delta = score + futilityMargins[0];

    MoveList moveStack;
    History history;
    MoveGen::generatePseudoLegalCaptureMoves(pos, moveStack);
    // orderCaptures(pos, moveStack);

    for (auto i = 0; i < moveStack.size(); ++i)
    {
        // selectMove(moveStack, i);
        const auto & move = moveStack[i];

        // Bad capture pruning + delta pruning. Assumes that the moves are sorted from highest SEE value to lowest.
        if (move.getScore() < 0 || (delta + move.getScore() < alpha))
        {
            break;
        }

        if (!pos.makeMove(move, history))
        {
            continue;
        }

        score = -qSearch(pos, ply + 1, -beta, -alpha);
        pos.unmakeMove(move, history);

        if (score > bestScore)
        {
            bestScore = score;
            if (score > alpha)
            {
                if (score >= beta)
                {
                    return score;
                }
                alpha = score;
                pv[ply][ply] = move;
                for (auto i = ply + 1; i < pvLength[ply + 1]; ++i)
                {
                    pv[ply][i] = pv[ply + 1][i];
                }
                pvLength[ply] = pvLength[ply + 1];
            }
        }
    }

    return bestScore;
}



