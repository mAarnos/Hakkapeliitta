#ifndef TBPROBE_HPP_
#define TBPROBE_HPP_

#include <string>

class Position;

class Syzygy
{
public:
    static void initialize(const std::string& path);

    static int probeWdl(Position& pos, int& success);
    static int probeDtz(Position& pos, int& success);
    static bool rootProbe(Position& pos, int& tbScore);
    static bool rootProbeWdl(Position& pos, int& tbScore);

    static int tbLargest;
};

#endif
