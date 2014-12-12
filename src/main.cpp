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

    /*
    Position pos;
    bool zugzwangLikely;

    // pos.initializePositionFromFen("2kr1b2/pb3p2/2pqpn2/8/2B1q3/1N5Q/P2B1PPP/2Q2RK1 b - -");
    // auto score = Search::quiescenceSearch(pos, 0, -infinity, infinity, pos.inCheck());

    std::ifstream positions("C:\\draws.txt");
    std::ofstream results("results.txt");
    std::string text;

    while (std::getline(positions, text))
    {
        pos.initializePositionFromFen(text);
        auto score = Search::quiescenceSearch(pos, 0, -infinity, infinity, pos.inCheck());
        results << score << std::endl;
    }
    */

    uci.mainLoop();

    return 0;
}