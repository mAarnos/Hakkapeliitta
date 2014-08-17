#include <iostream>
#include "uci.hpp"
#include "benchmark.hpp"
#include "tuning.hpp"
#include "threadpool.hpp"
#include "eval.hpp"

int main() 
{
    std::cout << "Hakkapeliitta, (C) 2013-2014 Mikko Aarnos" << std::endl;

    Bitboards::initialize();
    Zobrist::initialize();
    Evaluation::initialize();

    UCI uci;

    // Benchmark::runPerft();

    Tuning tuning;
    tuning.tune();

    // ThreadPool pool(1);

    uci.mainLoop();

    return 0;
}