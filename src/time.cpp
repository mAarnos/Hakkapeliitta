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
    array<int, Colours> timeLimits = { 0, 0 };
    array<int, Colours> incrementAmount = { 0, 0 };
    int movestogo = 25;
    size_t pos;

    pos = s.find("movetime");
    if (pos != string::npos)
    {
        maxTime = stoi(s.substr(pos + 9));
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
        timeLimits[White] = stoi(s.substr(pos + 6));
    }

    pos = s.find("btime");
    if (pos != string::npos)
    {
        timeLimits[Black] = stoi(s.substr(pos + 6));
    }

    pos = s.find("winc");
    if (pos != string::npos)
    {
        incrementAmount[White] = stoi(s.substr(pos + 5));
    }

    pos = s.find("binc");
    if (pos != string::npos)
    {
        incrementAmount[Black] = stoi(s.substr(pos + 5));
    }

    pos = s.find("movestogo");
    if (pos != string::npos)
    {
        movestogo = stoi(s.substr(pos + 10)) + 2;
    }

    auto time = timeLimits[root.getSideToMove()];
    auto increment = incrementAmount[root.getSideToMove()];
    targetTime = std::min(std::max(1, time / movestogo + increment - lagBuffer), time - lagBuffer);
    maxTime = std::min(std::max(1, time / 2 + increment), time - lagBuffer);
}