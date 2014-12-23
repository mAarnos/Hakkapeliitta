#include "uci.hpp"
#include <iostream>
#include <regex>
#include "search.hpp"
#include "eval.hpp"
#include "utils\clamp.hpp"
#include "utils\synchronized_ostream.hpp"
#include "utils\large_pages.hpp"
#include "syzygy\tbprobe.hpp"

Position UCI::root;
ThreadPool UCI::tp(1);

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
    addCommand("displayboard", &UCI::displayBoard);
    root.initializePositionFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // Only works if done last in main.cpp
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

        // If we are currently searching only the commands stop and quit are legal.
        if (Search::searching && currentCommand.getName() != "stop" && currentCommand.getName() != "quit" && currentCommand.getName() != "isready")
        {
            continue;
        }

        // Go through the list of commands and call the correct function if the command entered is known.
        // If the command is unknown report that.
        if (commands.count(currentCommand.getName()))
        {
            (*commands[currentCommand.getName()])(currentCommand);
        }
        else
        {
            sync_cout << "info string unknown command" << std::endl;
        }
    }
}

void UCI::addCommand(std::string name, FunctionPointer fp)
{
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
    sync_cout << "id name Hakkapeliitta 2.0 beta" << std::endl;
    sync_cout << "id author Mikko Aarnos" << std::endl;

    // Send all possible options the engine has that can be modified.
    sync_cout << "option name Hash type spin default 32 min 1 max 65536" << std::endl;
    sync_cout << "option name PawnHash type spin default 4 min 1 max 8192" << std::endl;
    sync_cout << "option name Clear Hash type button" << std::endl;
    sync_cout << "option name Contempt type spin default 0 min -75 max 75" << std::endl;
    sync_cout << "option name SyzygyPath type string default C:\\wdl\\" << std::endl;
    sync_cout << "option name SyzygyProbeLimit type spin default 0 min 0 max 6" << std::endl;
    // sync_cout << "option name LargePages type check default false" << std::endl;

    // Send a response telling the listener that we are ready in UCI-mode.
    sync_cout << "uciok" << std::endl;
}

void UCI::isReady(const Command&)
{
    sync_cout << "readyok" << std::endl;
}

void UCI::stop(const Command&)
{
    Search::searching = false;
    Search::pondering = false;
    Search::infinite = false;
}

void UCI::quit(const Command&)
{
    Search::searching = false; // Send a signal for the search to stop. Without this terminating will finish whenever.
    tp.terminate();
    exit(0);
}

