#ifndef SYNCHRONIZED_OSTREAM_HPP_
#define SYNCHRONIZED_OSTREAM_HPP_

#include <sstream>
#include <mutex>

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
private:
    std::ostream& oStream;
    std::mutex mutex;
};

#endif
