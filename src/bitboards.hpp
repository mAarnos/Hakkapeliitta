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

/// @file bitboards.hpp
/// @author Mikko Aarnos

#ifndef BITBOARDS_HPP_
#define BITBOARDS_HPP_

#include <cstdint>
#include <cassert>
#include <array>
#include "square.hpp"
#include "color.hpp"
#include "piece.hpp"

/// @brief A bitboard is just a quadword, so let's do a simple typedef for convenience. Why am I even commenting this?
using Bitboard = uint64_t;

/// @brief Contains different kinds of bit-manipulation routines and bitboards used throughout the program for wildly different things.
///
/// Everything is static due to efficiency reasons.
class Bitboards
{
public:
    /// @brief Initializes the class, must be called before using any other methods.
    static void staticInitialize();

    /// @brief Calculates the bishop attacks from a square with the given occupied squares.
    /// @param sq The square.
    /// @param occupied The currently occupied squares.
    /// @return A bitboard containing the possible bishop attacks.
    static Bitboard bishopAttacks(Square sq, Bitboard occupied);

    /// @brief Calculates the rook attacks from a square with the given occupied squares.
    /// @param sq The square.
    /// @param occupied The currently occupied squares.
    /// @return A bitboard containing the possible rook attacks.
    static Bitboard rookAttacks(Square sq, Bitboard occupied);

    /// @brief Calculates the queen attacks from a square with the given occupied squares.
    /// @param sq The square.
    /// @param occupied The currently occupied squares.
    /// @return A bitboard containing the possible queen attacks.
    static Bitboard queenAttacks(Square sq, Bitboard occupied);

    /// @brief Get a bitboard with the bit corresponding to the given square set.
    /// @param sq The square.
    /// @return A bitboard which is equivalent to 1ULL << sq.
    static Bitboard bit(Square sq);

    /// @brief Get the king attacks possible from a given square.
    /// @param sq The square.
    /// @return A bitboard contatining the possible king attacks.
    static Bitboard kingAttacks(Square sq);

    /// @brief Get the knight attacks possible from a given square.
    /// @param sq The square.
    /// @return A bitboard containing the possible knight attacks.
    static Bitboard knightAttacks(Square sq);

    /// @brief Get the pawn attacks for a given color from a given square.
    /// @param c The color.
    /// @param sq The square.
    /// @return A bitboard containing the possible pawn attacks.
    static Bitboard pawnAttacks(Color c, Square sq);

    /// @brief Gets a bitboard with all squares between the given squares marked.
    /// @param from The from-square.
    /// @param to The to-square.
    /// @return The bitboard. If the given squares are not in a line the bitboard will be empty.
    static Bitboard squaresBetween(Square from, Square to);

    /// @brief Gets a bitboard with all squares in the line formed by the two given squares marked.
    /// @param from The from-square.
    /// @param to The to-square.
    /// @return The bitboard. If the given squares are not in a line the bitboard will be empty.
    static Bitboard lineFormedBySquares(Square from, Square to);

    /// @brief Gets a bitboard with all squares to some direction from a given square marked.
    /// @param direction The direction of the ray. 
    /// @param sq The square.
    /// @return The bitboard. 
    static Bitboard ray(int direction, Square sq);

    /// @brief Gets a bitboard for detecting if a pawn of a given color on a given square is passed.
    /// @param c The color.
    /// @param sq The square.
    /// @return A bitboard with all squares in front of the pawn on the same or adjacent files marked. If the marked squares contain no friendly pawns the given pawn is passed.
    static Bitboard passedPawn(Color c, Square sq);

    /// @brief Gets a bitboard for detecting if a pawn of a given color on a given square is backward.
    /// @param c The color.
    /// @param sq The square.
    /// @return A bitboard which has the adjacent files marked as long as these files are <= the rank of the pawn (taking into account the color).
    static Bitboard backwardPawn(Color c, Square sq);

    /// @brief Gets a bitboard for detecting if a pawn on a given square is isolated.
    /// @param sq The square.
    /// @return A bitboard which has the adjacent files marked. If the marked squares contain no friendly pawns the given pawn is isolated.
    static Bitboard isolatedPawn(Square sq);

    /// @brief Get the zone used for king safety calculations for a given color and square.
    /// @param c The color.
    /// @param sq The square.
    /// @return A bitboard which has the square of the king, all king attacks from that square and three (or two) squares in front of those marked.
    static Bitboard kingSafetyZone(Color c, Square sq);

    /// @brief An array containing all rank bitboards.
    static const std::array<Bitboard, 8> ranks;

    /// @brief An array containing all file bitboards.
    static const std::array<Bitboard, 8> files;

    /// @brief Returns the amount of set bits in a given bitboard.
    /// @tparam hardwarePopcntEnabled A switch for changing between software and hardware POPCNT.
    /// @param bb The bitboard.
    /// @return The amount of set bits.
    template <bool hardwarePopcntEnabled>
    static int popcnt(Bitboard bb) noexcept;

