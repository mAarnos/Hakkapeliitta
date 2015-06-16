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

/// @file synchronized_ostream.hpp
/// @author Mikko Aarnos

#ifndef SYNCHRONIZED_OSTREAM_HPP_
#define SYNCHRONIZED_OSTREAM_HPP_

#include <sstream>
#include <mutex>

/// @brief A thread-safe output stream.
class synchronized_ostream
{
public:
    /// @brief Default constructor.
    /// @param oStream The output stream where to output data.
    synchronized_ostream(std::ostream& oStream) : oStream(oStream) 
    {
    };

    /// @brief A simple typedef for convenience.
    using manip_t = std::ostream& (*)(std::ostream&);

    /// @brief A locked ostream to which only one thread can write stuff into.
    /// 
    /// Basically, whenever synchronized_ostream starts writing something it first locks a mutex 
    /// and creates an instance of locked_ostream which unlocks the mutex when there is nothing more to write.
    /// Since the mutex is locked, no other threads can print anything to the same ostream at the same time, 
    /// making synchronized_ostream thread-safe.
    class locked_ostream
    {
    public:
        /// @brief Default constructor.
        /// @param os The output stream where to output data.
        /// @param mutex A reference to the mutex of the synchronized_ostream so that we can unlock it when done.
        locked_ostream(std::ostream& os, std::mutex& mutex) : mutex(mutex), os(os)
        {
        };

        /// @brief When destroyed, unlocks the mutex so that other threads can use the synchronized_ostream.
        ~locked_ostream()
        {
            mutex.unlock();
        }

        /// @brief Output some data.
        /// @param t The data.
        /// @return A reference to the locked_ostream so that we can chain this operator.
        template <typename T>
        locked_ostream& operator<<(const T& t)
        {
            os << t;
            return *this;
        }

        /// @brief Change the properties of the output stream.
        /// @param m The ostream manipulator.
        /// @return A reference to the locked_ostream so that we can chain this operator.
        locked_ostream& operator<<(const manip_t m)
        {
            os << m;
            return *this;
        }

    private:
        std::mutex& mutex;
        std::ostream& os;

        locked_ostream& operator=(const locked_ostream&) = delete;
    };

    /// @brief Output some data.
    /// @param t The data.
    /// @return A reference to the locked_ostream so that we can chain this operator.
    template <typename T>
    locked_ostream operator<<(const T& t)
    {
        mutex.lock();
        oStream << t;
        return locked_ostream(oStream, mutex);
    }

    /// @brief Change the properties of the output stream.
    /// @param m The ostream manipulator.        
    /// @return A reference to the locked_ostream so that we can chain this operator.
    locked_ostream operator<<(const manip_t m)
    {
        mutex.lock();
        oStream << m;
        return locked_ostream(oStream, mutex);
    }

private:
    std::ostream& oStream;
    std::mutex mutex;
};

#endif
