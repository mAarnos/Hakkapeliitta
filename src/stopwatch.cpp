#include "stopwatch.hpp"

Stopwatch::Stopwatch()
{
	reset();
}

void Stopwatch::start()
{
	startTime = std::chrono::high_resolution_clock::now();
	running = true;
}

void Stopwatch::stop()
{
	stopTime = std::chrono::high_resolution_clock::now();
	running = false;
}

void Stopwatch::reset()
{
	startTime = std::chrono::high_resolution_clock::now();
	stopTime = startTime;
	currentTime = startTime;
	running = false;
}

template <typename resolution> uint64_t Stopwatch::elapsed()
{
    auto elapsed = (running ? std::chrono::high_resolution_clock::now() - startTime : stopTime - startTime);
    auto time = std::chrono::duration_cast<resolution>(elapsed).count();
	return time;
}

template uint64_t Stopwatch::elapsed<std::chrono::milliseconds>();