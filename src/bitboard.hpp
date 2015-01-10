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

    static Bitboard bishopAttacks(Square sq, Bitboard occupied)
    {
        const auto& mag = bishopMagic[sq];
        return mag.data[((occupied & mag.mask) * mag.magic) >> (64 - 9)];
    }

    static Bitboard rookAttacks(Square sq, Bitboard occupied)
    {
        const auto& mag = rookMagic[sq];
        return mag.data[((occupied & mag.mask) * mag.magic) >> (64 - 12)];
    }

    static Bitboard queenAttacks(Square sq, Bitboard occupied)
    {
        return (bishopAttacks(sq, occupied) | rookAttacks(sq, occupied));
    }

    static Bitboard bit(Square sq) { return bits[sq]; }
    static Bitboard kingAttacks(Square sq) { return kingAttack[sq]; }
    static Bitboard knightAttacks(Square sq) { return knightAttack[sq]; }
    static Bitboard pawnAttacks(Color c, Square sq) { return pawnAttack[c][sq]; }
    static Bitboard pawnSingleMoves(Color c, Square sq) { return pawnSingleMove[c][sq]; }
    static Bitboard pawnDoubleMoves(Color c, Square sq) { return pawnDoubleMove[c][sq]; }
    static Bitboard squaresBetween(Square from, Square to) { return between[from][to]; }
    static Bitboard lineFormedBySquares(Square from, Square to) { return line[from][to]; }
    static Bitboard ray(int direction, Square sq) { return rays[direction][sq]; }
    static Bitboard passedPawn(Color c, Square sq) { return passed[c][sq]; }
    static Bitboard backwardPawn(Color c, Square sq) { return backward[c][sq]; }
    static Bitboard isolatedPawn(Square sq) { return isolated[sq]; }
    static Bitboard kingSafetyZone(Color c, Square sq) { return kingZone[c][sq]; }

    static const std::array<Bitboard, 8> ranks;
    static const std::array<Bitboard, 8> files;

    // Returns the amount of set bits in bb.
    // We have two versions, one using hardware and the other using sofware.
    // We automatically detect which one to use.
    template <bool hardwarePopcntEnabled>
    static inline int popcnt(Bitboard bb)
    {
        return (hardwarePopcntEnabled ? hardwarePopcnt(bb) : softwarePopcnt(bb));
    }

    // Returns the least significant set bit in the mask.
    static unsigned long lsb(Bitboard bb)
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
    static unsigned long msb(Bitboard bb)
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
    static bool moreThanOneBitSet(Bitboard bb)
    {
        return (bb & (bb - 1)) != 0;
    }

    // Tests whether the bitboard has a particular bit set.
    static bool testBit(Bitboard bb, int b)
    {
        return (bb & bits[b]) != 0;
    }

    // Sets the specified bit to 1.
    static void setBit(Bitboard& bb, int b)
    {
        bb |= bits[b];
    }

    // Sets the specified bit to 0. 
    // We assume that the bit is already 1, otherwise this doesn't work.
    static void clearBit(Bitboard& bb, int b)
    {
        assert(testBit(bb, b));
        bb ^= bits[b];
    }

    static Bitboard pieceAttacks(Color sideToMove, Piece piece, Square sq, Bitboard occupied)
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

    static int hardwarePopcnt(Bitboard bb)
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
    static std::array<MagicInit, 64> rookInit;
    static std::array<MagicInit, 64> bishopInit;

    static void initMagics(std::array<MagicInit, 64>& magicInit, std::array<Magic, 64>& magic, std::array<int, 2> dir[], int shift);

    static std::array<Bitboard, 64> bits;
    static std::array<Bitboard, 64> kingAttack;
    static std::array<Bitboard, 64> knightAttack;
    static std::array<std::array<Bitboard, 64>, 2> pawnAttack;
    static std::array<std::array<Bitboard, 64>, 2> pawnSingleMove;
    static std::array<std::array<Bitboard, 64>, 2> pawnDoubleMove;
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
