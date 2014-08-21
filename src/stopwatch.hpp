#ifndef STOPWATCH_HPP_
#define STOPWATCH_HPP_

#include <cstdint>
#include <chrono>

class Stopwatch
{
public:
    Stopwatch();

    void start();
    void stop();
    void reset();

    template <typename Resolution> uint64_t elapsed();
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point stopTime;
    std::chrono::high_resolution_clock::time_point currentTime;
    bool running;
};

inline Stopwatch::Stopwatch()
{
    reset();
}

inline void Stopwatch::start()
{
    startTime = std::chrono::high_resolution_clock::now();
    running = true;
}

inline void Stopwatch::stop()
{
    stopTime = std::chrono::high_resolution_clock::now();
    running = false;
}

inline void Stopwatch::reset()
{
    startTime = std::chrono::high_resolution_clock::now();
    stopTime = startTime;
    currentTime = startTime;
    running = false;
}

template <typename resolution> uint64_t Stopwatch::elapsed()
{
    auto elapsed = (running ? (std::chrono::high_resolution_clock::now() - startTime) : (stopTime - startTime));
    auto time = std::chrono::duration_cast<resolution>(elapsed).count();
    return time;
}

#endif
