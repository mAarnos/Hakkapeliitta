#ifndef TIME_H
#define TIME_H

#include "defs.h"
#include "uci.h"

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

extern Timer t;

extern int targetTime;
extern int maxTime;

extern int countDown;

const int stopInterval = 10000;
const int lagBuffer = 50;
const double stopFraction = 0.7;

void allocateSearchTime(string s);
void readClockAndInput();

#endif
