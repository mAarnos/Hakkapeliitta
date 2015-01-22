#ifndef BENCHMARK_HPP_
#define BENCHMARK_HPP_

#include <cstdint>
#include "position.hpp"

class Benchmark
{
public:
    void runPerft(Position root, int depth);
private:
    uint64_t perft(const Position& pos, int depth);
};

#endif