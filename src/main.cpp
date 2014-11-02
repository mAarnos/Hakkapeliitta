#include <iostream>
#include "uci.hpp"
#include "benchmark.hpp"
#include "tuning\tuning.hpp"
#include "utils\threadpool.hpp"
#include "eval.hpp"
#include "search.hpp"
#include "utils\synchronized_ostream.hpp"
#include <memory>

synchronized_ostream sync_cout(std::cout);

void call_from_thread(int tid) 
{
    sync_cout << "Launched by thread " << tid << std::endl;
}

int main() 
{
    std::cout << "Hakkapeliitta 2.0 alpha, (C) 2013-2014 Mikko Aarnos" << std::endl;

    Bitboards::initialize();
    Zobrist::initialize();
    Evaluation::initialize();
    Search::initialize();
    UCI uci;
    // ThreadPool pool(1);

    const int num_threads = 30;
    std::thread t[num_threads];

    // Launch a group of threads
    for (int i = 0; i < num_threads; ++i) 
    {
        t[i] = std::thread(call_from_thread, i);
    }

    sync_cout << "Launched from the main thread" << std::endl;

    for (int i = 0; i < num_threads; ++i) {
        t[i].join();
    }

    // Benchmark::runPerft();

    // Tuning tuning;
    // tuning.tune();

    uci.mainLoop();

    return 0;
}