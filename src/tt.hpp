#ifndef TT_HPP_
#define TT_HPP_

#include <cstdint>
#include <array>
#include <vector>
#include "move.hpp"
#include "zobrist.hpp"
#include "position.hpp"

// Flags for different types of TT entries.
// ExactScore means that the score is exactly what is in the entry.
// UpperBoundScore means that the score is at most the score in the entry.
// LowerBoundScore means the the score is at least the score in the entry.
enum TTFlags
{
    ExactScore = 1, UpperBoundScore = 2, LowerBoundScore = 4
};

// A single entry in the transposition table. 
// Contains the best move, score, generation, depth and flags for a single position encountered in the search.
class TranspositionTableEntry
{
public:
    TranspositionTableEntry() { hash = 0; data = 0; }

    void setHash(const uint64_t newHash) { hash = newHash; }
    void setData(const uint64_t newData) { data = newData; }
    uint64_t getHash() const { return hash; }
    uint64_t getData() const { return data; }
    uint16_t getBestMove() const { return static_cast<uint16_t>(data); }
    uint16_t getGeneration() const { return static_cast<uint16_t>(data >> 16); }
    int16_t getScore() const { return static_cast<int16_t>(data >> 32); }
    uint8_t getDepth() const { return static_cast<uint8_t>(data >> 48); }
    uint8_t getFlags() const { return data >> 56; };
private:
    uint64_t hash;
    uint64_t data;
};

class TranspositionTable
{
public:
    TranspositionTable();
    ~TranspositionTable();

    // Save some information to the transposition table.
    void save(HashKey hk, int ply, const Move& move, int score, int depth, int flags);
    // Get the entry for the given hash key.
    const TranspositionTableEntry* probe(HashKey hk) const;
    // Load a part of the transposition table into L1/L2 cache. Used as a speed optimization.
    void prefetch(HashKey hk);

	void setSize(size_t sizeInMegaBytes);
    void clear();
    void startNewSearch() { ++generation; }

    // This function is used for extracting the PV of the search out of the TT starting from the given position.
    std::vector<Move> extractPv(Position root) const;
private:
    // A bucket containing four hash entries.
    // Doing this has some desirable properties when deciding what entries to overwrite.
    // We have four entries because the common cache line size nowadays is 64 bytes.
    // uint64_t hash * 4 + uint64_t data * 4 = 8 * uint64_t = 64 bytes.
    // Basically this means that a single cluster fits perfectly into the cacheline.
    struct Cluster
    {
        TranspositionTableEntry entries[4];
    };

    Cluster* table;
    size_t tableSize;
    uint32_t generation;
};

#endif
