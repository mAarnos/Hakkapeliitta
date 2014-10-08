#include "uci.hpp"
#include <iostream>
#include <regex>

UCI::UCI()
{
    addCommand("uci", &UCI::sendInformation);
    addCommand("isready", &UCI::isReady);
    addCommand("stop", &UCI::stop);
    addCommand("exit", &UCI::quit);
}

void UCI::mainLoop()
{
    std::string line;

    for (;;)
    {
        // Read a line from stdin.
        if (!std::getline(std::cin, line))
            break;

        preprocessLine(line);
        if (line.empty())
            continue;

        // The easiest way to parse the std::string is to use a regular expression.
        // Important: everything matches expr so no need for checks on success.
        std::regex expr("(\\w*)\\s?(.*)");
        std::smatch matches;
        std::regex_match(line, matches, expr);
        // matches[1] == command name
        // matches[2] == parameters
        Command currentCommand(matches[1], matches[2]);

        // Go through the list of commands and call the correct function if the command entered is known.
        // If the command is unknown report that.
        if (commands.count(currentCommand.getName()))
        {
            (*commands[currentCommand.getName()])(currentCommand);
        }
        else
        {
            std::cout << "info string unknown command" << std::endl;
        }
    }
}

void UCI::addCommand(std::string name, FunctionPointer fp)
{
    if (commands.count(name))
    {
        std::cerr << "Error: command with the same name already exists!" << std::endl;
        return;
    }
    commands[name] = fp;
}

void UCI::preprocessLine(std::string & line)
{
    // Discard the line if it is empty or whitespace only.
    if (line.find_first_not_of(' ') == std::string::npos)
    {
        line = "";
        return;
    }

    // Change all characters to lowercase.
    transform(line.begin(), line.end(), line.begin(), tolower);
    // Remove all extra whitespace.
    line = line.substr(line.find_first_not_of(' '));
    line = line.substr(0, line.find_last_not_of(' ') + 1);
    while (line.find("  ") != std::string::npos)
    {
        line.replace(line.find("  "), 2, " ");
    }
}

// UCI commands.

void UCI::sendInformation(const Command &)
{
    // Send the name of the engine and the name of it's author.
    std::cout << "id name Hakkapeliitta 2.0 alpha" << std::endl;
    std::cout << "id author Mikko Aarnos" << std::endl;

    // Send all possible options the engine has that can be modified.
    std::cout << "option name Hash type spin default 32 min 1 max 65536" << std::endl;
    std::cout << "option name Pawn Hash type spin default 4 min 1 max 8192" << std::endl;
    std::cout << "option name Clear Hash type button" << std::endl;
    std::cout << "option name Contempt type spin default 0 min -75 max 75" << std::endl;
    std::cout << "option name SyzygyPath type string default C:\\wdl\\" << std::endl;
    std::cout << "option name SyzygyProbeLimit type spin default 0 min 0 max 6" << std::endl;

    // Send a response telling the listener that we are ready in UCI-mode.
    std::cout << "uciok" << std::endl;
}

void UCI::isReady(const Command &)
{
    std::cout << "readyok" << std::endl;
}

void UCI::stop(const Command &)
{
    // searching = false;
}

void UCI::quit(const Command &)
{
    exit(0);
}
