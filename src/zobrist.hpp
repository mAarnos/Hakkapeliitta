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

#ifndef ZOBRIST_HPP_
#define ZOBRIST_HPP_

#include <cstdint>
#include <array>
#include "piece.hpp"
#include "square.hpp"

using HashKey = uint64_t;

class Zobrist
{
public:
    static void initialize();

    static HashKey pieceHashKey(const Piece p, const Square sq) { return piece[p][sq]; }
    static HashKey materialHashKey(const Piece p, const int amount) { return material[p][amount]; }
    static HashKey castlingRightsHashKey(const int castlingRight) { return castling[castlingRight]; }
    static HashKey enPassantHashKey(const Square enPassant) { return ep[enPassant]; }
    static HashKey turnHashKey() { return turn; }
    static HashKey mangleHashKey() { return mangle; }
private:
    static std::array<std::array<HashKey, 64>, 12> piece;
    static std::array<std::array<HashKey, 8>, 12> material;
    static std::array<HashKey, 16> castling;
    static std::array<HashKey, 64> ep;
    static HashKey turn;
    static HashKey mangle;
};

#endif
