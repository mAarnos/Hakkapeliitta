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
    // UCI uci(transpositionTable, pawnHashTable, killerTable, historyTable);

    if (Bitboards::isHardwarePopcntSupported())
    {
        sync_cout << "Detected hardware POPCNT" << std::endl;
    }

    /*
    Position pos;
    std::ifstream ifs("C:\\GMblackwin.txt");
    std::ofstream res("results.txt");
    std::string s;

    auto count = 1;
    while (std::getline(ifs, s))
    {
        pos = Position(s);
        if (count == 24337)
            std::cout << pos.displayPositionAsString() << std::endl;
        auto score = search.quiescenceSearch(pos, 0, 0, -100000, 100000, pos.inCheck());
        res << score << std::endl;
        std::cout << count++ << "\n";
    }
    */

    // uci.mainLoop();

    return 0;
}
