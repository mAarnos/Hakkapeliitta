#ifndef TIME_H
#define TIME_H

#include "defs.h"
#include <ctime>

struct Timer
{
	clock_t startTime;
	clock_t stopTime;
	clock_t currentTime;
    bool running;  

	void start();
	void stop();
	void reset();
	U64 getms();
};

extern Timer t;

// stopinterval means after how many searched nodes we check if time has run out
const int stopinterval = 10000;
// don't start a new iteration of search if stopfrac of our max search time has passed
const double stopfrac = 0.7;
// how many nodes left until we check if time has run out
extern signed long long countdown;
extern int maxtime;
extern bool timedout;

const int timeBuffer = 50;

extern void readClockAndInput();

extern void go(char * command);

#endif
