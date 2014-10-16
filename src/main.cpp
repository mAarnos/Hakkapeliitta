#include <iostream>
#include "uci.hpp"
#include "benchmark.hpp"
#include "tuning\tuning.hpp"
#include "utils\threadpool.hpp"
#include "eval.hpp"
#include "search.hpp"

int main() 
{
    std::cout << "Hakkapeliitta 2.0 alpha, (C) 2013-2014 Mikko Aarnos" << std::endl;

    Bitboards::initialize();
    Zobrist::initialize();
    Evaluation::initialize();
    Search::initialize();
    UCI uci;
    // ThreadPool pool(1);

    // Benchmark::runPerft();

    //Tuning tuning;
    // tuning.tune();

    uci.mainLoop();

    return 0;
}