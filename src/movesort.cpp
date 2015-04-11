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

#include "movesort.hpp"

MoveGen MoveSort::moveGen;

enum Phase {
    Normal, Captures, Killers, QuietMoves, BadCaptures,
    Evasion, Evasions,
    Stop
};

MoveSort::MoveSort(const Position& pos, const HistoryTable& historyTable, uint16_t ttm, uint16_t k1, uint16_t k2, uint16_t counter, bool inCheck) :
pos(pos), historyTable(historyTable), ttMove(ttm, 0), k1(k1), k2(k2), counter(counter)
{
    phase = inCheck ? Evasion : Normal;
    currentLocation = 0;
    badCapturesLocation = 217;
    if (!pos.pseudoLegal(ttMove, inCheck))
    {
        ttMove.setMove(0);
    }
    if (!ttMove.empty()) moveList.resize(1);
}

void MoveSort::generateNextPhase()
{
    ++phase;
    if (phase == Captures)
    {
        moveGen.generatePseudoLegalCaptures(pos, moveList, true);
        for (auto i = currentLocation; i < moveList.size(); ++i)
        {
            moveList[i].setScore(pos.SEE(moveList[i]));
        }
    }
    else if (phase == Killers)
    {
        moveList.emplace_back(k1);
        moveList.emplace_back(k2);
        if (counter != k1 && counter != k2)
        {
            moveList.emplace_back(counter);
        }
    }
    else if (phase == QuietMoves)
    {
        moveGen.generatePseudoLegalQuietMoves(pos, moveList);
        for (auto i = currentLocation; i < moveList.size(); ++i)
        {
            moveList[i].setScore(historyTable.getScore(pos, moveList[i]));
        }
    }
    else if (phase == BadCaptures)
    {
        moveList.resize(badCapturesLocation);
        currentLocation = 217;
    }
    else if (phase == Evasions)
    {
        moveGen.generateLegalEvasions(pos, moveList);
        if (moveList.size() > 1) scoreEvasions();
    }
    else if (phase == Normal || phase == Evasion || phase == Stop)
    {
        phase = Stop;
        moveList.resize(currentLocation + 1);
    }
}

Move MoveSort::next()
{
    for (;;)
    {
        while (currentLocation == moveList.size())
        {
            generateNextPhase();
        }

        if (phase == Normal || phase == Evasion)
        {
            ++currentLocation;
            return ttMove;
        }
        else if (phase == Captures)
        {
            const auto move = selectionSort(currentLocation++);
            if (move.getMove() != ttMove.getMove())
            {
                if (move.getScore() >= 0)
                {
                    return move;
                }
                // A losing capture, move it to the end of the movelist.
                // Since we have a bit of buffer at the end this should be safe.
                moveList[--badCapturesLocation] = move;
            }
        }
        else if (phase == Killers)
        {
            const auto& move = moveList[currentLocation++];
            if (!move.empty() && move.getMove() != ttMove.getMove() && !pos.captureOrPromotion(move) && pos.pseudoLegal(move, false))
            {
                return move;
            }
        }
        else if (phase == QuietMoves)
        {
            const auto move = selectionSort(currentLocation++);
            // Filter out TT-move, killer moves and countermoves as those have already been tried.
            if (move.getMove() != ttMove.getMove() && move.getMove() != k1 && move.getMove() != k2 && move.getMove() != counter)
            {
                return move;
            }
        }
        else if (phase == BadCaptures)
        {
            // We already sorted the bad captures above and so the last bad capture is the best.
            return moveList[--currentLocation];
        }
        else if (phase == Evasions)
        {
            const auto move = selectionSort(currentLocation++);
            // Filter out TT-move as it has been tried already.
            if (move.getMove() != ttMove.getMove())
            {
                return move;
            }
        }
        else if (phase == Stop)
        {
            return Move();
        }
    }
}

void MoveSort::scoreEvasions()
{
    static const int16_t hashMoveScore = 30000;
    static const int16_t captureMoveScore = hashMoveScore >> 1;

    for (auto i = currentLocation; i < moveList.size(); ++i)
    {
        auto& move = moveList[i];

        if (pos.getBoard(move.getTo()) != Piece::Empty || (move.getPromotion() != Piece::Empty && move.getPromotion() != Piece::King))
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
            move.setScore(historyTable.getScore(pos, move));
        }
    }
}

Move MoveSort::selectionSort(int startingLocation)
{
    auto bestMove = startingLocation;
    auto bestScore = moveList[startingLocation].getScore();

    for (auto i = startingLocation + 1; i < moveList.size(); ++i)
    {
        if (moveList[i].getScore() > bestScore)
        {
            bestScore = moveList[i].getScore();
            bestMove = i;
        }
    }

    if (bestMove > startingLocation)
    {
        std::swap(moveList[startingLocation], moveList[bestMove]);
    }

    return moveList[startingLocation];
}




