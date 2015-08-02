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
#pragma pack (push, 1)
    class TranspositionTableEntry
    {
    public:
        /// @brief Default constructor.
        TranspositionTableEntry() noexcept : mHash(0), mScore(0), mDepth(0), mFlagsAndGeneration(0)
        {
        }

        /// @brief Set the hash key of this TT entry.
        /// @param newHash The new hash key.
        void setHash(uint32_t newHash) noexcept 
        { 
            mHash = newHash; 
        }

        /// @brief Set the best move of this TT entry.
        /// @param move The new move.
        void setBestMove(Move move) noexcept
        {
            mMove = move;
        }

        /// @brief Set the score of this TT entry.
        /// @param score The new score.
        void setScore(int16_t score) noexcept
        {
            mScore = score;
        }

        /// @brief Set the depth of this TT entry.
        /// @param depth The new depth.
        void setDepth(int8_t depth) noexcept
        {
            mDepth = depth;
        }

        /// @brief Set the flags and generation of this hash entry.
        /// @param flagsAndGeneration The new flags and generation.
        void setFlagsAndGeneration(uint8_t flagsAndGeneration) noexcept
        {
            mFlagsAndGeneration = flagsAndGeneration;
        }

        /// @brief Get the hash key of this TT entry.
        /// @return The hash key.
        uint64_t getHash() const noexcept 
        {
            return mHash; 
        }

        /// @brief Get the saved best move of this TT entry.
        /// @return The best move. Note that ALL-nodes have no best move.
        Move getBestMove() const noexcept 
        { 
            return mMove; 
        }

        /// @brief Get the score of this TT entry.
        /// @return The score. Mate scores need to be adjusted.
        int16_t getScore() const noexcept 
        { 
            return mScore; 
        }

        /// @brief Get the depth of this TT entry.
        /// @return The depth.
        int8_t getDepth() const noexcept 
        { 
            return mDepth;
        }

        /// @brief Get the flags of this TT entry. 
        /// @return The flags. 
        uint8_t getFlags() const noexcept 
        {
            return mFlagsAndGeneration & 3; 
        };

        /// @brief Get the generation of this TT entry. Used for TT replacement policy.
        /// @return The generation.
        uint8_t getGeneration() const noexcept
        {
            return (mFlagsAndGeneration & 252) >> 2;
        }

    private:
        uint32_t mHash;
        Move mMove;
        int16_t mScore;
        int8_t mDepth;
        uint8_t mFlagsAndGeneration; // 2 first bits are flags, the rest are the generation
    };
#pragma pack (pop)

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

    /// @brief Used for approximately measuring how full the TT is. 
    /// @return An integer in the range [0, 1000], where 0 is completely empty and 1000 completely full.
    int hashFull() const noexcept;

private:
    static const auto bucketSize = 6;
    static const auto cacheLineSize = 64;

    struct Cluster
    {
        std::array<TranspositionTableEntry, bucketSize> mEntries;
        std::array<char, 4> mPadding; // We could put a spinlock here.
    };

    std::vector<Cluster> mTable;
    uint8_t mGeneration;

    static_assert(sizeof(Cluster) == cacheLineSize, "Cluster size is wrong");
};

#endif
