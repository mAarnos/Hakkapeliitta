#ifndef STOPWATCH_HPP_
#define STOPWATCH_HPP_

#include <cstdint>
#include <chrono>

// A high-resolution stopwatch-type timer.
class Stopwatch
{
public:
    Stopwatch();

    void start();
    void stop();
    void reset();
    bool isRunning() const;
	
	// If Stopwatch is running this returns the amount of time between now and starting the clock.
	// If Stopwatch is not running it returns the amount of time between the starting and stopping points.
    template <typename Resolution> 
	uint64_t elapsed() const;
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point stopTime;
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
    startTime = stopTime = std::chrono::high_resolution_clock::now();
    running = false;
}

inline bool Stopwatch::isRunning() const
{
    return running;
}

template <typename Resolution> 
uint64_t Stopwatch::elapsed() const
{
    const auto elapsed = (running ? std::chrono::high_resolution_clock::now() : stopTime)  - startTime;
    const auto time = std::chrono::duration_cast<Resolution>(elapsed).count();
    return time;
}

#endif
