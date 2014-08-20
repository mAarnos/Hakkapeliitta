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

  //  Tuning tuning;
//    tuning.tune();

    ThreadPool pool(2);

    std::function<void()> job = []() { std::cout << "hello" << std::endl; };
    std::function<void()> job2 = []() { std::cout << "hello2" << std::endl; };
    std::function<void()> job3 = []() { std::cout << "hello3" << std::endl; };
    pool.addJob(job);
    pool.addJob(job2);
    pool.addJob(job3);

    uci.mainLoop();

    return 0;
}