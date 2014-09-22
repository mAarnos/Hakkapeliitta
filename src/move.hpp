#ifndef MOVE_HPP_
#define MOVE_HPP_

#include <cstdint>

class Move
{
public:
    Move(int32_t from = 0, int32_t to = 0, int32_t promotion = 0, int32_t score = 0);

    int32_t getFrom() const { return from; }
    int32_t getTo() const { return to; }
    int32_t getPromotion() const { return promotion; }
    int32_t getScore() const { return score; }
    int16_t getPacket() const { return static_cast<int16_t>(from & (to << 6) & (promotion << 12)); }
    void setScore(int32_t newScore) { score = newScore; }
private:
    int32_t from;
    int32_t to;
    int32_t promotion;
    int32_t score;
};

#endif
