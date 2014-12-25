#ifndef PHT_HPP_
#define PHT_HPP_

#include <cstdint>
#include <vector>
#include "position.hpp"

class PawnHashTable
{
public:
    PawnHashTable();

	void setSize(size_t sizeInMegaBytes);
    void clear();
    void save(const Position& pos, int scoreOp, int scoreEd);
    bool probe(const Position& pos, int& scoreOp, int& scoreEd) const;
private:
    // A single entry in the pawn hash table. Unlike with the tt, we have no need for buckets here.
    // Notice that the size of the hash is smaller than with TTEntry. 
    // This is simply because pawn hash collisions are very rare even with 32 bits so we don't need to have a long hash.
    // Also, here collisions are far less dangerous than when with tt. 
    // With the pawn hash table if something happens we just get a false score. With the tt we might get an illegal move which crashes the engine.
    class PawnHashTableEntry
    {
    public:
        PawnHashTableEntry() : hash(0), data(0) {}

        void setHash(uint32_t newHash) { hash = newHash; }
        void setData(uint32_t newData) { data = newData; }

        uint32_t getHash() const { return hash; }
        uint32_t getData() const { return data; }
    private:
        uint32_t hash;
        uint32_t data;
    };

    std::vector<PawnHashTableEntry> table;
};

#endif
