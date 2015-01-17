#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <stdexcept>

// Thrown inside the search function when either the allocated time is up or we have been ordered to stop.
class StopSearchException : public std::runtime_error
{
public:
    StopSearchException(const std::string& description) : std::runtime_error(description) {}
};

#endif
