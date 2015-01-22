#ifndef HISTORY_HPP_
#define HISTORY_HPP_

#include <array>
#include "move.hpp"
#include "position.hpp"

// History heuristic for the search function encapsulated.
class HistoryTable
{
public:
    HistoryTable();

    // Change the score of a move which caused a beta-cutoff at the given depth.
    void addCutoff(const Position& pos, const Move& move, int depth);
    // Change the score of a move which failed to cause a beta-cutoff at the given depth when some other move did cause one.
    void addNotCutoff(const Position& pos, const Move& move, int depth);
    // Get the history score for the given move.
    int getScore(const Position& pos, const Move& move) const;
    // Clears the table.
    void clear();
    // From time to time we should age the table to make sure that scores for previous positions won't dominate the scores.
    void age();
private:
    std::array<std::array<int, 64>, 12> history;
    std::array<std::array<int, 64>, 12> butterfly;
};

#endif
