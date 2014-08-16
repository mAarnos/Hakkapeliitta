#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_

#include <mutex>
#include <vector>
#include <thread>
#include <condition_variable>

class ThreadPool
{
public:
    ThreadPool(int amountOfThreads);
    ~ThreadPool();
private:
    void loop();

    std::vector<std::thread> threads;

    std::mutex jobQueueMutex;
    std::condition_variable cv;
    bool terminate;
};

#endif