#include <iostream>
#include "uci.hpp"
#include "benchmark.hpp"
#include "tuning.hpp"
#include "threadpool.hpp"
#include "eval.hpp"

void job()
{
    std::cout << "hello" << std::endl;
}

void job2()
{
    std::cout << "hello2" << std::endl;
}

void job3(int i)
{
    std::cout << "hello" << i << std::endl;
}

int main() 
{
    std::cout << "Hakkapeliitta, (C) 2013-2014 Mikko Aarnos" << std::endl;

    Bitboards::initialize();
    Zobrist::initialize();
    Evaluation::initialize();

    UCI uci;

    // Benchmark::runPerft();

    // Tuning tuning;
    // tuning.tune();

    // ThreadPool pool(2);

    // pool.addJob(job);
    // pool.addJob(job2);
    // pool.addJob(job3, 7);

    uci.mainLoop();

    return 0;
}