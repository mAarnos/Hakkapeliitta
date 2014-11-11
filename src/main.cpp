#include <iostream>
#include <thread>
#include "uci.hpp"
#include "benchmark.hpp"
#include "utils\threadpool.hpp"
#include "eval.hpp"
#include "search.hpp"
#include "utils\synchronized_ostream.hpp"
#include "utils\large_pages.hpp"

int main() 
{
    sync_cout << "Hakkapeliitta 2.0 alpha, (C) 2013-2014 Mikko Aarnos" << std::endl;
    sync_cout << "Detected " << std::thread::hardware_concurrency() << " CPU core(s)" << std::endl;

    Bitboards::initialize();
    Zobrist::initialize();
    LargePages::initialize(); // not used currently
    Evaluation::initialize();
    Search::initialize();
    UCI uci;

    LargePages::setAllowedToUse(true);
    auto ptr = LargePages::malloc(64 * 1024 * 1024, 64);
    LargePages::free(ptr);
    LargePages::setAllowedToUse(false);

    Benchmark::runPerft();

    uci.mainLoop();

    return 0;
}