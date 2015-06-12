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
#include "bitboards.hpp"

std::array<std::array<HashKey, 64>, 12> Zobrist::mPieceHashKeys;
std::array<std::array<HashKey, 8>, 12> Zobrist::mMaterialHashKeys;
std::array<HashKey, 16> Zobrist::mCastlingHashKeys;
std::array<HashKey, 64> Zobrist::mEnPassantHashKeys;
HashKey Zobrist::mTurnHashKey;
HashKey Zobrist::mManglingHashKey;

void Zobrist::staticInitialize()
{
    std::mt19937_64 rng(123456789); // TODO: use std::random_device?

    for (Piece p = Piece::WhitePawn; p <= Piece::BlackKing; ++p)
    {
        for (Square sq = Square::A1; sq <= Square::H8; ++sq)
        {
            mPieceHashKeys[p][sq] = rng();
        }
        for (auto j = 0; j < 8; ++j)
        {
            mMaterialHashKeys[p][j] = rng();
        }
    }

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        mEnPassantHashKeys[sq] = rng();
    }

    for (auto cr = 0; cr <= 15; ++cr)
    {
        Bitboard castlingRight = cr;
        while (castlingRight)
        {
            const auto key = mCastlingHashKeys[1ULL << Bitboards::popLsb(castlingRight)];
            mCastlingHashKeys[cr] ^= key ? key : rng();
        }
    }

    mTurnHashKey = rng();
    mManglingHashKey = rng();
}