void UCI::setOption(const Command& c)
{
    std::string option, parameter;
    auto arguments = c.getArguments();

    // The string s for setoption comes in the form "name" option "value" parameter.
    // We just ignore "name" and "value" and get option and parameter.
    std::regex expr("\\w*\\s(\\w*)\\s\\w*\\s*(.*)");
    std::smatch matches;
    if (std::regex_search(arguments, matches, expr))
    {
        option = matches[1];
        parameter = matches[2];
    }

    if (option == "Contempt")
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
    else if (option == "Hash" || option == "PawnHash") // Thanks to our parsing pawn hash is shortened to pawn
    {
        int sizeInMegabytes;

        try
        {
            sizeInMegabytes = stoi(parameter);
            sizeInMegabytes = clamp(sizeInMegabytes, 1, option == "Hash" ? 65536 : 8192);
        }
        catch (const std::exception&)
        {
            sizeInMegabytes = (option == "Hash" ? 32 : 4);
        }

        option == "Hash" ? Search::transpositionTable.setSize(sizeInMegabytes)
                         : Evaluation::pawnHashTable.setSize(sizeInMegabytes);
    }
    else if (option == "Clear") // Thanks to our parsing clear hash is shortened to clear. Can't be helped.
    {
        Search::transpositionTable.clear();
        Search::historyTable.clear();
        Search::killerTable.clear();
        Evaluation::pawnHashTable.clear();
        sync_cout << "info string hash cleared" << std::endl;
    }
    else if (option == "SyzygyProbeLimit") 
    {
        try
        {
            Search::syzygyProbeLimit = clamp(stoi(parameter), 0, 6);
        }
        catch (const std::exception&)
        {
            Search::syzygyProbeLimit = 0;
        }
    }
    else if (option == "SyzygyPath") 
    {
        Syzygy::initialize(parameter);
    }
    else if (option == "LargePages")
    {
        if (parameter == "true")
        {
            LargePages::setAllowedToUse(true);
        }
        else if (parameter == "false")
        {
            LargePages::setAllowedToUse(false);
        }
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
    static const auto lagBuffer = 50;
    auto s = c.getArguments();
    auto movesToGo = 25;
    auto moveTime = 0;
    std::array<int, 2> timeLimits = { 0, 0 };
    std::array<int, 2> incrementAmount = { 0, 0 };
    std::istringstream iss(s);
    std::string token;
    Search::searching = true;
    Search::pondering = false;
    Search::infinite = false;

    // Parse the string to get the time limits.
    while (iss >> token)
    {
        if (token == "movetime") iss >> moveTime;
        else if (token == "infinite") Search::infinite = true;
        else if (token == "wtime") iss >> timeLimits[Color::White];
        else if (token == "btime") iss >> timeLimits[Color::Black];
        else if (token == "winc") iss >> incrementAmount[Color::White];
        else if (token == "binc") iss >> incrementAmount[Color::Black];
        else if (token == "movestogo") { iss >> movesToGo; movesToGo += 2; } 
    }

    // Allocate the time limits.
    if (moveTime)
    {
        Search::targetTime = Search::maxTime = moveTime;
    }
    else
    {
        auto time = timeLimits[root.getSideToMove()];
        auto increment = incrementAmount[root.getSideToMove()];
        Search::targetTime = clamp(time / movesToGo + increment - lagBuffer, 1, time - lagBuffer);
        Search::maxTime = clamp(time / 2 + increment, 1, time - lagBuffer);
    }

    // And finally start searching.
    tp.addJob(Search::think, root);
}

void UCI::position(const Command& c)
{
    auto arguments = c.getArguments();
    std::string movesAsAString;
    std::vector<std::string> moves;
    History history;
    Search::repetitionHashesBeforeRoot.clear();

    auto pos = arguments.find("moves");
    if (pos != std::string::npos)
    {
        movesAsAString = arguments.substr(pos + 6);
        arguments = arguments.substr(0, pos - 1);
    }

    if (arguments.find("fen") != std::string::npos)
    {
        root.initializePositionFromFen(arguments.substr(4));
    }
    else
    {
        root.initializePositionFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }

    std::regex expr("([a-z][0-9][a-z][0-9][a-z]?)");
    for (std::sregex_iterator it(movesAsAString.begin(), movesAsAString.end(), expr), end; it != end; ++it)
    {
        moves.push_back((*it)[0]);
    }

    for (auto& move : moves)
    {
        Piece promotion = Piece::Empty;
        auto from = (move[0] - 'a') + 8 * (move[1] - '1');
        auto to = (move[2] - 'a') + 8 * (move[3] - '1');

        if (move.size() == 5)
        {
            switch (move[4])
            {
                case 'q': promotion = Piece::Queen; break;
                case 'r':promotion = Piece::Rook; break;
                case 'b': promotion = Piece::Bishop; break;
                case 'n': promotion = Piece::Knight; break;
                default:;
            }
        }
        else if (((root.getBoard(from) % 6) == Piece::King) && (std::abs(from - to) == 2))
        {
            promotion = Piece::King;
        }
        else if (((root.getBoard(from) % 6) == Piece::Pawn) && (to == root.getEnPassantSquare()))
        {
            promotion = Piece::Pawn;
        }

        Search::repetitionHashesBeforeRoot.insert(root.getHashKey());
        root.makeMove(Move(from, to, promotion, 0), history);
    }
}

void UCI::displayBoard(const Command&)
{
    root.displayBoard();
}
