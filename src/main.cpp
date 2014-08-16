#include <iostream>
#include "uci.hpp"
#include "benchmark.hpp"
#include "tuning.hpp"

int main() 
{
    std::cout << "Hakkapeliitta, (C) 2013-2014 Mikko Aarnos" << std::endl;

    Bitboards::initialize();
    Zobrist::initialize();

    UCI uci;

    // Benchmark::runPerft();

    Tuning tuning;

    uci.mainLoop();

    return 0;
}