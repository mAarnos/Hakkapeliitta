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

#ifndef PHT_HPP_
#define PHT_HPP_

#include <cstdint>
#include <vector>
#include "position.hpp"

// Hash table for speeding up pawn evaluation.
// Default size of the pawn hash table is 4MB.
class PawnHashTable
{
public:
    PawnHashTable();

    // Sets the size of the pawn hash table.
	void setSize(size_t sizeInMegaBytes);
    // Clears the pawn hash table. Can potentially be an expensive operation.
    void clear();
    // Save some information to the pawn hash table.
    void save(HashKey hk, int scoreOp, int scoreEd);
    // Get some information from the hash table. Returns true on a successful probe.
    bool probe(HashKey hk, int& scoreOp, int& scoreEd) const;
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

        void setHash(const uint64_t newHash) { hash = newHash; }
        void setData(const uint64_t newData) { data = newData; }

        uint64_t getHash() const { return hash; }
        uint64_t getData() const { return data; }
    private:
        uint64_t hash;
        uint64_t data;
    };

    std::vector<PawnHashTableEntry> table;
};

#endif
