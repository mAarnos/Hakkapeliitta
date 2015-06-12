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

/// @file stopwatch.hpp
/// @author Mikko Aarnos

#ifndef STOPWATCH_HPP_
#define STOPWATCH_HPP_

#include <cstdint>
#include <chrono>

/// @brief A high-resolution stopwatch-type timer.
class Stopwatch
{
public:
    /// @brief Default constructor.
    Stopwatch() noexcept;

    /// @brief Starts the stopwatch.
    void start() noexcept;

    /// @brief Stops the stopwatch.
    void stop() noexcept;

    /// @brief Resets the stopwatch.
    void reset() noexcept;

    /// @brief Used for checking if the stopwatch is currently running.
    /// @return True if it is, false otherwise.
    bool isRunning() const noexcept;
    
    /// @brief Returns the elapsed time between the starting and stopping (or current if not stopped) points. 
    /// @tparam Resolution The resolution to report the elapsed time in.
    /// @return The elapsed time.
    template <typename Resolution> 
    uint64_t elapsed() const;

private:
    std::chrono::high_resolution_clock::time_point mStartTime;
    std::chrono::high_resolution_clock::time_point mStopTime;
    bool mRunning;
};

inline Stopwatch::Stopwatch() noexcept
{
    reset();
}

inline void Stopwatch::start() noexcept
{
    mStartTime = std::chrono::high_resolution_clock::now();
    mRunning = true;
}

inline void Stopwatch::stop() noexcept
{
    mStopTime = std::chrono::high_resolution_clock::now();
    mRunning = false;
}

inline void Stopwatch::reset() noexcept
{
    mStartTime = mStopTime = std::chrono::high_resolution_clock::now();
    mRunning = false;
}

inline bool Stopwatch::isRunning() const noexcept
{
    return mRunning;
}

template <typename Resolution> 
uint64_t Stopwatch::elapsed() const
{
    const auto elapsed = (mRunning ? std::chrono::high_resolution_clock::now() : mStopTime)  - mStartTime;
    const auto time = std::chrono::duration_cast<Resolution>(elapsed).count();
    return time;
}

#endif
