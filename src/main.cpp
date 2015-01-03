#include <iostream>
#include <thread>
#include "uci.hpp"
#include "benchmark.hpp"
#include "eval.hpp"
#include "search.hpp"
#include "utils/synchronized_ostream.hpp"
#include "utils/large_pages.hpp"

int main() 
{
    sync_cout << "Hakkapeliitta dev 030115 2, (C) 2013-2015 Mikko Aarnos" << std::endl;
    sync_cout << "Detected " << std::max(1u, std::thread::hardware_concurrency()) << " CPU core(s)" << std::endl;

    Bitboards::initialize();
    Zobrist::initialize();
    LargePages::initialize(); 
    Evaluation::initialize();
    Search::initialize();
    UCI uci;

    // Benchmark::runPerft();

    uci.mainLoop();

    return 0;
}
