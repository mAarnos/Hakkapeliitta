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

/// @file tt.hpp
/// @author Mikko Aarnos

#ifndef TT_HPP_
#define TT_HPP_

#include <cstdint>
#include <array>
#include <vector>
#include "move.hpp"
#include "zobrist.hpp"

/// @brief Transposition table used for storing previous results of the search function.
///
/// Default size of the transposition table is 32MB.
class TranspositionTable
{
public:
    /// @brief Flags for different types of TT entries.
    /// Empty means that the entry is not usable as nothing has been stored there.
    /// ExactScore means that the real score is exactly what is in the entry.
    /// UpperBoundScore means that the real score is at most the score in the entry.
    /// LowerBoundScore means the the real score is at least the score in the entry.
    enum Flags
    {
        Empty = 0, ExactScore = 1, UpperBoundScore = 2, LowerBoundScore = 3
    };

    /// @brief A single entry in the transposition table.
    ///
    /// Contains the best move, score, generation, depth and flags for a single position encountered in the search.
    class TranspositionTableEntry
    {
    public:
        /// @brief Default constructor.
        TranspositionTableEntry() noexcept : mHash(0), mData(0) 
        {
        }

        /// @brief Set the hash key of this TT entry.
        void setHash(uint64_t newHash) noexcept 
        { 
            mHash = newHash; 
        }

        /// @brief Set the data of this TT entry. This data should be in a packed format.
        void setData(uint64_t newData) noexcept 
        {
            mData = newData; 
        }

        /// @brief Get the hash key of this TT entry.
        /// @return The hash key.
        uint64_t getHash() const noexcept 
        {
            return mHash; 
        }

        /// @brief Get the packed data of this TT entry. Not really used as is except for validation of TT entry integrity.
        /// @return The packed data.
        uint64_t getData() const noexcept 
        { 
            return mData; 
        }

        /// @brief Get the saved best move of this TT entry.
        /// @return The best move. Note that ALL-nodes have no best move.
        Move getBestMove() const noexcept 
        { 
            return static_cast<uint16_t>(mData); 
        }

        /// @brief Get the generation of this TT entry. Used for TT replacement policy.
        /// @return The generation.
        uint16_t getGeneration() const noexcept 
        { 
            return static_cast<uint16_t>(mData >> 16); 
        }

        /// @brief Get the score of this TT entry.
        /// @return The score. Mate scores need to be adjusted.
        int16_t getScore() const noexcept 
        { 
            return static_cast<int16_t>(mData >> 32); 
        }

        /// @brief Get the depth of this TT entry.
        /// @return The depth.
        uint8_t getDepth() const noexcept 
        { 
            return static_cast<uint8_t>(mData >> 48);
        }

        /// @brief Get the flags of this TT entry. 
        /// @return The flags. 
        uint8_t getFlags() const noexcept 
        {
            return mData >> 56; 
        };

    private:
        uint64_t mHash;
        uint64_t mData; // 16 bits for the best move, 16 bits for the generation (only 4 or so are actually necessary), 16 bits for the score, 8 bits for the depth and 8 bits for the flags (only 2 bits necessary).
    };

    /// @brief Default constructor.
    TranspositionTable();

    /// @brief Save some information to the transposition table.
    /// @param hk The hash key for the position the information is for.
    /// @param move The best move in the position. Note that ALL-nodes have no best move by definition.
    /// @param score The score of the position.
    /// @param depth The depth the position was searched to.
    /// @param flags Flags indicating whether the position is a PV, CUT, or an ALL node.
    void save(HashKey hk, const Move& move, int score, int depth, int flags);

    /// @brief Get the transposition table entry for a given hash key.
    /// @param hk The hash key for the position we want the entry for.
    /// @return A valid const pointer to the entry if the probe is succesful, a nullptr otherwise. 
    const TranspositionTableEntry* probe(HashKey hk) const;

    /// @brief Load a part of the transposition table into L1/L2 cache. Used as a speed optimization.
    /// @param hk The hash key for the part of the transposition table we want to load to the cache.
    void prefetch(HashKey hk) const;

    /// @brief Sets the size of the transposition table.
    /// @param sizeInMegaBytes Obviously, the new size of the hash table in megabytes.
    void setSize(size_t sizeInMegaBytes);

    /// @brief Clears the transposition table. Can potentially be an expensive operation.
    void clear();

    /// @brief Used for notifying the TT that we are starting a new search. That information is used in the replacement policy.
    void startNewSearch() noexcept;

private:
    // Notice how we have a bucket containing four hash entries.
    // Doing this has some desirable properties when deciding what entries to overwrite.
    // Also, we have four entries because the common cache line size nowadays is 64 bytes.
    // uint64_t hash * 4 + uint64_t data * 4 = 8 * uint64_t = 64 bytes.
    // Basically this means that a single cluster fits perfectly into the cacheline.
    std::vector<std::array<TranspositionTableEntry, 4>> mTable;
    uint16_t mGeneration;
};

#endif
