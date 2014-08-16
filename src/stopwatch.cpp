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

uint64_t Stopwatch::getTimeInMilliSeconds()
{
	uint64_t time;
	if (running)
	{
		auto elapsed = std::chrono::high_resolution_clock::now() - startTime;
		time = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	}
	else
	{
		auto elapsed = stopTime - startTime;
		time = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	}
	return time;
}
