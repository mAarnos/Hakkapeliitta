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

    locked_ostream& operator<<(manip_t m)
    {
        os << m;
        return *this;
    }
private:
    std::mutex& mutex;
    std::ostream& os;
};

class synchonized_ostream
{
public:
    synchonized_ostream(std::ostream& oStream) : oStream(oStream) {};

    template <typename T>
    locked_ostream operator<<(const T& t)
    {
        mutex.lock();
        oStream << t;
        return locked_ostream(oStream, mutex);
    }

    locked_ostream operator<<(manip_t m)
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
