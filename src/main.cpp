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

#include <iostream>
#include <thread>
#include <algorithm>
#include "bitboards.hpp"
#include "zobrist.hpp"
#include "evaluation.hpp"
#include "uci.hpp"
// #include "test.hpp"

int main() 
{
    std::cout << "Hakkapeliitta 3.0 alpha (C) 2013-2015 Mikko Aarnos" << std::endl;
    std::cout << "Detected " << std::max(1u, std::thread::hardware_concurrency()) << " CPU core(s)" << std::endl;

    Bitboards::staticInitialize();
    Zobrist::staticInitialize();
    Evaluation::staticInitialize();

    if (Bitboards::hardwarePopcntSupported())
    {
        std::cout << "Detected hardware POPCNT" << std::endl;
    }

    /*
    Testing testing("C:\\GMblackwin.txt");
    std::cout << "Starting testing!" << std::endl;
    std::cout << " Reversed eval: ";
    std::cout << (testing.testReversedEval() ? "passed" : "failed") << std::endl;
    std::cout << " Pseudo-legal: ";
    std::cout << (testing.testPseudoLegal() ? "passed" : "failed") << std::endl;
    */

    UCI uci;

    uci.mainLoop();

    return 0;
}