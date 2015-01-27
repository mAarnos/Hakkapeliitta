#ifndef UCI_HPP_
#define UCI_HPP_

#include <iostream>
#include <map>
#include "utils/threadpool.hpp"
#include "benchmark.hpp"

class UCI
{
public:
    UCI();// GameState& gameState);

    void mainLoop();
private:
    using FunctionPointer = void(UCI::*)(Position& pos, std::istringstream& iss);

    void addCommand(const std::string& name, FunctionPointer fp);
    std::map<std::string, FunctionPointer> commands;

    // GameState& gameState;
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
};


#endif
