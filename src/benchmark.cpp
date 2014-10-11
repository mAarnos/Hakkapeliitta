#include "benchmark.hpp"
#include <vector>
#include <iostream>
#include "move.hpp"
#include "movegen.hpp"
#include "stopwatch.hpp"

void Benchmark::runPerft()
{
    Stopwatch sw;
    Position pos;

    pos.initializePositionFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // d7: 3,195,901,860, d8: 84,998,978,956
    // pos.initializePositionFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"); // d5: 193690690
    // pos.initializePositionFromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"); // d7: 178633661
    // pos.initializePositionFromFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -"); // d6: 706045033
    // pos.initializePositionFromFen("rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6"); // d3: 53392
    // pos.initializePositionFromFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"); // d6: 6,923,051,137
    // pos.initializePositionFromFen("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w KQkq - 0 1"); // d6: 8509434052, d7: 390020558283

    sw.start();
    auto perftResult = perft(pos, 8);
    sw.stop();

    std::cout << perftResult << std::endl;
    std::cout << "Time in ms = " << sw.elapsed<std::chrono::milliseconds>() << std::endl;
    std::cout << "NPS = " << perftResult / ((sw.elapsed<std::chrono::milliseconds>() + 1) / 1000.0) << std::endl;
}

uint64_t Benchmark::perft(Position & pos, int depth)
{
    MoveList moves;
    History history;
    auto nodes = 0ull;

    pos.inCheck() ? MoveGen::generateLegalEvasions(pos, moves)
                  : MoveGen::generatePseudoLegalMoves(pos, moves);
    for (auto i = 0; i < moves.size(); ++i)
    {
        const auto & move = moves[i];
        if (!(pos.makeMove(move, history)))
        {
            continue;
        }
        depth == 1 ? ++nodes : nodes += perft(pos, depth - 1);
        pos.unmakeMove(move, history);
    }

    return nodes;
}
