#ifndef BENCHMARK_HPP_
#define BENCHMARK_HPP_

#include <cstdint>
#include "position.hpp"

class Benchmark
{
public:
    static void runPerft(const Position& root, int depth);
private:
    static uint64_t perft(const Position& pos, int depth);
};

#endif