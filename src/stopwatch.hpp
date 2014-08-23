#ifndef STOPWATCH_HPP_
#define STOPWATCH_HPP_

#include <cstdint>
#include <chrono>

// TODO: make everything that is possible noexcept

class Stopwatch
{
public:
    Stopwatch(bool autoStart = false);

    void start();
    void stop();
    void reset();
    bool isRunning() const;
    template <typename Resolution> uint64_t elapsed() const;
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point stopTime;
    bool running;
};

inline Stopwatch::Stopwatch(bool autoStart)
{
    reset();
    if (autoStart)
    {
        start();
    }
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

template <typename Resolution> uint64_t Stopwatch::elapsed() const
{
    auto elapsed = (running ? (std::chrono::high_resolution_clock::now() - startTime) : (stopTime - startTime));
    auto time = std::chrono::duration_cast<Resolution>(elapsed).count();
    return time;
}

#endif
