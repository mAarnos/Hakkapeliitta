#include "benchmark.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "utils/stopwatch.hpp"
#include "utils/synchronized_ostream.hpp"

void Benchmark::runPerft(const Position& root, const int depth)
{
    Position pos(root);
    Stopwatch sw;

    sw.start();
    const auto perftResult = perft(pos, depth);
    sw.stop();

    sync_cout << "Perft result: " << perftResult << std::endl;
    sync_cout << "Time (in ms): " << sw.elapsed<std::chrono::milliseconds>() << std::endl;
    sync_cout << "NPS: " << (perftResult / (sw.elapsed<std::chrono::milliseconds>() + 1)) * 1000 << std::endl;
}

uint64_t Benchmark::perft(const Position& pos, const int depth)
{
    MoveList moves;
    auto nodes = 0ull; 
    const auto inCheck = pos.inCheck();

    inCheck ? MoveGen::generateLegalEvasions(pos, moves) : MoveGen::generatePseudoLegalMoves(pos, moves); 
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
