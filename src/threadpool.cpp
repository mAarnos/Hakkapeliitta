#include "threadpool.hpp"

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
    cv.notify_all();

    for (auto & thread : threads)
    {
        thread.join();
    }
}

void ThreadPool::addJob(std::function<void()> job)
{ 
    std::unique_lock<std::mutex> lock(jobQueueMutex);
    jobQueue.push(job);
    cv.notify_one();
}

void ThreadPool::loop()
{
    for (;;)
    {
        std::unique_lock<std::mutex> lock(jobQueueMutex);
        while (!terminateFlag && jobQueue.empty())
            cv.wait(lock);
        if (terminateFlag)
            return;
        auto job = jobQueue.front();
        jobQueue.pop();
        lock.unlock();
        job();
    }
}