#ifndef BITBOARD_HPP_
#define BITBOARD_HPP_

#include <cstdint>
#include <cassert>
#include <array>
#include "square.hpp"

typedef uint64_t Bitboard;

class Bitboards
{
public:
    static void initialize();

    static Bitboard bishopAttacks(Square sq, Bitboard occupied)
    {
        assert(isSquareOk(sq));
        const Magic & mag = bishopMagic[sq];
        return mag.data[((occupied & mag.mask) * mag.magic) >> (64 - 9)];
    }

    static Bitboard rookAttacks(Square sq, Bitboard occupied)
    {
        assert(isSquareOk(sq));
        const Magic & mag = rookMagic[sq];
        return mag.data[((occupied & mag.mask) * mag.magic) >> (64 - 12)];
    }

    static Bitboard queenAttacks(Square sq, Bitboard occupied)
    {
        return (bishopAttacks(sq, occupied) | rookAttacks(sq, occupied));
    }

    static std::array<Bitboard, 64> bit;
    static std::array<Bitboard, 64> kingAttacks;
    static std::array<Bitboard, 64> knightAttacks;
    static std::array<Bitboard, 64> pawnAttacks[2];
    static std::array<Bitboard, 64> pawnSingleMoves[2];
    static std::array<Bitboard, 64> pawnDoubleMoves[2];
    static std::array<Bitboard, 64> rays[8];
    static std::array<Bitboard, 64> between[64];

    static std::array<Bitboard, 64> passed[2];
    static std::array<Bitboard, 64> backward[2];
    static std::array<Bitboard, 64> isolated;

    static const std::array<Bitboard, 8> ranks;

    static const std::array<Bitboard, 8> files;

    // Returns the amount of set bits in bb.
    // We have two versions, one using hardware and the other using sofware.
    // We automatically detect which one to use.
    static int hardwarePopcnt(Bitboard bb)
    {
#if (defined _WIN64 || defined __x86_64__)
#ifdef __clang__
        return static_cast<int>(__builtin_popcountll(bb));
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
#if (defined _WIN64 || defined __x86_64__)
    // Returns the least significant set bit in the mask.
    static unsigned long lsb(Bitboard bb)
    {
        assert(bb);
#ifdef _MSC_VER
        unsigned long index;
        _BitScanForward64(&index, bb);
        return index;
#else
	return __builtin_ctzll(bb);
#endif
    }

    // Returns the most significant set bit in the mask.
    static unsigned long msb(Bitboard bb)
    {
        assert(bb);
#ifdef _MSC_VER
        unsigned long index;
        _BitScanReverse64(&index, bb);
        return index;
#else
        return (63 - __builtin_clzll(bb));
#endif
    }
#else
    // The author of both the lsb and msb is Kim Walisch (2012).
    static const std::array<int, 64> index;

    static int lsb(Bitboard bb)
    {
        assert(bb);
        return index[((bb ^ (bb - 1)) * 0x03f79d71b4cb0a89) >> 58];
    }

    static int msb(Bitboard bb)
    {
        assert(bb);
        bb |= bb >> 1;
        bb |= bb >> 2;
        bb |= bb >> 4;
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;
        return index[(bb * 0x03f79d71b4cb0a89) >> 58];
    }
#endif

    static bool isHardwarePopcntSupported() { return hardwarePopcntSupported; }
private:
    class Magic
    {
    public:
        Bitboard * data;
        Bitboard mask;
        Bitboard magic;
    };

    class MagicInit
    {
    public:
        Bitboard magic;
        int32_t index;
    };

    static std::array<Magic, 64> bishopMagic;
    static std::array<Magic, 64> rookMagic;
    static std::array<Bitboard, 97264> lookupTable;

    static std::array<MagicInit, 64> rookInit;
    static std::array<MagicInit, 64> bishopInit;

    static void initMagics(std::array<MagicInit, 64> & magicInit, std::array<Magic, 64> & magic, std::array<int, 2> dir[], int shift);

    static bool hardwarePopcntSupported;
};

#endif
