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

#include <cstdlib>
#include <cstdint>
#include <vector>
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
    /// @param scoreOp The opening pawn score of the position.
    /// @param scoreEd The ending pawn score of the position.
    void save(HashKey phk, int scoreOp, int scoreEd);

    /// @brief Get some information from the hash table. 
    /// @param phk The pawn hash key of the position we are probing information for.
    /// @param scoreOp On a succesful probe the opening pawn score is put here.
    /// @param scoreEd On a succesful probe the ending pawn score is put here.
    /// @return True on a succesful probe, false otherwise.
    bool probe(HashKey phk, int& scoreOp, int& scoreEd) const;

private:
    // A single entry in the pawn hash table.
    class PawnHashTableEntry
    {
    public:
        PawnHashTableEntry() noexcept : mHash(0), mData(0) 
        {
        }

        void setHash(uint64_t newHash) noexcept 
        {
            mHash = newHash; 
        }

        void setData(uint64_t newData) noexcept 
        { 
            mData = newData; 
        }

        uint64_t getHash() const noexcept 
        { 
            return mHash; 
        }

        uint64_t getData() const noexcept 
        { 
            return mData; 
        }

        int16_t getScoreOp() const noexcept 
        {
            return static_cast<int16_t>(mData);
        }

        int16_t getScoreEd() const noexcept
        { 
            return static_cast<int16_t>(mData >> 16); 
        }

    private:
        uint64_t mHash;
        uint64_t mData;
    };

    std::vector<PawnHashTableEntry> mTable;
};

#endif
