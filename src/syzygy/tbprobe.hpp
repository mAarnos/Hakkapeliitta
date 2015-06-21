#ifndef TBPROBE_HPP
#define TBPROBE_HPP

#include <string>

class Syzygy
{
public:
    static void initialize(const std::string& path);

    // Probe the WDL table for a particular position.
    // If success != 0, the probe was successful.
    // The return value is from the point of view of the side to move:
    // -2 : loss
    // -1 : loss, but draw under 50-move rule
    //  0 : draw
    //  1 : win, but draw under 50-move rule
    //  2 : win
    static int probeWdl(const Position& pos, int& success);

    static int maxCardinality;
};

#endif
