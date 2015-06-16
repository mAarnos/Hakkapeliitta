/*
    Hakkapeliitta - A UCI chess engine. Copyright (C) 2013-2015 Mikko Aarnos.

    Hakkapeliitta is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Hakkapeliitta is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Hakkapeliitta. If not, see <http://www.gnu.org/licenses/>.
*/

/// @file threadpool.hpp
/// @author Mikko Aarnos

#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_

#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <functional>
#include <condition_variable>

/// @brief A simple thread pool implementation.
///
/// Most likely contains bugs. Haven't found them so far, if they exist.
/// All functions given as jobs must return nothing (i.e. be void).
class ThreadPool
{
public:
    /// @brief Constructs a new threadpool.
    /// @param amountOfThreads The amount of threads that the thread pool should have.
    ThreadPool(int amountOfThreads);

    /// @brief Destructor, terminates all running threads after they have finished their work.
    ~ThreadPool();

    /// @brief Adds a new job into the queue of the thread pool.
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

inline ThreadPool::ThreadPool(int amountOfThreads) :
    terminateFlag(false)
{
    for (auto i = 0; i < amountOfThreads; ++i)
    {
        threads.emplace_back(&ThreadPool::loop, this);
    }
}

inline ThreadPool::~ThreadPool()
{
    terminateFlag = true;
    cv.notify_all();
    for (auto& thread : threads)
    {
        thread.join();
    }
}

template<class Fn, class... Args>
void ThreadPool::addJob(Fn&& fn, Args&&... args)
{
    const auto job = std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
    std::unique_lock<std::mutex> lock(jobQueueMutex);
    jobQueue.push(job);
    cv.notify_one();
}

inline void ThreadPool::loop()
{
    for (;;)
    {
        std::function<void()> job;

        {
            std::unique_lock<std::mutex> lock(jobQueueMutex);
            while (!terminateFlag && jobQueue.empty())
            {
                cv.wait(lock);
            }

            if (terminateFlag)
            {
                return;
            }

            job = jobQueue.front();
            jobQueue.pop();
        }

        job();
    }
}

#endif