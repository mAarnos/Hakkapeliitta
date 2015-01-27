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

#include "zobrist.hpp"
#include <random>
#include "square.hpp"
#include "piece.hpp"
#include "bitboard.hpp"

std::array<std::array<HashKey, 64>, 12> Zobrist::piece;
std::array<std::array<HashKey, 8>, 12> Zobrist::material;
std::array<HashKey, 16> Zobrist::castling;
std::array<HashKey, 64> Zobrist::ep;
HashKey Zobrist::turn;
HashKey Zobrist::mangle;

void Zobrist::initialize()
{
    std::mt19937_64 rng(123456789);

    for (Piece p = Piece::WhitePawn; p <= Piece::BlackKing; ++p)
    {
        for (Square sq = Square::A1; sq <= Square::H8; ++sq)
        {
            piece[p][sq] = rng(); 
        }
        for (auto j = 0; j < 8; ++j)
        {
            material[p][j] = rng(); 
        }
    }

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        ep[sq] = rng(); 
    }

    for (auto cr = 0; cr <= 15; ++cr)
    {
        Bitboard castlingRight = cr;
        while (castlingRight)
        {
            const auto key = castling[1ull << Bitboards::popLsb(castlingRight)];
            castling[cr] ^= key ? key : rng(); 
        }
    }

    turn = rng(); 
    mangle = rng();
}
