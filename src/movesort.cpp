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
#include "movegen.hpp"

// In all cases, search the TT-move first.
// In a normal search, then search good captures, then killers, then quiet moves and finally bad captures.
enum Phase {
    Normal, GoodCaptures, Killers, QuietMoves, BadCaptures,
    Evasion, Evasions,
    Stop
};

MoveSort::MoveSort(const Position& pos, const HistoryTable& historyTable, Move ttMove, Move k1, Move k2, Move counter, bool inCheck) :
mPos(pos), mHistoryTable(historyTable), mTtMove(ttMove), mKiller1(k1), mKiller2(k2), mCounter(counter)
{
    mPhase = inCheck ? Evasion : Normal;
    mCurrentLocation = 0;
    if (pos.pseudoLegal(ttMove, inCheck))
    {
        mMoveList.resize(1);
    }
}

void MoveSort::generateNextPhase()
{
    ++mPhase;
    mCurrentLocation = 0;
    mMoveList.clear();

    if (mPhase == GoodCaptures)
    {
        MoveGen::generatePseudoLegalCaptures(mPos, mMoveList, true);
        for (auto i = 0; i < mMoveList.size(); ++i)
        {
            mMoveList.setScore(i, mPos.mvvLva(mMoveList.getMove(i)));
        }
    }
    else if (mPhase == Killers)
    {
        mMoveList.emplace_back(mKiller1);
        mMoveList.emplace_back(mKiller2);
        if (mCounter != mKiller1 && mCounter != mKiller2)
        {
            mMoveList.emplace_back(mCounter);
        }
    }
    else if (mPhase == QuietMoves)
    {
        MoveGen::generatePseudoLegalQuietMoves(mPos, mMoveList);
        for (auto i = 0; i < mMoveList.size(); ++i)
        {
            mMoveList.setScore(i, mHistoryTable.getScore(mPos, mMoveList.getMove(i)));
        }
    }
    else if (mPhase == BadCaptures)
    {
        mMoveList = mTemp;
    }
    else if (mPhase == Evasions)
    {
        MoveGen::generateLegalEvasions(mPos, mMoveList);
        if (mMoveList.size() > 1) scoreEvasions();
    }
    else if (mPhase == Normal || mPhase == Evasion || mPhase == Stop)
    {
        mPhase = Stop;
        mMoveList.resize(mCurrentLocation + 1);
    }
}

Move MoveSort::next()
{
    for (;;)
    {
        while (mCurrentLocation == mMoveList.size())
        {
            generateNextPhase();
        }

        if (mPhase == Normal || mPhase == Evasion)
        {
            ++mCurrentLocation;
            return mTtMove;
        }
        else if (mPhase == GoodCaptures)
        {
            selectionSort(mCurrentLocation);
            const auto move = mMoveList.getMove(mCurrentLocation++);
            if (move != mTtMove)
            {
                if (mPos.SEE(move) >= 0)
                {
                    return move;
                }
                // Put bad captures into a temporary movelist.
                mTemp.emplace_back(move);
            }
        }
        else if (mPhase == Killers)
        {
            const auto move = mMoveList.getMove(mCurrentLocation++);
            if (!move.empty() && move != mTtMove && !mPos.captureOrPromotion(move) && mPos.pseudoLegal(move, false))
            {
                return move;
            }
        }
        else if (mPhase == QuietMoves)
        {
            selectionSort(mCurrentLocation);
            const auto move = mMoveList.getMove(mCurrentLocation++);
            // Filter out TT-move, killer moves and countermoves as those have already been tried.
            if (move != mTtMove && move != mKiller1 && move != mKiller2 && move != mCounter)
            {
                return move;
            }
        }
        else if (mPhase == BadCaptures)
        {
            // We already sorted the bad captures above and checked that they do not equal the TT-move.
            return mMoveList.getMove(mCurrentLocation++);
        }
        else if (mPhase == Evasions)
        {
            selectionSort(mCurrentLocation);
            const auto move = mMoveList.getMove(mCurrentLocation++);
            // Filter out TT-move as it has been tried already.
            if (move != mTtMove)
            {
                return move;
            }
        }
        else if (mPhase == Stop)
        {
            return Move();
        }
    }
}

void MoveSort::scoreEvasions()
{
    static const int16_t hashMoveScore = 30000;
    static const int16_t captureMoveScore = hashMoveScore >> 1;

    for (auto i = 0; i < mMoveList.size(); ++i)
    {
        const auto move = mMoveList.getMove(i);

        if (mPos.captureOrPromotion(move))
        {
            auto score = mPos.SEE(move);
            if (score >= 0) // Order good captures and promotions after ttMove
            {
                score += captureMoveScore;
            }
            mMoveList.setScore(i, score);
        }
        else
        {
            mMoveList.setScore(i, mHistoryTable.getScore(mPos, move));
        }
    }
}

void MoveSort::selectionSort(int startingLocation)
{
    auto bestLocation = startingLocation;
    auto bestScore = mMoveList.getScore(startingLocation);

    for (auto i = startingLocation + 1; i < mMoveList.size(); ++i)
    {
        if (mMoveList.getScore(i) > bestScore)
        {
            bestScore = mMoveList.getScore(i);
            bestLocation = i;
        }
    }

    if (bestLocation > startingLocation)
    {
        // Swap the values at bestLocation and startingLocation.
        const auto m = mMoveList.getMove(startingLocation);
        const auto s = mMoveList.getScore(startingLocation);
        mMoveList.setMove(startingLocation, mMoveList.getMove(bestLocation));
        mMoveList.setScore(startingLocation, mMoveList.getScore(bestLocation));
        mMoveList.setMove(bestLocation, m);
        mMoveList.setScore(bestLocation, s);
    }
}




