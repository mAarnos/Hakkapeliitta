#include "uci.hpp"
#include <iostream>
#include "utils/clamp.hpp"
#include "utils/synchronized_ostream.hpp"
#include "benchmark.hpp"

UCI::UCI(TranspositionTable& transpositionTable, PawnHashTable& pawnHashTable, KillerTable& killerTable, HistoryTable& historyTable) :
tp(1), transpositionTable(transpositionTable), pawnHashTable(pawnHashTable), killerTable(killerTable), historyTable(historyTable)
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
    Position pos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::string cmd, commandName;

    for (;;)
    {
        // Read a line from stdin.
        if (!std::getline(std::cin, cmd))
            break;

        // Using string stream makes parsing a lot easier than regexes.
        // Note to self: Lucas Braesch is always right.
        std::istringstream iss(cmd);
        iss >> commandName;
        
        // Ignore empty lines.
        if (commandName.empty())
            continue;

        // If we are currently searching only the commands stop, quit, and isready are legal.
        /*
        if (commandName != "stop" && commandName != "quit" && commandName != "isready")
        {
            continue;
        }
        */

        // Go through the list of commands and call the correct function if the command entered is known.
        // If the command is unknown report that.
        if (commands.count(commandName))
        {
            (this->*commands[commandName])(pos, iss);
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

void UCI::sendInformation(Position&, std::istringstream&)
{
    // Send the name of the engine and the name of it's author.
    sync_cout << "id name Hakkapeliitta 2.5" << std::endl;
    sync_cout << "id author Mikko Aarnos" << std::endl;

    // Send all possible options the engine has that can be modified.
    sync_cout << "option name Hash type spin default 32 min 1 max 65536" << std::endl;
    sync_cout << "option name Pawn Hash type spin default 4 min 1 max 8192" << std::endl;
    sync_cout << "option name Clear Hash type button" << std::endl;
    sync_cout << "option name Contempt type spin default 0 min -75 max 75" << std::endl;
    // sync_cout << "option name Ponder type check default false" << std::endl;
    // sync_cout << "option name SyzygyPath type string default C:\\wdl\\" << std::endl;
    // sync_cout << "option name SyzygyProbeLimit type spin default 0 min 0 max 6" << std::endl;
    // sync_cout << "option name LargePages type check default false" << std::endl;

    // Send a response telling the listener that we are ready in UCI-mode.
    sync_cout << "uciok" << std::endl;
}

void UCI::isReady(Position&, std::istringstream&)
{
    sync_cout << "readyok" << std::endl;
}

void UCI::stop(Position&, std::istringstream&)
{
    /*
    gameState.searching = false;
    gameState.pondering = false;
    gameState.infiniteSearch = false;
    */
}

void UCI::quit(Position&, std::istringstream&)
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

void UCI::setOption(Position&, std::istringstream& iss) 
{
    std::string name, value, s;
    
    // Read the name of the option. 
    // Since the name can contains spaces we have this loop.
    while (iss >> s && s != "value")
    {
        name += std::string(" ", !name.empty()) + s;
    }

    // Read the value of the option.
    // The loop has the same reason as the previous one.
    while (iss >> s)
    {
        name += std::string(" ", !name.empty()) + s;
    }

    if (name == "Contempt")
    {

    }
    else if (name == "Hash")
    {

    }
    else if (name == "Pawn Hash")
    {

    }
    else if (name == "Clear Hash")
    {
        transpositionTable.clear();
        pawnHashTable.clear();
        historyTable.clear();
        killerTable.clear();
    }
    else
    {
        sync_cout << "info string no such option exists" << std::endl;
    }
}

void UCI::newGame(Position&, std::istringstream&)
{
    transpositionTable.clear();
    pawnHashTable.clear();
    historyTable.clear();
    killerTable.clear();
}

void UCI::go(Position&, std::istringstream& iss)
{
    auto movesToGo = 25;
    auto moveTime = 0;
    std::array<int, 2> timeLimits = { 0, 0 };
    std::array<int, 2> incrementAmount = { 0, 0 };
    std::string token;

    // Search::searching = true;
    // Search::pondering = false;
    // Search::infinite = false;

    // Parse the string to get the time limits.
    while (iss >> token)
    {
        if (token == "movetime") iss >> moveTime;
        else if (token == "infinite") {} //Search::infinite = true;
        else if (token == "wtime") iss >> timeLimits[Color::White];
        else if (token == "btime") iss >> timeLimits[Color::Black];
        else if (token == "winc") iss >> incrementAmount[Color::White];
        else if (token == "binc") iss >> incrementAmount[Color::Black];
        else if (token == "movestogo") { iss >> movesToGo; movesToGo += 2; }
    }
}

void UCI::position(Position&, std::istringstream&)
{
}

void UCI::ponderhit(Position&, std::istringstream&)
{
}

void UCI::displayBoard(Position& pos, std::istringstream&)
{
    sync_cout << pos.displayPositionAsString() << std::endl;
}

void UCI::perft(Position& pos, std::istringstream& iss)
{
    int depth;

    if (iss >> depth)
    {
        const auto result = benchMark.runPerft(pos, depth);
        sync_cout << "info string nodes " << result.first
                  << " time " << result.second
                  << " nps " << (result.first / (result.second + 1)) * 1000 << std::endl;
    }
    else
    {
        sync_cout << "info string argument invalid" << std::endl;
    }
}
