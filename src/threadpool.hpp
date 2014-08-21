#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_

#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <functional>
#include <condition_variable>

class ThreadPool
{
public:
    ThreadPool(int amountOfThreads);
    ~ThreadPool();

    template<class Fn, class... Args>
    void addJob(Fn&& fn, Args&&... args);
private:
    void loop();

    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobQueue;

    std::mutex jobQueueMutex;
    std::condition_variable cv;
    std::atomic<bool> terminateFlag;
};

inline ThreadPool::ThreadPool(int amountOfThreads):
terminateFlag(false)
{
    for (auto i = 0; i < amountOfThreads; ++i)
    {
        threads.push_back(std::thread(&ThreadPool::loop, this));
    }
}

inline ThreadPool::~ThreadPool()
{
    terminateFlag = true;
    cv.notify_all();

    for (auto & thread : threads)
    {
        thread.join();
    }
}

template<class Fn, class... Args>
void ThreadPool::addJob(Fn&& fn, Args&&... args)
{
    auto job = std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
    std::unique_lock<std::mutex> lock(jobQueueMutex);
    jobQueue.push(job);
    cv.notify_one();
}

inline void ThreadPool::loop()
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

#endif