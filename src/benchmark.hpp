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

#ifndef BENCHMARK_HPP_
#define BENCHMARK_HPP_

#include <cstdint>
#include "position.hpp"
#include "movegen.hpp"

class Benchmark
{
public:
    // Run perft to depth given on the position given.
    // Returns a pair of the perft result and the time it took to calculate it, in ms.
    std::pair<uint64_t, uint64_t> runPerft(Position root, int depth);
private:
    MoveGen moveGen;
    uint64_t perft(const Position& pos, int depth);
};

#endif