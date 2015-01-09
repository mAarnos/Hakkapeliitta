#ifndef BENCHMARK_HPP_
#define BENCHMARK_HPP_

#include <cstdint>
#include "position.hpp"

// No further improvement necessary.

class Benchmark
{
public:
    static void runPerft(Position& pos, int depth);
private:
    static uint64_t perft(const Position& pos, int depth);
};

#endif