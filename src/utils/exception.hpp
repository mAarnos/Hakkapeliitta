#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <stdexcept>

// Specific exception for problems in the algorithms of this program.
class HakkapeliittaException : public std::runtime_error
{
public:
    HakkapeliittaException(const std::string& description) : std::runtime_error(description) {}
};

#endif
