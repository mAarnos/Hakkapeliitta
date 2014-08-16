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
    uint64_t getTimeInMilliSeconds();
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point stopTime;
    std::chrono::high_resolution_clock::time_point currentTime;
    bool running;
};

#endif
