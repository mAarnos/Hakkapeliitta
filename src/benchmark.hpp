#ifndef BENCHMARK_HPP_
#define BENCHMARK_HPP_

#include <cstdint>
#include "position.hpp"

class Benchmark
{
public:
    static void runPerft();
private:
    static uint64_t perft(Position& pos, int depth);
};

#endif