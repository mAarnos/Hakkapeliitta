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

#ifndef UCI_HPP_
#define UCI_HPP_

#include <iostream>
#include <map>
#include "utils/threadpool.hpp"
#include "benchmark.hpp"
#include "tt.hpp"
#include "pht.hpp"
#include "history.hpp"
#include "killer.hpp"
#include "search.hpp"

class UCI
{
public:
    UCI();

    void mainLoop();
private:
    using FunctionPointer = void(UCI::*)(Position& pos, std::istringstream& iss);

    void addCommand(const std::string& name, FunctionPointer fp);
    std::map<std::string, FunctionPointer> commands;

    ThreadPool tp;
    Benchmark benchMark;

    // UCI commands.

    void sendInformation(Position& pos, std::istringstream& iss);
    void isReady(Position& pos, std::istringstream& iss);
    void stop(Position& pos, std::istringstream& iss);
    void quit(Position& pos, std::istringstream& iss);
    void setOption(Position& pos, std::istringstream& iss);
    void newGame(Position& pos, std::istringstream& iss);
    void position(Position& pos, std::istringstream& iss);
    void go(Position& pos, std::istringstream& iss);
    void ponderhit(Position& pos, std::istringstream& iss);
    void displayBoard(Position& pos, std::istringstream& iss);
    void perft(Position& pos, std::istringstream& iss);

    Search search;

    bool ponder;
    int contempt;
    size_t pawnHashTableSize;
    size_t transpositionTableSize;

    int rootPly;
    std::array<HashKey, 1024> repetitionHashKeys;
};


#endif
