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

/// @file zobrist.hpp
/// @author Mikko Aarnos

#ifndef ZOBRIST_HPP_
#define ZOBRIST_HPP_

#include <cstdint>
#include <array>
#include "piece.hpp"
#include "square.hpp"

/// @brief A hashkey, just like a bitboard, is just a quadword, so let's do a simple typedef for convenience. Again, why am I even commenting this?
using HashKey = uint64_t;

/// @brief Contains all different kinds of zobrist keys used for hashing wildly different things around the program.
///
/// Everything is static due to efficiency reasons.
class Zobrist
{
public:
    /// @brief Initializes the class, must be called before using any other methods.
    static void staticInitialize();

    /// @brief Gets the hash key for a piece at a given square.
    /// @param p The piece.
    /// @param sq The square.
    /// @return The hash key.
    static HashKey pieceHashKey(Piece p, Square sq);

    /// @brief Gets the material hash key for a given piece and multiplicity.
    /// @param p The piece.
    /// @param multiplicity The multiplicity.
    /// @return The material hash key.
    static HashKey materialHashKey(Piece p, int multiplicity);

    /// @brief Gets the castling rights hash key for a given combination of castling rights.
    /// @param castlingRights The castling rights.
    /// @return The castling rights hash key.
    static HashKey castlingRightsHashKey(int castlingRights);

    /// @brief Gets the en passant hash key for a given square. Most squares are never used.
    /// @param enPassant The en passant square.
    /// @return The en passant hash key.
    static HashKey enPassantHashKey(Square enPassant);

    /// @brief Gets the turn hash key.
    /// @return The turn hash key.
    static HashKey turnHashKey() noexcept;

    /// @brief Gets the mangling hash key. That is used in the exclusion search if that is ever implemented.
    /// @return The mangling hash key.
    static HashKey manglingHashKey() noexcept;

private:
    static std::array<std::array<HashKey, 64>, 12> mPieceHashKeys;
    static std::array<std::array<HashKey, 8>, 12> mMaterialHashKeys;
    static std::array<HashKey, 16> mCastlingHashKeys;
    static std::array<HashKey, 64> mEnPassantHashKeys;
    static HashKey mTurnHashKey;
    static HashKey mManglingHashKey;
};

inline HashKey Zobrist::pieceHashKey(Piece p, Square sq) 
{ 
    return mPieceHashKeys[p][sq];
}

inline HashKey Zobrist::materialHashKey(Piece p, int multiplicity)
{
    return mMaterialHashKeys[p][multiplicity];
}

inline HashKey Zobrist::castlingRightsHashKey(int castlingRight)
{ 
    return mCastlingHashKeys[castlingRight];
}

inline HashKey Zobrist::enPassantHashKey(Square enPassant) 
{ 
    return mEnPassantHashKeys[enPassant];
}

inline HashKey Zobrist::turnHashKey() noexcept
{ 
    return mTurnHashKey;
}

inline HashKey Zobrist::manglingHashKey() noexcept
{ 
    return mManglingHashKey;
}

#endif
