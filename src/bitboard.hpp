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

#ifndef BITBOARD_HPP_
#define BITBOARD_HPP_

#include <cstdint>
#include <cassert>
#include <array>
#include "square.hpp"
#include "color.hpp"
#include "piece.hpp"

using Bitboard = uint64_t;

class Bitboards
{
public:
    static void initialize();

    static Bitboard bishopAttacks(const Square sq, const Bitboard occupied)
    {
        const auto& mag = bishopMagic[sq];
        return mag.data[((occupied & mag.mask) * mag.magic) >> (64 - 9)];
    }

    static Bitboard rookAttacks(const Square sq, const Bitboard occupied)
    {
        const auto& mag = rookMagic[sq];
        return mag.data[((occupied & mag.mask) * mag.magic) >> (64 - 12)];
    }

    static Bitboard queenAttacks(const Square sq, const Bitboard occupied)
    {
        return (bishopAttacks(sq, occupied) | rookAttacks(sq, occupied));
    }

    static Bitboard bit(const Square sq) { return bits[sq]; }
    static Bitboard kingAttacks(const Square sq) { return kingAttack[sq]; }
    static Bitboard knightAttacks(const Square sq) { return knightAttack[sq]; }
    static Bitboard pawnAttacks(const Color c, const Square sq) { return pawnAttack[c][sq]; }
    static Bitboard squaresBetween(const Square from, const Square to) { return between[from][to]; }
    static Bitboard lineFormedBySquares(const Square from, const Square to) { return line[from][to]; }
    static Bitboard ray(const int direction, const Square sq) { return rays[direction][sq]; }
    static Bitboard passedPawn(const Color c, const Square sq) { return passed[c][sq]; }
    static Bitboard backwardPawn(const Color c, const Square sq) { return backward[c][sq]; }
    static Bitboard isolatedPawn(const Square sq) { return isolated[sq]; }
    static Bitboard kingSafetyZone(const Color c, const Square sq) { return kingZone[c][sq]; }

    static const std::array<Bitboard, 8> ranks;
    static const std::array<Bitboard, 8> files;

    // Returns the amount of set bits in bb.
    // We have two versions, one using hardware and the other using sofware.
    // We automatically detect which one to use.
    template <bool hardwarePopcntEnabled>
    static inline int popcnt(const Bitboard bb)
    {
        return (hardwarePopcntEnabled ? hardwarePopcnt(bb) : softwarePopcnt(bb));
    }

    // Returns the least significant set bit in the mask.
    static unsigned long lsb(const Bitboard bb)
    {
#if (defined _WIN64 || defined __x86_64__)
        assert(bb);
 #ifdef _MSC_VER
        unsigned long index;
        _BitScanForward64(&index, bb);
        return index;
 #else
        __asm__ ("bsfq %0, %0" : "=r" (bb) : "0" (bb));
        return static_cast<unsigned long>(bb);
 #endif
#else
        return index[((bb ^ (bb - 1)) * 0x03f79d71b4cb0a89) >> 58];
#endif
    }

    // Returns the most significant set bit in the mask.
    static unsigned long msb(const Bitboard bb)
    {
#if (defined _WIN64 || defined __x86_64__)
        assert(bb);
 #ifdef _MSC_VER
        unsigned long index;
        _BitScanReverse64(&index, bb);
        return index;
 #else
        __asm__ ("bsrq %0, %0" : "=r" (bb) : "0" (bb));
        return static_cast<unsigned long>(bb);
 #endif
#else
        bb |= bb >> 1;
        bb |= bb >> 2;
        bb |= bb >> 4;
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;
        return index[(bb * 0x03f79d71b4cb0a89) >> 58];
#endif
    }

    static int popLsb(Bitboard& bb)
    {
        auto ret = lsb(bb);
        bb &= (bb - 1);
        return ret;
    }

    // Tests whether the input bitboard has more than one bit set.
    static bool moreThanOneBitSet(const Bitboard bb)
    {
        return (bb & (bb - 1)) != 0;
    }

    // Tests whether the bitboard has a particular bit set.
    static bool testBit(const Bitboard bb, const int b)
    {
        return (bb & bits[b]) != 0;
    }

    // Sets the specified bit to 1.
    static void setBit(Bitboard& bb, const int b)
    {
        bb |= bits[b];
    }

    // Sets the specified bit to 0. 
    // We assume that the bit is already 1, otherwise this doesn't work.
    static void clearBit(Bitboard& bb, const int b)
    {
        assert(testBit(bb, b));
        bb ^= bits[b];
    }

    static Bitboard pieceAttacks(const Color sideToMove, const Piece piece, const Square sq, const Bitboard occupied)
    {
        assert(pieceTypeIsOk(piece));

        if (piece == Piece::Pawn) return pawnAttacks(sideToMove, sq);
        if (piece == Piece::Knight) return knightAttacks(sq);
        if (piece == Piece::Bishop) return bishopAttacks(sq, occupied);
        if (piece == Piece::Rook) return rookAttacks(sq, occupied);
        if (piece == Piece::Queen) return queenAttacks(sq, occupied);
        if (piece == Piece::King) return kingAttacks(sq);

        assert(false);
        return 0;
    }

    static bool isHardwarePopcntSupported() { return hardwarePopcntSupported; }
private:
    class Magic
    {
    public:
        Bitboard* data;
        Bitboard mask;
        Bitboard magic;
    };

    class MagicInit
    {
    public:
        Bitboard magic;
        int32_t index;
    };

    static int hardwarePopcnt(const Bitboard bb)
    {
#if (defined _WIN64 || defined __x86_64__)
 #if (defined __clang__ || defined __GNUC__)
        __asm__ ("popcnt %1, %0" : "=r" (bb) : "0" (bb));
        return static_cast<int>(bb);
 #else
        return static_cast<int>(_mm_popcnt_u64(bb));
 #endif
#else
        assert(false);
        return bb != 0; // gets rid of unreferenced formal parameter warning
#endif
    }

    static int softwarePopcnt(Bitboard bb)
    {
        bb -= ((bb >> 1) & 0x5555555555555555);
        bb = (bb & 0x3333333333333333) + ((bb >> 2) & 0x3333333333333333);
        bb = (bb + (bb >> 4)) & 0x0f0f0f0f0f0f0f0f;
        return (bb * 0x0101010101010101) >> 56;
    }

    static std::array<Magic, 64> bishopMagic;
    static std::array<Magic, 64> rookMagic;
    static std::array<Bitboard, 97264> lookupTable;
    const static std::array<MagicInit, 64> rookInit;
    const static std::array<MagicInit, 64> bishopInit;

    static void initMagics(const std::array<MagicInit, 64>& magicInit, std::array<Magic, 64>& magic, 
                           const std::array<std::array<int, 2>, 4>& dir, int shift);

    static std::array<Bitboard, 64> bits;
    static std::array<Bitboard, 64> kingAttack;
    static std::array<Bitboard, 64> knightAttack;
    static std::array<std::array<Bitboard, 64>, 2> pawnAttack;
    static std::array<std::array<Bitboard, 64>, 64> line;
    static std::array<std::array<Bitboard, 64>, 64> between;
    static std::array<std::array<Bitboard, 64>, 8> rays;
    static std::array<std::array<Bitboard, 64>, 2> passed;
    static std::array<std::array<Bitboard, 64>, 2> backward;
    static std::array<Bitboard, 64> isolated;
    static std::array<std::array<Bitboard, 64>, 2> kingZone;

#if !(defined _WIN64 || defined __x86_64__)
    static const std::array<int, 64> index;
#endif

    static bool hardwarePopcntSupported;
};

#endif
