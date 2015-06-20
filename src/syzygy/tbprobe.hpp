#ifndef TBPROBE_HPP
#define TBPROBE_HPP

#include <string>

class Syzygy
{
public:
    static void initialize(const std::string& path);

    static int maxCardinality;
};

#endif
