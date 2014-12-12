#ifndef UCI_HPP_
#define UCI_HPP_

#include <iostream>
#include <map>
#include "position.hpp"
#include "utils\threadpool.hpp"

class UCI
{
public:
    UCI();

    void mainLoop();
private:
    // A single command from the controller to us.
    class Command
    {
    public:
        Command(std::string name, std::string arguments) :
            name(name), arguments(arguments)
        {
        }

        std::string getName() const { return name; }
        std::string getArguments() const { return arguments; }
    private:
        std::string name;
        std::string arguments;
    };

    using FunctionPointer = void(*)(const Command& c);

    void addCommand(std::string name, FunctionPointer fp);
    std::map<std::string, FunctionPointer> commands;

    static void preprocessLine(std::string& line);

    static Position root;
    static ThreadPool tp;

    // UCI commands.

    static void sendInformation(const Command& c);
    static void isReady(const Command& c);
    static void stop(const Command&);
    static void quit(const Command& c);
    static void setOption(const Command& c);
    static void newGame(const Command& c);
    static void position(const Command& c);
    static void go(const Command& c);
};


#endif
