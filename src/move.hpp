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
#include <array>
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
    int32_t getScore() const;
    void setMove(uint16_t newMove);
    void setScore(int32_t newScore);
    bool empty() const; 
private:
    uint16_t move;
    int32_t score;
};

inline Move::Move() : move(0), score(0)
{
}

inline Move::Move(const Square from, const Square to, const Piece promotion, const int32_t iScore)
{
    assert(squareIsOkStrict(from) && squareIsOkStrict(to) && pieceIsOk(promotion));
    score = iScore;
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

inline int32_t Move::getScore() const 
{ 
    return score; 
}

inline void Move::setMove(const uint16_t newMove) 
{ 
    move = newMove; 
}

inline void Move::setScore(const int32_t newScore) 
{ 
    score = newScore; 
}

inline bool Move::empty() const 
{ 
    return !move; 
}

inline std::string moveToUciFormat(const Move& move)
{
    const static std::array<std::string, 64> squareToNotation = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    };

    const static std::array<std::string, 64> promotionToNotation = {
        "p", "n", "b", "r", "q", "k"
    };

    std::string s;
    const auto from = move.getFrom();
    const auto to = move.getTo();
    const auto promotion = move.getPromotion();

    s += squareToNotation[from] + squareToNotation[to];
    if (promotion != Piece::Empty && promotion != Piece::King && promotion != Piece::Pawn)
    {
        s += promotionToNotation[promotion];
    }

    return s;
}

#endif