    /// @brief Returns the least significant set bit in a given bitboard. Assumes that at least one bit is set.
    /// @param bb The bitboard.
    /// @return The least significant bit, little endian order.
    static unsigned long lsb(Bitboard bb);

    /// @brief Returns the most significant set bit in a given bitboard. Assumes that at least one bit is set.
    /// @param bb The bitboard.
    /// @return The most significant bit, little endian order.
    static unsigned long msb(Bitboard bb);

    /// @brief Resets and returns the least significant set bit in a given bitboard. Assumes that at least one bit is set.
    /// @param bb The bitboard.
    /// @return The least significant bit, little endian order.
    static int popLsb(Bitboard& bb);

    /// @brief Tests whether a given bitboard has more than one bit set.
    /// @param bb The bitboard.
    /// @return True if the bitboard has more than one bit set, false otherwise.
    static bool moreThanOneBitSet(Bitboard bb) noexcept;

    /// @brief Tests whether a given bitboard has a particular bit set.
    /// @param bb The bitboard.
    /// @param b The bit to check.
    /// @return True if the bit is set, false otherwise.
    static bool testBit(Bitboard bb, int b);

    /// @brief Sets the specified bit of a bitboard to 1.
    /// @param bb The bitboard.
    /// @param b The bit to set. 
    static void setBit(Bitboard& bb, int b);

    /// @brief Sets the specified bit of the given bitboard to 0. We assume that the bit is already 1, otherwise this doesn't work.
    /// @param bb The bitboard.
    /// @param b The bit to clear.
    static void clearBit(Bitboard& bb, int b);

    /// @brief Used for generating different piece attacks easily.
    /// @param sideToMove The side to generate moves for. Only used by pawn attacks.
    /// @param piece The piece type to generate attacks for.
    /// @param sq The square of the piece.
    /// @param occupied The current occupied squares.
    /// @return A bitboard containing the piece attacks.
    static Bitboard pieceAttacks(Color sideToMove, Piece piece, Square sq, Bitboard occupied);

    /// @brief Used for checking if the processor we are running on supports hardware POPCNT.
    /// @return True if hardware POPCNT is supported, false otherwise.
    static bool hardwarePopcntSupported() noexcept;

private:
    struct Magic
    {
        Bitboard* mData;
        Bitboard mMask;
        Bitboard mMagic;
    };

    struct MagicInit
    {
        Bitboard mMagic;
        int32_t mIndex;
    };

    static int hardwarePopcnt(Bitboard bb) noexcept;
    static int softwarePopcnt(Bitboard bb) noexcept;

    static std::array<Magic, 64> mBishopMagics;
    static std::array<Magic, 64> mRookMagics;
    static std::array<Bitboard, 97264> mLookupTable;
    static const std::array<MagicInit, 64> mRookInit;
    static const std::array<MagicInit, 64> mBishopInit;

    static void initializeMagics(const std::array<MagicInit, 64>& magicInit, std::array<Magic, 64>& magic, 
                                 const std::array<std::array<int, 2>, 4>& dir, int shift);

    static std::array<Bitboard, 64> mBits;
    static std::array<Bitboard, 64> mKingAttacks;
    static std::array<Bitboard, 64> mKnightAttacks;
    static std::array<std::array<Bitboard, 64>, 2> mPawnAttacks;
    static std::array<std::array<Bitboard, 64>, 64> mLines;
    static std::array<std::array<Bitboard, 64>, 64> mBetween;
    static std::array<std::array<Bitboard, 64>, 8> mRays;
    static std::array<std::array<Bitboard, 64>, 2> mPassed;
    static std::array<std::array<Bitboard, 64>, 2> mBackward;
    static std::array<Bitboard, 64> mIsolated;
    static std::array<std::array<Bitboard, 64>, 2> mKingZone;

#if !(defined _WIN64 || defined __x86_64__)
    static const std::array<int, 64> mIndex;
#endif

    static bool mHardwarePopcntSupported;
};

inline Bitboard Bitboards::bishopAttacks(Square sq, Bitboard occupied)
{
    const auto& mag = mBishopMagics[sq];
    return mag.mData[((occupied & mag.mMask) * mag.mMagic) >> (64 - 9)];
}

inline Bitboard Bitboards::rookAttacks(Square sq, Bitboard occupied)
{
    const auto& mag = mRookMagics[sq];
    return mag.mData[((occupied & mag.mMask) * mag.mMagic) >> (64 - 12)];
}

inline Bitboard Bitboards::queenAttacks(Square sq, Bitboard occupied)
{
    return (bishopAttacks(sq, occupied) | rookAttacks(sq, occupied));
}

inline Bitboard Bitboards::bit(Square sq) 
{ 
    return mBits[sq]; 
}

inline Bitboard Bitboards::kingAttacks(Square sq) 
{ 
    return mKingAttacks[sq]; 
}

inline Bitboard Bitboards::knightAttacks(Square sq) 
{ 
    return mKnightAttacks[sq]; 
}

