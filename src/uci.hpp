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

/// @file uci.hpp
/// @author Mikko Aarnos

#ifndef UCI_HPP_
#define UCI_HPP_

#include <iostream>
#include <map>
#include "benchmark.hpp"
#include "search_listener.hpp"
#include "search.hpp"
#include "utils/synchronized_ostream.hpp"

/// @brief Used for handling all communication between the chess engine and the outside world. 
class UCI : public SearchListener
{
public:
    /// @brief Default constructor.
    UCI();

    /// @brief Enter the event loop. There is no way to return from this function.
    void mainLoop();

private:
    using FunctionPointer = void(UCI::*)(Position& pos, std::istringstream& iss);

    void addCommand(const std::string& name, FunctionPointer fp);
    std::map<std::string, FunctionPointer> commands;

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
    synchronized_ostream sync_cout;

    // The current status of some UCI-options.
    bool ponder;
    int contempt;
    size_t pawnHashTableSize;
    size_t transpositionTableSize;
    int syzygyProbeDepth;
    int syzygyProbeLimit;
    bool syzygy50MoveRule;

    // History of the current position, if any.
    int rootPly;
    std::vector<HashKey> repetitionHashKeys;

    // Implementations of some pure virtual functions in SearchListener.
    virtual void infoCurrMove(const Move& move, int depth, int i);
    virtual void infoRegular(uint64_t nodeCount, uint64_t tbHits, uint64_t searchTime, int hashFull);
    virtual void infoPv(const std::vector<Move>& pv, uint64_t searchTime,
                        uint64_t nodeCount, uint64_t tbHits,
                        int depth, int score, int flags, int selDepth, int hashFull);
    virtual void infoBestMove(const std::vector<Move>& pv, uint64_t searchTime, 
                              uint64_t nodeCount, uint64_t tbHits, int hashFull);
};

#endif
