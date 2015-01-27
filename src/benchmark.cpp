#include "benchmark.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "utils/stopwatch.hpp"

std::pair<uint64_t, uint64_t> Benchmark::runPerft(const Position root, const int depth)
{
    Stopwatch sw;

    sw.start();
    const auto perftResult = perft(root, depth);
    sw.stop();

    return std::make_pair(perftResult, sw.elapsed<std::chrono::milliseconds>());
}

uint64_t Benchmark::perft(const Position& pos, const int depth)
{
    MoveList moves;
    auto nodes = 0ull; 
    const auto inCheck = pos.inCheck();

    inCheck ? moveGen.generateLegalEvasions(pos, moves) : moveGen.generatePseudoLegalMoves(pos, moves);
    for (auto i = 0; i < moves.size(); ++i)
    {
        const auto& move = moves[i];
        if (!pos.legal(move, inCheck))
        {
            continue;
        }

        /*
        if (depth == 1)
        {
            ++nodes;
            continue;
        }
        */

        Position newPos(pos);
        newPos.makeMove(move);
        nodes += depth == 1 ? 1 : perft(newPos, depth - 1);
    }

    return nodes;
}
