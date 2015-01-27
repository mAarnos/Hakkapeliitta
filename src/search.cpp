#include "search.hpp"

Search::Search(TranspositionTable& transpositionTable, PawnHashTable& pawnHashTable, KillerTable& killerTable, HistoryTable& historyTable) :
transpositionTable(transpositionTable), killerTable(killerTable), historyTable(historyTable), evaluation(pawnHashTable)
{
}

