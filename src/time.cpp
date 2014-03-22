#ifndef TIME_CPP
#define TIME_CPP

#include "time.h"

void Timer::start()
{
	startTime = clock();
	running = true;
}

void Timer::stop()
{
	stopTime = clock();
	running = false;
}

void Timer::reset() 
{
	startTime = 0;
	stopTime = 0;
	currentTime = 0;
	running = false;
}

uint64_t Timer::getms()
{
	if (running)
	{
		currentTime = clock();
		return (currentTime - startTime);
	}
	else
	{
		return (stopTime - startTime);
	}
}

#endif