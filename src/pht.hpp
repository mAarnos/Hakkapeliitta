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

/// @file pht.hpp
/// @author Mikko Aarnos

#ifndef PHT_HPP_
#define PHT_HPP_

#include <cstdint>
#include <vector>
#include "bitboards.hpp"
#include "zobrist.hpp"

/// @brief Hash table for speeding up pawn evaluation.
///
/// Default size of the pawn hash table is 4MB.
class PawnHashTable
{
public:
    /// @brief Default constructor.
    PawnHashTable();

    /// @brief Sets the size of the pawn hash table.
    /// @param sizeInMegaBytes Obviously, the new size of the hash table in megabytes.
    void setSize(size_t sizeInMegaBytes);

    /// @brief Clears the pawn hash table. Can potentially be an expensive operation.
    void clear();

    /// @brief Save some information to the pawn hash table.
    /// @param phk The pawn hash key of the position the information is for.
    /// @param passers The passed pawns of the position.
    /// @param scoreOp The opening pawn score of the position.
    /// @param scoreEd The ending pawn score of the position.
    /// @param pawnShieldScore The penalties for messed up pawn shelters for both sides.
    void save(HashKey phk, Bitboard passers, int scoreOp, int scoreEd, const std::array<uint8_t, 2>& pawnShieldScore);

    /// @brief Get some information from the hash table. 
    /// @param phk The pawn hash key of the position we are probing information for.
    /// @param passers On a succesful probe the bitboard containing passed pawns of the position is put here.
    /// @param scoreOp On a succesful probe the opening pawn score is put here.
    /// @param scoreEd On a succesful probe the ending pawn score is put here.
    /// @param pawnShieldScore On a succsful probe the penalties for bad pawn shelters will be put here.
    /// @return True on a succesful probe, false otherwise.
    bool probe(HashKey phk, Bitboard& passers, int& scoreOp, int& scoreEd, std::array<uint8_t, 2>& pawnShieldScore) const;

private:
    // A single entry in the pawn hash table.
    class PawnHashTableEntry
    {
    public:
        // Initializing the hash key to 1 is pretty important, as the pawn hash key of a position without pawns is 0.
        PawnHashTableEntry() noexcept : mHash(1), mPassers(0), mScoreOp(0), mScoreEd(0), mPawnShieldScore({ 0, 0 })
        {
        }

        HashKey mHash;
        Bitboard mPassers;
        int16_t mScoreOp;
        int16_t mScoreEd;
        std::array<uint8_t, 2> mPawnShieldScore;
    };

    std::vector<PawnHashTableEntry> mTable;
};

#endif
