#ifndef UCI_HPP_
#define UCI_HPP_

#include <iostream>
#include <map>
#include "utils/threadpool.hpp"
#include "benchmark.hpp"
// #include "gamestate.hpp"

class UCI
{
public:
    UCI();// GameState& gameState);

    void mainLoop();
private:
    using FunctionPointer = void(UCI::*)(std::istringstream& iss);

    void addCommand(const std::string& name, FunctionPointer fp);
    std::map<std::string, FunctionPointer> commands;

    // GameState& gameState;
    ThreadPool tp;
    Benchmark benchMark;

    // UCI commands.

    void sendInformation(std::istringstream& iss);
    void isReady(std::istringstream& iss);
    void stop(std::istringstream& iss);
    void quit(std::istringstream& iss);
    void setOption(std::istringstream& iss);
    void newGame(std::istringstream& iss);
    void position(std::istringstream& iss);
    void go(std::istringstream& iss);
    void ponderhit(std::istringstream& iss);
    void displayBoard(std::istringstream& iss);
    void perft(std::istringstream& iss);
};


#endif
