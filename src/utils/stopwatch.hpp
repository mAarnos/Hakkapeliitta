/*
    Hakkapeliitta - A UCI chess engine. Copyright (C) 2013-2015 Mikko Aarnos.

    Hakkapeliitta is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Hakkapeliitta is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Hakkapeliitta. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef STOPWATCH_HPP_
#define STOPWATCH_HPP_

#include <cstdint>
#include <chrono>

// A high-resolution stopwatch-type timer.
class Stopwatch
{
public:
    Stopwatch();

    void start();
    void stop();
    void reset();
    bool isRunning() const;
	
	// If Stopwatch is running this returns the amount of time between now and starting the clock.
	// If Stopwatch is not running it returns the amount of time between the starting and stopping points.
    template <typename Resolution> 
	uint64_t elapsed() const;
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point stopTime;
    bool running;
};

inline Stopwatch::Stopwatch()
{
    reset();
}

inline void Stopwatch::start()
{
    startTime = std::chrono::high_resolution_clock::now();
    running = true;
}

inline void Stopwatch::stop()
{
    stopTime = std::chrono::high_resolution_clock::now();
    running = false;
}

inline void Stopwatch::reset()
{
    startTime = stopTime = std::chrono::high_resolution_clock::now();
    running = false;
}

inline bool Stopwatch::isRunning() const
{
    return running;
}

template <typename Resolution> 
uint64_t Stopwatch::elapsed() const
{
    const auto elapsed = (running ? std::chrono::high_resolution_clock::now() : stopTime)  - startTime;
    const auto time = std::chrono::duration_cast<Resolution>(elapsed).count();
    return time;
}

#endif
