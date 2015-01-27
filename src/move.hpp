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

#ifndef MOVE_HPP_
#define MOVE_HPP_

#include <cstdint>
#include <cassert>
#include "square.hpp"
#include "piece.hpp"

// Represents a single move.
// Contains not only the move but also the score for the move. 
// This score is used to order moves in the search function.
class Move
{
public:
    Move();
    Move(Square from, Square to, Piece promotion, int32_t score);

    Square getFrom() const;
    Square getTo() const;
    Piece getPromotion() const;
    uint16_t getMove() const;
    int16_t getScore() const;
    void setMove(uint16_t newMove);
    void setScore(int16_t newScore);
    bool empty() const; 
private:
    uint16_t move;
    int16_t score;
};

inline Move::Move() : move(0), score(0)
{
}

inline Move::Move(const Square from, const Square to, const Piece promotion, const int32_t iScore)
{
    assert(squareIsOkStrict(from) && squareIsOkStrict(to) && pieceIsOk(promotion));
    score = static_cast<int16_t>(iScore);
    move = static_cast<uint16_t>(from) | static_cast<uint16_t>(to << 6) | static_cast<uint16_t>(promotion << 12);
}

inline Square Move::getFrom() const 
{ 
    return (move & 0x3f); 
}

inline Square Move::getTo() const 
{ 
    return ((move >> 6) & 0x3f); 
}

inline Piece Move::getPromotion() const 
{ 
    return (move >> 12); 
}

inline uint16_t Move::getMove() const
{ 
    return move; 
}

inline int16_t Move::getScore() const 
{ 
    return score; 
}

inline void Move::setMove(const uint16_t newMove) 
{ 
    move = newMove; 
}

inline void Move::setScore(const int16_t newScore) 
{ 
    score = newScore; 
}

inline bool Move::empty() const 
{ 
    return !move; 
}

#endif
