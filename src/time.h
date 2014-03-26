#ifndef TIME_H
#define TIME_H

#include "defs.h"

// A stopwatch-type timer.
class Timer
{
	private:
		clock_t startTime;
		clock_t stopTime;
		clock_t currentTime;
		bool running;
	public:
		void start();
		void stop();
		void reset();
		uint64_t getms();
};

#endif
