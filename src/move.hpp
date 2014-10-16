#ifndef MOVE_HPP_
#define MOVE_HPP_

#include <cstdint>

class Move
{
public:
    Move() {}

    Move(int from, int to, int promotion, int score):
        score(static_cast<int16_t>(score))
    {
        move = static_cast<uint16_t>(from) | static_cast<uint16_t>(to << 6) | static_cast<uint16_t>(promotion << 12);
    }

    uint16_t getFrom() const { return (move & 0x3f); }
    uint16_t getTo() const { return ((move >> 6) & 0x3f); }
    uint16_t getPromotion() const { return (move >> 12); }
    uint16_t getMove() const { return move; }
    int16_t getScore() const { return score; }
    void setMove(uint16_t newMove) { move = newMove; }
    void setScore(int newScore) { score = static_cast<int16_t>(newScore); }
    bool empty() const { return !move; }
private:
    uint16_t move;
    int16_t score;
};

#endif
