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

    Position pos("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -");

    Benchmark::runPerft(pos, 6);

    return 0;
}
