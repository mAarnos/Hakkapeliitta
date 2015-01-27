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

#ifndef SYNCHRONIZED_OSTREAM_HPP_
#define SYNCHRONIZED_OSTREAM_HPP_

#include <sstream>
#include <mutex>

using manip_t = std::ostream& (*)(std::ostream&);

class locked_ostream
{
public:
    locked_ostream(std::ostream& os, std::mutex& mutex) : mutex(mutex), os(os) {};
    ~locked_ostream() { mutex.unlock(); }

    template <typename T>
    locked_ostream& operator<<(const T& t)
    {
        os << t;
        return *this;
    }

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

class synchronized_ostream
{
public:
    synchronized_ostream(std::ostream& oStream) : oStream(oStream) {};

    template <typename T>
    locked_ostream operator<<(const T& t)
    {
        mutex.lock();
        oStream << t;
        return locked_ostream(oStream, mutex);
    }

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

extern synchronized_ostream sync_cout;

#endif
