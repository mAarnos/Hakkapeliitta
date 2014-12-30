#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <stdexcept>

class StopSearchException : public std::runtime_error
{
public:
    StopSearchException(const std::string& description) : std::runtime_error(description) {}
};

#endif
