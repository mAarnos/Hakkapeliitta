#ifndef TT_HPP_
#define TT_HPP_

#include <cstdint>
#include <array>
#include <vector>
#include "move.hpp"
#include "position.hpp"

class TranspositionTable
{
public:
    TranspositionTable();

    void save(const Position & pos, int ply, const Move & move, int score, int depth, int flags);
    bool probe(const Position & pos, int ply, Move & move, int & score, int depth, int & alpha, int & beta) const;
    void prefetch(HashKey hk);

    void setSize(int sizeInMegaBytes);
    void clear();
    void startNewSearch() { ++generation; }
private:
    // A single entry in the transposition table. In actuality it is a bucket containing four entries and not a single entry.
    // Doing this has some desirable properties when deciding what entries to overwrite.
    // We have four entries because the common cache line size nowadays is 64 bytes.
    // uint64_t hash[4] + uint64_t data[4] = 8*uint64_t = 64 bytes.
    // Basically this means that a single tt entry fits perfectly into the cacheline.
    class TranspositionTableEntry
    {
    public:
        void setHash(int entry, uint64_t newHash) { hash[entry] = newHash; }
        void setData(int entry, uint64_t newData) { data[entry] = newData; }

        uint64_t getHash(int entry) const { return hash[entry]; }
        uint64_t getData(int entry) const { return data[entry]; }
        uint16_t getBestMove(int entry) const { return static_cast<uint16_t>(data[entry]); }
        uint16_t getGeneration(int entry) const { return static_cast<uint16_t>(data[entry] >> 16); }
        int16_t getScore(int entry) const { return static_cast<int16_t>(data[entry] >> 32); }
        uint8_t getDepth(int entry) const { return static_cast<uint8_t>(data[entry] >> 48); }
        uint8_t getFlags(int entry) const { return data[entry] >> 56; };
    private:
        std::array<uint64_t, 4> hash;
        std::array<uint64_t, 4> data;
    };

    enum {
        ExactScore, UpperBoundScore, LowerBoundScore
    };

    std::vector<TranspositionTableEntry> table;
    uint32_t generation;
};

#endif
