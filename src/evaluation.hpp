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

/// @file evaluation.hpp
/// @author Mikko Aarnos

#ifndef EVALUATION_HPP_
#define EVALUATION_HPP_

#include <array>
#include "position.hpp"
#include "zobrist.hpp"
#include "endgame.hpp"
#include "pht.hpp"

/// @brief The evaluation function.
class Evaluation
{
public:
    /// @brief Initializes the class, must be called before using any other methods.
    static void staticInitialize();

    /// @brief Evaluates a given position.
    /// @param pos The position.
    /// @return The heuristic score given to the position.
    int evaluate(const Position& pos);

    /// @brief Clears the pawn hash table used by the evalation function.
    void clearPawnHashTable();

    /// @brief Sets the size of the pawn hash table used by the evaluation function.
    /// @param sizeInMegaBytes The new size in megabytes.
    void setPawnHashTableSize(size_t sizeInMegaBytes);

    /// @brief Get the opening PST score of a given piece on a given square.
    /// @param p The piece.
    /// @param sq The square.
    /// @return The score.
    static short getPieceSquareTableOp(Piece p, Square sq);

    /// @brief Get the ending PST score of a given piece on a given square.
    /// @param p The piece.
    /// @param sq The square.
    /// @return The score.
    static short getPieceSquareTableEd(Piece p, Square sq);

private:
    EndgameModule mEndgameModule;
    PawnHashTable mPawnHashTable;

    // These two have to be annoyingly static, as we use them in position.cpp to incrementally update the PST eval.
    static std::array<std::array<short, 64>, 12> mPieceSquareTableOpening;
    static std::array<std::array<short, 64>, 12> mPieceSquareTableEnding;

    static std::array<std::array<int, 64>, 64> mChebyshevDistance;

    template <bool hardwarePopcnt> 
    int evaluate(const Position& pos);

    template <bool hardwarePopcnt> 
    int mobilityEval(const Position& pos, std::array<int, 2>& kingSafetyScore, int phase);

    int pawnStructureEval(const Position& pos, std::array<uint8_t, 2>& pawnShelterScore, int phase);
    // Static to get around a static analysis tool warning.
    static int kingSafetyEval(int phase, std::array<int, 2>& kingSafetyScore, std::array<uint8_t, 2>& pawnShelterScore);
};

inline void Evaluation::clearPawnHashTable()
{
    mPawnHashTable.clear();
}

inline void Evaluation::setPawnHashTableSize(size_t sizeInMegaBytes)
{
    mPawnHashTable.setSize(sizeInMegaBytes);
}

inline short Evaluation::getPieceSquareTableOp(Piece p, Square sq)
{
    return mPieceSquareTableOpening[p][sq];
}

inline short Evaluation::getPieceSquareTableEd(Piece p, Square sq)
{
    return mPieceSquareTableEnding[p][sq];
}

#endif