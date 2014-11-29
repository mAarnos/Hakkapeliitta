#include <iostream>
#include <thread>
#include <fstream>
#include "uci.hpp"
#include "benchmark.hpp"
#include "utils\threadpool.hpp"
#include "eval.hpp"
#include "search.hpp"
#include "utils\synchronized_ostream.hpp"
#include "utils\large_pages.hpp"

// Things to test/add/do:
// 1. TUNE THE EVALUATION! IT IS SO LAUGHABLY BAD THAT THIS IS FIRST!
// 2. TUNE THE SEARCH!
// 3. Material table
// 4. SEE pruning at low depths
// 5. Mate-threat extension
// 6. Singular extension
// 7. Large pages for TT and PHT
// 8. Correct staticEval with the value from the TT.

int main() 
{
    sync_cout << "Hakkapeliitta 2.0 alpha, (C) 2013-2014 Mikko Aarnos" << std::endl;
    sync_cout << "Detected " << std::thread::hardware_concurrency() << " CPU core(s)" << std::endl;

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