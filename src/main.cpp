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

#include <iostream>
#include <thread>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cctype>
#include <list>
#include <map>
#include "bitboards.hpp"
#include "constants.hpp"
#include "zobrist.hpp"
#include "benchmark.hpp"
#include "search.hpp"
#include "evaluation.hpp"
#include "textio.hpp"
#include "uci.hpp"

char switchCase(unsigned char c)
{
    return static_cast<char>(std::isupper(c) ? std::tolower(c) : std::toupper(c));
}

std::string flipFenString(const std::string& fen)
{
    std::string f, token;
    std::stringstream ss(fen);

    for (auto i = 0; i < 8; i++)
    {
        std::getline(ss, token, i < 7 ? '/' : ' ');
        std::transform(token.begin(), token.end(), token.begin(), switchCase);
        f.insert(0, token + (i ? "/" : " "));
    }

    ss >> token; // Side to move
    f += (token == "w" ? "b " : "w ");

    ss >> token; // Castling flags
    std::transform(token.begin(), token.end(), token.begin(), switchCase);
    f += token + " ";

    ss >> token; // En-passant square
    f += (token == "-" ? token : token.replace(1, 1, token[1] == '3' ? "6" : "3"));

    std::getline(ss, token); // Full and half moves
    f += token;

    return f;
}

bool testReversedEval(std::ifstream& games)
{
    Evaluation evaluation;
    std::string s;

    while (std::getline(games, s))
    {
        Position pos(s);
        Position pos2(flipFenString(s));
        const auto score = evaluation.evaluate(pos);
        const auto flippedScore = evaluation.evaluate(pos2);

        if (score != flippedScore)
        {
            return false;
        }
    }

    return true;
}

int main() 
{
    std::cout << "Hakkapeliitta 3.0 alpha (C) 2013-2015 Mikko Aarnos" << std::endl;
    std::cout << "Detected " << std::max(1u, std::thread::hardware_concurrency()) << " CPU core(s)" << std::endl;

    Bitboards::staticInitialize();
    Zobrist::staticInitialize();
    Evaluation::staticInitialize();

    if (Bitboards::hardwarePopcntSupported())
    {
        std::cout << "Detected hardware POPCNT" << std::endl;
    }

    /*
    std::ifstream games("C:\\GMdraw.txt");
    if (testReversedEval(games))
    {
        std::cout << "Reversed eval passes" << std::endl;
    }
    else
    {
        std::cout << "Reversed eval fails" << std::endl;
    }
    */

    /*
    Position pos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << pos << std::endl;
    try
    {
        const auto res = Benchmark::runPerftTestSuite();
        // const auto res = Benchmark::runPerft(pos, 7);
        std::cout << "Nodes searched: " << res.first << std::endl;
        std::cout << "Time (in ms): " << res.second << std::endl;
        std::cout << "NPS: " << (res.first / res.second) * 1000 << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    */


    UCI uci;

    uci.mainLoop();

    return 0;
}