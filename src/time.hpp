#ifndef TIME_H_
#define TIME_H_

#include "defs.hpp"
#include "uci.hpp"

// A stopwatch-type timer.
class Timer
{
	public:
		Timer();

		void start();
		void stop();
		void reset();
		uint64_t getms();
	private:
		clock_t startTime;
		clock_t stopTime;
		clock_t currentTime;
		bool running;
};

extern Timer t;

extern int targetTime;
extern int maxTime;

extern int countDown;

const int stopInterval = 10000;
const int lagBuffer = 50;
const double stopFraction = 0.7;

void allocateSearchTime(string s);
void checkTimeAndInput();

#endif
