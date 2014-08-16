#include "threadpool.hpp"
#include <iostream>

ThreadPool::ThreadPool(int amountOfThreads) :
terminate(false)
{
    for (auto i = 0; i < amountOfThreads; ++i)
    {
        threads.push_back(std::thread(&ThreadPool::loop, this));
    }
}

ThreadPool::~ThreadPool()
{
    for (auto & thread : threads)
    {
        thread.join();
    }
}

void ThreadPool::loop()
{
    for (;;)
    {
        std::cout << "do something" << std::endl;
    }
}