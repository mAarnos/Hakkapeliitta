#include "threadpool.hpp"
#include <iostream>

ThreadPool::ThreadPool(int amountOfThreads) :
terminateFlag(false)
{
    for (auto i = 0; i < amountOfThreads; ++i)
    {
        threads.push_back(std::thread(&ThreadPool::loop, this));
    }
}

ThreadPool::~ThreadPool()
{
    terminateFlag = true;

    for (auto & thread : threads)
    {
        thread.join();
    }
}

void ThreadPool::loop()
{
    for (;;)
    {
        // Critical section
        std::unique_lock<std::mutex> lock(jobQueueMutex);
        std::cout << "do something" << std::endl;
    }
}