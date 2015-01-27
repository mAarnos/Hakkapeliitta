#ifndef BENCHMARK_HPP_
#define BENCHMARK_HPP_

#include <cstdint>
#include "position.hpp"
#include "movegen.hpp"

class Benchmark
{
public:
    // Run perft to depth given on the position given.
    // Returns a pair of the perft result and the time it took to calculate it, in ms.
    std::pair<uint64_t, uint64_t> runPerft(Position root, int depth);
private:
    MoveGen moveGen;
    uint64_t perft(const Position& pos, int depth);
};

#endif