inline Bitboard Bitboards::pawnAttacks(Color c, Square sq)
{ 
    return mPawnAttacks[c][sq]; 
}

inline Bitboard Bitboards::squaresBetween(Square from, Square to)
{ 
    return mBetween[from][to]; 
}

inline Bitboard Bitboards::lineFormedBySquares(Square from, Square to) 
{ 
    return mLines[from][to]; 
}

inline Bitboard Bitboards::ray(int direction, Square sq) 
{ 
    return mRays[direction][sq]; 
}

inline Bitboard Bitboards::passedPawn(Color c, Square sq) 
{ 
    return mPassed[c][sq]; 
}

inline Bitboard Bitboards::backwardPawn(Color c, Square sq) 
{ 
    return mBackward[c][sq]; 
}

inline Bitboard Bitboards::isolatedPawn(Square sq) 
{
    return mIsolated[sq]; 
}

inline Bitboard Bitboards::kingSafetyZone(Color c, Square sq) 
{ 
    return mKingZone[c][sq]; 
}

template <bool hardwarePopcntEnabled>
inline int Bitboards::popcnt(Bitboard bb) noexcept
{
    return (hardwarePopcntEnabled ? hardwarePopcnt(bb) : softwarePopcnt(bb));
}

inline unsigned long Bitboards::lsb(Bitboard bb)
{
    // ia, ia, foo bar fhtagn
#if (defined _WIN64 || defined __x86_64__)
    assert(bb);
 #ifdef _MSC_VER
    unsigned long index;
    _BitScanForward64(&index, bb);
    return index;
 #else
    __asm__("bsfq %0, %0" : "=r" (bb) : "0" (bb));
    return static_cast<unsigned long>(bb);
 #endif
#else
    return mIndex[((bb ^ (bb - 1)) * 0x03f79d71b4cb0a89) >> 58];
#endif
}

inline unsigned long Bitboards::msb(Bitboard bb)
{
#if (defined _WIN64 || defined __x86_64__)
    assert(bb);
 #ifdef _MSC_VER
    unsigned long index;
    _BitScanReverse64(&index, bb);
    return index;
 #else
    __asm__("bsrq %0, %0" : "=r" (bb) : "0" (bb));
    return static_cast<unsigned long>(bb);
 #endif
#else
    bb |= bb >> 1;
    bb |= bb >> 2;
    bb |= bb >> 4;
    bb |= bb >> 8;
    bb |= bb >> 16;
    bb |= bb >> 32;
    return mIndex[(bb * 0x03f79d71b4cb0a89) >> 58];
#endif
}

inline int Bitboards::popLsb(Bitboard& bb)
{
    const auto ret = lsb(bb);
    bb &= (bb - 1);
    return ret;
}

inline bool Bitboards::moreThanOneBitSet(Bitboard bb) noexcept
{
    return (bb & (bb - 1)) != 0;
}

inline bool Bitboards::testBit(Bitboard bb, int b)
{
    return (bb & mBits[b]) != 0;
}

inline void Bitboards::setBit(Bitboard& bb, int b)
{
    bb |= mBits[b];
}

inline void Bitboards::clearBit(Bitboard& bb, int b)
{
    assert(testBit(bb, b));
    bb ^= mBits[b];
}

inline Bitboard Bitboards::pieceAttacks(Color sideToMove, Piece piece, Square sq, Bitboard occupied)
{
    assert(piece.pieceTypeIsOk());

    if (piece == Piece::Pawn) return pawnAttacks(sideToMove, sq);
    if (piece == Piece::Knight) return knightAttacks(sq);
    if (piece == Piece::Bishop) return bishopAttacks(sq, occupied);
    if (piece == Piece::Rook) return rookAttacks(sq, occupied);
    if (piece == Piece::Queen) return queenAttacks(sq, occupied);
    return kingAttacks(sq);
}

inline bool Bitboards::hardwarePopcntSupported() noexcept
{ 
    return mHardwarePopcntSupported;
}

inline int Bitboards::hardwarePopcnt(Bitboard bb) noexcept
{
#if (defined _WIN64 || defined __x86_64__)
 #if (defined __clang__ || defined __GNUC__)
    __asm__("popcnt %1, %0" : "=r" (bb) : "0" (bb));
    return static_cast<int>(bb);
 #else
    return static_cast<int>(_mm_popcnt_u64(bb));
 #endif
#else
    assert(false);
    return bb != 0; // gets rid of unreferenced formal parameter warning
#endif
}

inline int Bitboards::softwarePopcnt(Bitboard bb) noexcept
{
    bb -= ((bb >> 1) & 0x5555555555555555);
    bb = (bb & 0x3333333333333333) + ((bb >> 2) & 0x3333333333333333);
    bb = (bb + (bb >> 4)) & 0x0f0f0f0f0f0f0f0f;
    return (bb * 0x0101010101010101) >> 56;
}

#endif
