#ifndef TIME_CPP
#define TIME_CPP

#include "time.h"
#include "search.h"
#include "eval.h"
#include "hash.h"
#include "uci.h"

signed long long countdown = 10000;
int maxtime = 60000;
bool timedout;

Timer t;

int wtime;
int btime;
int winc;
int binc;
int movestogo;

void Timer::start()
{
	running = true;
	ftime(&startBuffer);
	startTime = startBuffer.time * 1000 + startBuffer.millitm;
}

void Timer::stop()
{
	running = false;
	ftime(&stopBuffer);
	stopTime = stopBuffer.time * 1000 + stopBuffer.millitm;
}

void Timer::reset() 
{
	running = false;
	startTime = 0;
	stopTime = 0;
	currentTime = 0;
}

U64 Timer::getms()
{
	if (running)
	{
		ftime(&currentBuffer);
		currentTime = currentBuffer.time * 1000 + currentBuffer.millitm;
		return (currentTime - startTime);
	}
	else 
		return (stopTime - startTime);
}

void readClockAndInput()
{
	countdown = stopinterval;
	if (t.getms() > maxtime)
	{
		timedout = true;
	}
	if (checkInput()) listenForInput();
}

void go(char * command)
{
	movestogo = 0;
	Searching = true;
	timedout = false;
	useTB = true;

	if (strstr(command, "wtime")) 
	{ 
		sscanf(strstr(command, "wtime"), "wtime %d", &wtime);
	}
    if (strstr(command, "btime"))
	{ 
		sscanf(strstr(command, "btime"), "btime %d", &btime);
	}
    if (strstr(command, "winc"))
	{ 
		sscanf(strstr(command, "winc"), "winc %d", &winc);
	}
    if (strstr(command, "binc")) 
	{ 
		sscanf(strstr(command, "binc"), "binc %d", &binc);
	}
    if (strstr(command, "movestogo")) 
	{ 
		sscanf(strstr(command, "movestogo"), "%*s %d", &movestogo);
	}
	
	if (!movestogo)
		movestogo = 25;
	else 
		movestogo += 2; // create a time buffer to avoid losses on time

	if (sideToMove == White)
	{
		maxtime = (wtime / movestogo) + winc;
	}
	else 
	{
		maxtime = (btime / movestogo) + binc;
	}

	// create a time buffer to avoid losses on time
	maxtime -= timeBuffer;

	if (maxtime <= 0)
		maxtime = 1;

	if (strstr(command, "movetime"))
	{ 
		sscanf(strstr(command, "movetime"), "%*s %d", &maxtime);
	}

	if (strstr(command, "infinite")) 
	{
		maxtime = 999999999;
		Infinite = true;
	}

	think();
}


#endif