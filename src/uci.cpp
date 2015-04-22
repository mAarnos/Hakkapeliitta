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

#include "uci.hpp"
#include <iostream>
#include "utils/clamp.hpp"
#include "utils/synchronized_ostream.hpp"
#include "benchmark.hpp"
#include "search_parameters.hpp"

UCI::UCI() :
tp(1), ponder(false),
contempt(0), pawnHashTableSize(4), transpositionTableSize(32), rootPly(0), repetitionHashKeys({})
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
    std::string cmd;

    for (;;)
    {
        // Read a line from stdin.
        if (!std::getline(std::cin, cmd))
            break;

        // Using string stream makes parsing a lot easier than regexes.
        // Note to self: Lucas Braesch is always right.
        std::istringstream iss(cmd);
        std::string commandName;
        iss >> commandName;
        
        // Ignore empty lines.
        if (commandName.empty())
            continue;

        // If we are currently searching only the commands stop, quit, and isready are legal.
        if (search.isSearching() && commandName != "stop" && commandName != "quit" && commandName != "isready" && commandName != "ponderhit")
        {
            continue;
        }

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
    sync_cout << "id name Hakkapeliitta 2.565" << std::endl;
    sync_cout << "id author Mikko Aarnos" << std::endl;

    // Send all possible options the engine has that can be modified.
    sync_cout << "option name Hash type spin default 32 min 1 max 65536" << std::endl;
    sync_cout << "option name Pawn Hash type spin default 4 min 1 max 8192" << std::endl;
    sync_cout << "option name Clear Hash type button" << std::endl;
    sync_cout << "option name Contempt type spin default 0 min -75 max 75" << std::endl;
    // sync_cout << "option name Ponder type check default false" << std::endl;
    // sync_cout << "option name SyzygyPath type string default C:\\wdl\\" << std::endl;
    // sync_cout << "option name SyzygyProbeLimit type spin default 0 min 0 max 6" << std::endl;

    // Send a response telling the listener that we are ready in UCI-mode.
    sync_cout << "uciok" << std::endl;
}

void UCI::isReady(Position&, std::istringstream&)
{
    sync_cout << "readyok" << std::endl;
}

void UCI::stop(Position&, std::istringstream&)
{
    search.stopPondering();
    search.stopSearching();
}

void UCI::quit(Position&, std::istringstream&)
{
    search.stopPondering();
    search.stopSearching();

    // Shut down the thread pool. This part is only here due to a bug in VS.
    tp.terminate();
    exit(0);
}

void UCI::setOption(Position&, std::istringstream& iss) 
{
    std::string name, s;
    
    iss >> s; // Get rid of the "value" in front.

    // Read the name of the option. 
    // Since the name can contains spaces we have this loop.
    while (iss >> s && s != "value")
    {
        name += std::string(" ", !name.empty()) + s;
    }

    if (name == "Contempt")
    {
        iss >> contempt;
        contempt = clamp(contempt, -75, 75);
    }
    else if (name == "Hash")
    {
        iss >> transpositionTableSize;
        search.setTranspositionTableSize(transpositionTableSize);
    }
    else if (name == "Pawn Hash")
    {
        iss >> pawnHashTableSize;
        search.setPawnHashTableSize(pawnHashTableSize);
    }
    else if (name == "Clear Hash")
    {
        search.clearSearch();
    }
    else if (name == "Ponder")
    {
        iss >> ponder;
    }
    else
    {
        sync_cout << "info string no such option exists" << std::endl;
    }
}

void UCI::newGame(Position&, std::istringstream&)
{
    search.clearSearch();
}

void UCI::go(Position& pos, std::istringstream& iss)
{
    SearchParameters searchParameters;
    std::string s;

    // Parse the string to get the parameters for the search.
    while (iss >> s)
    {
        if (s == "searchmoves") {}
        else if (s == "ponder") { searchParameters.ponder = true; }
        else if (s == "wtime") { iss >> searchParameters.time[Color::White]; }
        else if (s == "btime") { iss >> searchParameters.time[Color::Black]; }
        else if (s == "winc") { iss >> searchParameters.increment[Color::White]; }
        else if (s == "binc") { iss >> searchParameters.increment[Color::Black]; }
        else if (s == "movestogo") { iss >> searchParameters.movesToGo; searchParameters.movesToGo += 2; }
        else if (s == "depth") { iss >> searchParameters.depth; }
        else if (s == "nodes") { iss >> searchParameters.nodes; }
        else if (s == "mate") { searchParameters.mate = true; }
        else if (s == "movetime") { iss >> searchParameters.moveTime; }
        else if (s == "infinite") { searchParameters.infinite = true; }
    }

    tp.addJob(&Search::think, &search, pos, searchParameters, rootPly, repetitionHashKeys, contempt);
}

void UCI::position(Position& pos, std::istringstream& iss)
{
    std::string s, fen;
    rootPly = 0;

    iss >> s;
    
    if (s == "startpos")
    {
        fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        iss >> s; // Get rid of "moves" which should follow this
    }
    else if (s == "fen")
    {
        while (iss >> s && s != "moves")
        {
            fen += s + " ";
        }
    }
    else
    {
        return;
    }

    pos = Position(fen);

    // Parse the moves.
    while (iss >> s)
    {
        Piece promotion = Piece::Empty;
        auto from = (s[0] - 'a') + 8 * (s[1] - '1');
        auto to = (s[2] - 'a') + 8 * (s[3] - '1');

        if (s.size() == 5)
        {
            switch (s[4])
            {
                case 'q': promotion = Piece::Queen; break;
                case 'r':promotion = Piece::Rook; break;
                case 'b': promotion = Piece::Bishop; break;
                case 'n': promotion = Piece::Knight; break;
                default:;
            }
        }
        else if (getPieceType(pos.getBoard(from)) == Piece::King && std::abs(from - to) == 2)
        {
            promotion = Piece::King;
        }
        else if (getPieceType(pos.getBoard(from)) == Piece::Pawn && to == pos.getEnPassantSquare())
        {
            promotion = Piece::Pawn;
        }

        repetitionHashKeys[rootPly++] = pos.getHashKey();
        pos.makeMove(Move(from, to, promotion, 0));
    }
}

void UCI::ponderhit(Position&, std::istringstream&)
{
    search.stopPondering();
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
