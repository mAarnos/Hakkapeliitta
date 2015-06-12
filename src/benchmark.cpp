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

#include "benchmark.hpp"
#include <sstream>
#include "utils\stopwatch.hpp"

std::pair<uint64_t, uint64_t> Benchmark::runPerft(const Position& pos, int depth)
{
    Stopwatch sw;

    sw.start();
    const auto perftResult = perft(pos, depth, pos.inCheck());
    sw.stop();

    return std::make_pair(perftResult, sw.elapsed<std::chrono::milliseconds>());
}

std::pair<uint64_t, uint64_t> Benchmark::runPerftTestSuite()
{
    struct PerftTest
    {
        std::string fen;
        int depth;
        uint64_t result;
    };

    static const std::array<PerftTest, 7> tests = { {
        { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 7, 3195901860 },
        { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 5, 193690690 },
        { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 7, 178633661 },
        { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -", 6, 706045033 },
        { "rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6", 3, 53392 },
        { "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 6, 6923051137 },
        { "rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w KQkq - 0 1", 6, 8509434052 },
    } };

    auto total = 0ULL;
    Stopwatch sw;

    sw.start();
    for (auto i = 0; i < 7; ++i)
    {
        auto& test = tests[i];
        Position pos(test.fen);
        const auto result = perft(pos, test.depth, pos.inCheck());
        total += result;
        if (result != test.result)
        {
            std::stringstream ss;
            ss << "Test " << i + 1 << " failed: should have been " << test.result << " but was " << result;
            throw std::exception(ss.str().c_str());
        }
    }
    sw.stop();

    return std::make_pair(total, sw.elapsed<std::chrono::milliseconds>());
}

uint64_t Benchmark::perft(const Position& pos, int depth, bool inCheck)
{
    MoveList moveList;
    auto nodes = 0ULL; 

    inCheck ? MoveGen::generateLegalEvasions(pos, moveList) : MoveGen::generatePseudoLegalMoves(pos, moveList);
    for (auto i = 0; i < moveList.size(); ++i)
    {
        const auto move = moveList.getMove(i);
        if (!pos.legal(move, inCheck))
        {
            continue;
        }

        if (depth == 1)
        {
            ++nodes;
            continue;
        }

        Position newPos(pos);
        newPos.makeMove(move);
        nodes += depth == 1 ? 1 : perft(newPos, depth - 1, newPos.inCheck());
    }

    return nodes;
}
