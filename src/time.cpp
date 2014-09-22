#include "time.hpp"
#include "position.hpp"
#include "search.hpp"
#include "eval.hpp"

Timer t;

int targetTime;
int maxTime;

int countDown;

Timer::Timer():
startTime(0), stopTime(0), currentTime(0), running(0)
{
}

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

void checkTimeAndInput()
{
	countDown = stopInterval;
    auto time = t.getms();

    if (time > maxTime) // hard cutoff for search time
    {
        searching = false;
    }
    else if (time > targetTime)
    {
        if (bestRootScore == -mateScore) // no score for root
        {
            searching = false;
        }
        else if (bestRootScore < lastRootScore) // score dropping
        {
            if (time > 5 * targetTime) // extend search time up to 5x
            {
                searching = false;
            }
        }
    }
    else
    {
        // add easy move here
    }

    if (inputAvailable())
    {
        uciProcessInput();
    }
}

void allocateSearchTime(string s)
{
	array<int, Colours> time;
	array<int, Colours> increment;
	int movestogo;
	size_t pos;

	pos = s.find("movetime");
	if (pos != string::npos)
	{
		targetTime = stoi(s.substr(pos + 9));
		targetTime -= lagBuffer;
		return;
	}

	pos = s.find("infinite");
	if (pos != string::npos)
	{
		targetTime = INT_MAX;
        maxTime = INT_MAX;
		return;
	}

	pos = s.find("wtime");
	if (pos != string::npos)
	{
		time[White] = stoi(s.substr(pos + 6));
	}

	pos = s.find("btime");
	if (pos != string::npos)
	{
		time[Black] = stoi(s.substr(pos + 6));
	}

	pos = s.find("winc");
	if (pos != string::npos)
	{
		increment[White] = stoi(s.substr(pos + 5));
	}

	pos = s.find("binc");
	if (pos != string::npos)
	{
		increment[Black] = stoi(s.substr(pos + 5));
	}

	pos = s.find("movestogo");
	if (pos != string::npos)
	{
		movestogo = stoi(s.substr(pos + 10));
		movestogo += 2;
	}
	else
	{
		movestogo = 25;
	}

	targetTime = time[root.getSideToMove()] / movestogo + increment[root.getSideToMove()];
	targetTime -= lagBuffer;
    maxTime = time[root.getSideToMove()] / 2 + increment[root.getSideToMove()];

	if (targetTime < 0)
	{
		targetTime = 0;
	}
}