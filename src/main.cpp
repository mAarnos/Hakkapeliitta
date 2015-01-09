#include <iostream>
#include <thread>
#include <algorithm>
#include "zobrist.hpp"
#include "bitboard.hpp"
#include "utils/synchronized_ostream.hpp"
#include "position.hpp"
#include "benchmark.hpp"
#include "movelist.hpp"

int main() 
{
    sync_cout << "Hakkapeliitta 2.5, (C) 2013-2015 Mikko Aarnos" << std::endl;
    sync_cout << "Detected " << std::max(1u, std::thread::hardware_concurrency()) << " CPU core(s)" << std::endl;

    Bitboards::initialize();
    Zobrist::initialize();

    sync_cout << sizeof(Position) << std::endl;

    Position pos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Benchmark::runPerft(pos, 7);

    return 0;
}
