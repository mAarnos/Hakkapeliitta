#include "uci.hpp"
#include <iostream>
#include <regex>
#include "search.hpp"
#include "eval.hpp"
#include "utils\clamp.hpp"

UCI::UCI()
{
    addCommand("uci", &UCI::sendInformation);
    addCommand("isready", &UCI::isReady);
    addCommand("stop", &UCI::stop);
    addCommand("setoption", &UCI::setOption);
    addCommand("ucinewgame", &UCI::newGame);
    addCommand("position", &UCI::position);
    addCommand("go", &UCI::go);
    addCommand("quit", &UCI::quit);
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

void UCI::preprocessLine(std::string& line)
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

void UCI::sendInformation(const Command&)
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

void UCI::isReady(const Command&)
{
    std::cout << "readyok" << std::endl;
}

void UCI::stop(const Command&)
{
    Search::searching = false;
    Search::pondering = false;
    Search::infinite = false;
}

void UCI::quit(const Command&)
{
    exit(0);
}

void UCI::setOption(const Command& c)
{
    std::string option, parameter;

    // The string s for setoption comes in the form "name" option "value" parameter.
    // We just ignore "name" and "value" and get option and parameter.
    std::regex expr("\\w*\\s(\\w*)\\s\\w*\\s*(.*)");
    std::smatch matches;
    if (std::regex_search(c.getArguments(), matches, expr))
    {
        option = matches[1];
        parameter = matches[2];
    }

    if (option == "contempt")
    {
        try
        {
            Search::contemptValue = clamp(stoi(parameter), -75, 75);
        }
        catch (const std::exception&)
        {
            Search::contemptValue = 0;
        }
    }
    else if (option == "hash" || option == "pawn") // Thanks to our parsing pawn hash is shortened to pawn
    {
        int sizeInMegabytes;

        try
        {
            sizeInMegabytes = stoi(parameter);
            sizeInMegabytes = clamp(sizeInMegabytes, 1, option == "hash" ? 65536 : 8192);
        }
        catch (const std::exception&)
        {
            sizeInMegabytes = (option == "hash" ? 32 : 4);
        }

        option == "hash" ? Search::transpositionTable.setSize(sizeInMegabytes)
                         : Evaluation::pawnHashTable.setSize(sizeInMegabytes);
    }
    else if (option == "clear") // Thanks to our parsing clear hash is shortened to clear. Can't be helped.
    {
        Search::transpositionTable.clear();
        Search::historyTable.clear();
        Search::killerTable.clear();
        Evaluation::pawnHashTable.clear();
    }
    else if (option == "syzygyprobelimit") // FIX ME!
    {
        int syzygyProbeLimit;
        try
        {
            syzygyProbeLimit = clamp(stoi(parameter), 0, 6);
        }
        catch (const std::exception&)
        {
            syzygyProbeLimit = 0;
        }
    }
    else if (option == "syzygypath") // FIX ME!
    {
    }
}

void UCI::newGame(const Command&)
{
    Search::transpositionTable.clear();
    Search::historyTable.clear();
    Search::killerTable.clear();
    Evaluation::pawnHashTable.clear();
}

void UCI::go(const Command& c)
{

}

void UCI::position(const Command& c)
{

}
