#ifndef SEARCH_HPP_
#define SEARCH_HPP_

#include "movegen.hpp"
#include "tt.hpp"
#include "history.hpp"
#include "killer.hpp"
#include "eval.hpp"
#include "pht.hpp"

class Search
{
public:
    Search(TranspositionTable& transpositionTable, PawnHashTable& pawnHashTable, KillerTable& killerTable, HistoryTable& historyTable);
private:
    TranspositionTable& transpositionTable;
    KillerTable& killerTable;
    HistoryTable& historyTable;
    Evaluation evaluation;
    MoveGen moveGen;

    Search& operator=(const Search&) = delete;
};

#endif