#include <iostream>
#include <thread>
#include <algorithm>
#include "utils/synchronized_ostream.hpp"
#include "zobrist.hpp"
#include "bitboard.hpp"
#include "uci.hpp"
#include "killer.hpp"
#include "history.hpp"
#include "pht.hpp"
#include "tt.hpp"
#include "search.hpp"

int main() 
{
    sync_cout << "Hakkapeliitta 2.5, (C) 2013-2015 Mikko Aarnos" << std::endl;
    sync_cout << "Detected " << std::max(1u, std::thread::hardware_concurrency()) << " CPU core(s)" << std::endl;

    Bitboards::initialize();
    Zobrist::initialize();
    KillerTable killerTable;
    HistoryTable historyTable;
    PawnHashTable pawnHashTable;
    TranspositionTable transpositionTable;
    Search search(transpositionTable, pawnHashTable, killerTable, historyTable);
    UCI uci(transpositionTable, pawnHashTable, killerTable, historyTable);

    if (Bitboards::isHardwarePopcntSupported())
    {
        sync_cout << "Detected hardware POPCNT" << std::endl;
    }

    uci.mainLoop();

    return 0;
}
