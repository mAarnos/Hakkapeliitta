#include "uci.hpp"
#include <iostream>
#include "utils/clamp.hpp"
#include "utils/synchronized_ostream.hpp"
#include "benchmark.hpp"

UCI::UCI() :
tp(1)
{
    addCommand("uci", &UCI::sendInformation);
    addCommand("isready", &UCI::isReady);
    addCommand("stop", &UCI::stop);
    addCommand("setoption", &UCI::setOption);
    addCommand("ucinewgame", &UCI::newGame);
    addCommand("position", &UCI::position);
    addCommand("go", &UCI::go);
    addCommand("quit", &UCI::quit);
    addCommand("ponderhit", &UCI::ponderhit);
    addCommand("displayboard", &UCI::displayBoard);
    addCommand("perft", &UCI::perft);
}

void UCI::mainLoop()
{
    std::string cmd, commandName;

    for (;;)
    {
        // Read a line from stdin.
        if (!std::getline(std::cin, cmd))
            break;

        // Using string stream makes parsing a lot easier than regexes.
        // Note to self: Lucas Braesch is always right.
        std::istringstream iss(cmd);
        
        // Ignore empty lines.
        if (!(iss >> commandName))
            continue;

        // If we are currently searching only the commands stop, quit, and isready are legal.
        if (commandName != "stop" && commandName != "quit" && commandName != "isready")
        {
            continue;
        }

        // Go through the list of commands and call the correct function if the command entered is known.
        // If the command is unknown report that.
        if (commands.count(commandName))
        {
            (this->*commands[commandName])(iss);
        }
        else
        {
            sync_cout << "info string unknown command" << std::endl;
        }
    }
}

void UCI::addCommand(const std::string& name, FunctionPointer fp)
{
    commands[name] = fp;
}

// UCI commands.

void UCI::sendInformation(std::istringstream&)
{
    // Send the name of the engine and the name of it's author.
    sync_cout << "id name Hakkapeliitta 2.5" << std::endl;
    sync_cout << "id author Mikko Aarnos" << std::endl;

    // Send all possible options the engine has that can be modified.
    sync_cout << "option name Hash type spin default 32 min 1 max 65536" << std::endl;
    sync_cout << "option name PawnHash type spin default 4 min 1 max 8192" << std::endl;
    sync_cout << "option name Clear Hash type button" << std::endl;
    sync_cout << "option name Contempt type spin default 0 min -75 max 75" << std::endl;
    // sync_cout << "option name Ponder type check default false" << std::endl;
    // sync_cout << "option name SyzygyPath type string default C:\\wdl\\" << std::endl;
    // sync_cout << "option name SyzygyProbeLimit type spin default 0 min 0 max 6" << std::endl;
    // sync_cout << "option name LargePages type check default false" << std::endl;

    // Send a response telling the listener that we are ready in UCI-mode.
    sync_cout << "uciok" << std::endl;
}

void UCI::isReady(std::istringstream&)
{
    sync_cout << "readyok" << std::endl;
}

void UCI::stop(std::istringstream&)
{
    /*
    gameState.searching = false;
    gameState.pondering = false;
    gameState.infiniteSearch = false;
    */
}

void UCI::quit(std::istringstream&)
{
    // First shut down the possible search.
    /*
    gameState.searching = false;
    gameState.pondering = false;
    gameState.infiniteSearch = false;
    */
    // Then shut down the thread pool. This part is only here due to a bug in VS.
    tp.terminate();
    // Finally exit.
    exit(0);
}

void UCI::setOption(std::istringstream&)
{
}

void UCI::newGame(std::istringstream&)
{
}

void UCI::go(std::istringstream&)
{
}

void UCI::position(std::istringstream&)
{
}

void UCI::ponderhit(std::istringstream&)
{
}

void UCI::displayBoard(std::istringstream&)
{
    // sync_cout << gameState.root.displayPositionAsString() << std::endl;
}

void UCI::perft(std::istringstream& iss)
{
    int depth;

    if (!(iss >> depth))
    {
        sync_cout << "info string argument is invalid" << std::endl;
    }
}
