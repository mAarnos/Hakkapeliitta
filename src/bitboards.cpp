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

#include "bitboards.hpp"
#include <vector>

std::array<Bitboard, 64> Bitboards::mBits;
std::array<Bitboard, 64> Bitboards::mKingAttacks;
std::array<Bitboard, 64> Bitboards::mKnightAttacks;
std::array<std::array<Bitboard, 64>, 2> Bitboards::mPawnAttacks;
std::array<std::array<Bitboard, 64>, 8> Bitboards::mRays;
std::array<std::array<Bitboard, 64>, 64> Bitboards::mBetween;
std::array<std::array<Bitboard, 64>, 64> Bitboards::mLines;

std::array<std::array<Bitboard, 64>, 2> Bitboards::mPassed;
std::array<std::array<Bitboard, 64>, 2> Bitboards::mBackward;
std::array<Bitboard, 64> Bitboards::mIsolated;
std::array<std::array<Bitboard, 64>, 2> Bitboards::mKingZone;

const std::array<Bitboard, 8> Bitboards::ranks = {
    0x00000000000000FF,
    0x000000000000FF00,
    0x0000000000FF0000,
    0x00000000FF000000,
    0x000000FF00000000,
    0x0000FF0000000000,
    0x00FF000000000000,
    0xFF00000000000000
};

const std::array<Bitboard, 8> Bitboards::files = {
    0x0101010101010101,
    0x0202020202020202,
    0x0404040404040404,
    0x0808080808080808,
    0x1010101010101010,
    0x2020202020202020,
    0x4040404040404040,
    0x8080808080808080
};

bool Bitboards::mHardwarePopcntSupported;

#if !(defined _WIN64 || defined __x86_64__)
const std::array<int, 64> Bitboards::mIndex = {
    0, 47, 1, 56, 48, 27, 2, 60,
    57, 49, 41, 37, 28, 16, 3, 61,
    54, 58, 35, 52, 50, 42, 21, 44,
    38, 32, 29, 23, 17, 11, 4, 62,
    46, 55, 26, 59, 40, 36, 15, 53,
    34, 51, 20, 43, 31, 22, 10, 45,
    25, 39, 14, 33, 19, 30, 9, 24,
    13, 18, 8, 12, 7, 6, 5, 63
};
#endif

std::array<Bitboards::Magic, 64> Bitboards::mBishopMagics;
std::array<Bitboards::Magic, 64> Bitboards::mRookMagics;

std::array<Bitboard, 97264> Bitboards::mLookupTable;

const std::array<Bitboards::MagicInit, 64> Bitboards::mBishopInit = { {
        { 0x007bfeffbfeffbff, 16530 },
        { 0x003effbfeffbfe08, 9162 },
        { 0x0000401020200000, 9674 },
        { 0x0000200810000000, 18532 },
        { 0x0000110080000000, 19172 },
        { 0x0000080100800000, 17700 },
        { 0x0007efe0bfff8000, 5730 },
        { 0x00000fb0203fff80, 19661 },
        { 0x00007dff7fdff7fd, 17065 },
        { 0x0000011fdff7efff, 12921 },
        { 0x0000004010202000, 15683 },
        { 0x0000002008100000, 17764 },
        { 0x0000001100800000, 19684 },
        { 0x0000000801008000, 18724 },
        { 0x000007efe0bfff80, 4108 },
        { 0x000000080f9fffc0, 12936 },
        { 0x0000400080808080, 15747 },
        { 0x0000200040404040, 4066 },
        { 0x0000400080808080, 14359 },
        { 0x0000200200801000, 36039 },
        { 0x0000240080840000, 20457 },
        { 0x0000080080840080, 43291 },
        { 0x0000040010410040, 5606 },
        { 0x0000020008208020, 9497 },
        { 0x0000804000810100, 15715 },
        { 0x0000402000408080, 13388 },
        { 0x0000804000810100, 5986 },
        { 0x0000404004010200, 11814 },
        { 0x0000404004010040, 92656 },
        { 0x0000101000804400, 9529 },
        { 0x0000080800104100, 18118 },
        { 0x0000040400082080, 5826 },
        { 0x0000410040008200, 4620 },
        { 0x0000208020004100, 12958 },
        { 0x0000110080040008, 55229 },
        { 0x0000020080080080, 9892 },
        { 0x0000404040040100, 33767 },
        { 0x0000202040008040, 20023 },
        { 0x0000101010002080, 6515 },
        { 0x0000080808001040, 6483 },
        { 0x0000208200400080, 19622 },
        { 0x0000104100200040, 6274 },
        { 0x0000208200400080, 18404 },
        { 0x0000008840200040, 14226 },
        { 0x0000020040100100, 17990 },
        { 0x007fff80c0280050, 18920 },
        { 0x0000202020200040, 13862 },
        { 0x0000101010100020, 19590 },
        { 0x0007ffdfc17f8000, 5884 },
        { 0x0003ffefe0bfc000, 12946 },
        { 0x0000000820806000, 5570 },
        { 0x00000003ff004000, 18740 },
        { 0x0000000100202000, 6242 },
        { 0x0000004040802000, 12326 },
        { 0x007ffeffbfeff820, 4156 },
        { 0x003fff7fdff7fc10, 12876 },
        { 0x0003ffdfdfc27f80, 17047 },
        { 0x000003ffefe0bfc0, 17780 },
        { 0x0000000008208060, 2494 },
        { 0x0000000003ff0040, 17716 },
        { 0x0000000001002020, 17067 },
        { 0x0000000040408020, 9465 },
        { 0x00007ffeffbfeff9, 16196 },
        { 0x007ffdff7fdff7fd, 6166 }
        }
};

const std::array<Bitboards::MagicInit, 64> Bitboards::mRookInit = { {
        { 0x00a801f7fbfeffff, 85487 },
        { 0x00180012000bffff, 43101 },
        { 0x0040080010004004, 0 },
        { 0x0040040008004002, 49085 },
        { 0x0040020004004001, 93168 },
        { 0x0020008020010202, 78956 },
        { 0x0040004000800100, 60703 },
        { 0x0810020990202010, 64799 },
        { 0x000028020a13fffe, 30640 },
        { 0x003fec008104ffff, 9256 },
        { 0x00001800043fffe8, 28647 },
        { 0x00001800217fffe8, 10404 },
        { 0x0000200100020020, 63775 },
        { 0x0000200080010020, 14500 },
        { 0x0000300043ffff40, 52819 },
        { 0x000038010843fffd, 2048 },
        { 0x00d00018010bfff8, 52037 },
        { 0x0009000c000efffc, 16435 },
        { 0x0004000801020008, 29104 },
        { 0x0002002004002002, 83439 },
        { 0x0001002002002001, 86842 },
        { 0x0001001000801040, 27623 },
        { 0x0000004040008001, 26599 },
        { 0x0000802000200040, 89583 },
        { 0x0040200010080010, 7042 },
        { 0x0000080010040010, 84463 },
        { 0x0004010008020008, 82415 },
        { 0x0000020020040020, 95216 },
        { 0x0000010020020020, 35015 },
        { 0x0000008020010020, 10790 },
        { 0x0000008020200040, 53279 },
        { 0x0000200020004081, 70684 },
        { 0x0040001000200020, 38640 },
        { 0x0000080400100010, 32743 },
        { 0x0004010200080008, 68894 },
        { 0x0000200200200400, 62751 },
        { 0x0000200100200200, 41670 },
        { 0x0000200080200100, 25575 },
        { 0x0000008000404001, 3042 },
        { 0x0000802000200040, 36591 },
        { 0x00ffffb50c001800, 69918 },
        { 0x007fff98ff7fec00, 9092 },
        { 0x003ffff919400800, 17401 },
        { 0x001ffff01fc03000, 40688 },
        { 0x0000010002002020, 96240 },
        { 0x0000008001002020, 91632 },
        { 0x0003fff673ffa802, 32495 },
        { 0x0001fffe6fff9001, 51133 },
        { 0x00ffffd800140028, 78319 },
        { 0x007fffe87ff7ffec, 12595 },
        { 0x003fffd800408028, 5152 },
        { 0x001ffff111018010, 32110 },
        { 0x000ffff810280028, 13894 },
        { 0x0007fffeb7ff7fd8, 2546 },
        { 0x0003fffc0c480048, 41052 },
        { 0x0001ffffa2280028, 77676 },
        { 0x00ffffe4ffdfa3ba, 73580 },
        { 0x007ffb7fbfdfeff6, 44947 },
        { 0x003fffbfdfeff7fa, 73565 },
        { 0x001fffeff7fbfc22, 17682 },
        { 0x000ffffbf7fc2ffe, 56607 },
        { 0x0007fffdfa03ffff, 56135 },
        { 0x0003ffdeff7fbdec, 44989 },
        { 0x0001ffff99ffab2f, 21479 }
        }
};

void Bitboards::staticInitialize()
{
    static const std::array<int, 8> rankDirection = {
        -1, -1, -1, 0, 0, 1, 1, 1
    };
    static const std::array<int, 8> fileDirection = {
        -1, 0, 1, -1, 1, -1, 0, 1
    };

    static const std::array<std::array<int, 2>, 4> bishopDirections = {{
        { -9, -17 }, { -7, -15 }, { 7, 15 }, { 9, 17 }
    }};

    static const std::array<std::array<int, 2>, 4> rookDirections = { {
        { -8, -16 }, { -1, -1 }, { 1, 1 }, { 8, 16 }
    }};

    std::array<std::array<int8_t, 64>, 64> heading;
    for (auto i = 0; i < 64; ++i)
    {
        heading[i].fill(-1);
    }

    // Single bits
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        mBits[sq] = 1ULL << sq;
    }

    // King attacks
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        auto kingSet = mBits[sq];
        kingSet |= (mBits[sq] << 1) & 0xFEFEFEFEFEFEFEFE;
        kingSet |= (mBits[sq] >> 1) & 0x7F7F7F7F7F7F7F7F;
        kingSet = ((kingSet << 8) | (kingSet >> 8) | (kingSet ^ mBits[sq]));
        mKingAttacks[sq] = kingSet;
    }

    // Knight attacks
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        auto knightSet = (mBits[sq] << 17) & 0xFEFEFEFEFEFEFEFE;
        knightSet |= (mBits[sq] << 10) & 0xFCFCFCFCFCFCFCFC;
        knightSet |= (mBits[sq] << 15) & 0x7F7F7F7F7F7F7F7F;
        knightSet |= (mBits[sq] << 6) & 0x3F3F3F3F3F3F3F3F;
        knightSet |= (mBits[sq] >> 17) & 0x7F7F7F7F7F7F7F7F;
        knightSet |= (mBits[sq] >> 10) & 0x3F3F3F3F3F3F3F3F;
        knightSet |= (mBits[sq] >> 15) & 0xFEFEFEFEFEFEFEFE;
        knightSet |= (mBits[sq] >> 6) & 0xFCFCFCFCFCFCFCFC;
        mKnightAttacks[sq] = knightSet;
    }

    // Pawn attacks
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        mPawnAttacks[Color::White][sq] = ((mBits[sq] << 9) & 0xFEFEFEFEFEFEFEFE) | ((mBits[sq] << 7) & 0x7F7F7F7F7F7F7F7F);
        mPawnAttacks[Color::Black][sq] = ((mBits[sq] >> 9) & 0x7F7F7F7F7F7F7F7F) | ((mBits[sq] >> 7) & 0xFEFEFEFEFEFEFEFE);
    }

    // Rays to all directions
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        const auto r = rank(sq);
        const auto f = file(sq);

        for (auto i = 0; i < 8; ++i)
        {
            mRays[i][sq] = 0;
            for (auto j = 1; j < 8; ++j)
            {
                const auto toRank = rankDirection[i] * j + r;
                const auto toFile = fileDirection[i] * j + f;
                if (toRank < 0 || toRank > 7 || toFile < 0 || toFile > 7) 
                {
                    break; // We went over the side of the board, stop.
                }
                heading[sq][toRank * 8 + toFile] = static_cast<int8_t>(i);
                mRays[i][sq] |= mBits[toRank * 8 + toFile];
            }
        }
    }

    // All squares between two squares.
    for (Square i = Square::A1; i <= Square::H8; ++i)
    {
        mBetween[i].fill(0);
        mLines[i].fill(0);
        for (Square j = Square::A1; j <= Square::H8; ++j)
        {
            const auto h = heading[i][j];
            if (h != -1)
            {
                mBetween[i][j] = mRays[h][i] & mRays[7 - h][j];
                mLines[i][j] = mRays[h][i] | mRays[7 - h][j];
            }
        }
    }

    // Pawn evaluation bitboards: passed pawn, backward pawns, isolated pawns.
    mIsolated.fill(0);
    mPassed[Color::White].fill(0);
    mPassed[Color::Black].fill(0);
    mBackward[Color::White].fill(0);
    mBackward[Color::Black].fill(0);
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (sq < Square::A2 || sq > Square::H7)
        {
            continue;
        }

        mPassed[Color::White][sq] = mRays[6][sq];
        mPassed[Color::Black][sq] = mRays[1][sq];

        const auto f = file(sq);
        if (f != 7)
        {
            mPassed[Color::White][sq] |= mRays[6][sq + 1];
            mPassed[Color::Black][sq] |= mRays[1][sq + 1];
            mBackward[Color::White][sq] |= mRays[1][sq + 9];
            mBackward[Color::Black][sq] |= mRays[6][sq - 7];
            mIsolated[sq] |= files[f + 1];
        }

        if (f != 0)
        {
            mPassed[Color::White][sq] |= mRays[6][sq - 1];
            mPassed[Color::Black][sq] |= mRays[1][sq - 1];
            mBackward[Color::White][sq] |= mRays[1][sq + 7];
            mBackward[Color::Black][sq] |= mRays[6][sq - 9];
            mIsolated[sq] |= files[f - 1];
        }
    }

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        mKingZone[Color::White][sq] = sq < Square::A8 ? (mKingAttacks[sq] | mKingAttacks[sq + 8]) : (mKingAttacks[sq] | mBits[sq]);
        mKingZone[Color::Black][sq] = sq > Square::H1 ? (mKingAttacks[sq] | mKingAttacks[sq - 8]) : (mKingAttacks[sq] | mBits[sq]);
    }

    mLookupTable.fill(0);
    initializeMagics(mBishopInit, mBishopMagics, bishopDirections, 64 - 9);
    initializeMagics(mRookInit, mRookMagics, rookDirections, 64 - 12);

#if !(defined _WIN64 || defined __x86_64__)
    mHardwarePopcntSupported = false;
#else
    int regs[4] = { 0, 0, 0, 0 };
 #if (defined __clang__ || defined __GNUC__)
    regs[0] = 0x00000001;
    __asm__ __volatile__ (
     "cpuid;"
    : "+a" (regs[0]),
      "=b" (regs[1]),
      "=c" (regs[2]),
      "=d" (regs[3]));
 #else
    __cpuid(regs, 0x00000001);
 #endif
    mHardwarePopcntSupported = (regs[2] & (1 << 23)) != 0;
#endif
}

void Bitboards::initializeMagics(const std::array<MagicInit, 64>& magicInit, std::array<Magic, 64>& magic, 
                                 const std::array<std::array<int, 2>, 4>& dir, int shift)
{
    std::vector<int> squares;

    const auto rookMask = [](Square sq) 
    {
        auto result = 0ULL;
        const auto rk = rank(sq), fl = file(sq);
        for (auto r = rk + 1; r <= 6; ++r) result |= (mBits[fl + r * 8]);
        for (auto r = rk - 1; r >= 1; --r) result |= (mBits[fl + r * 8]);
        for (auto f = fl + 1; f <= 6; ++f) result |= (mBits[f + rk * 8]);
        for (auto f = fl - 1; f >= 1; --f) result |= (mBits[f + rk * 8]);
        return result;
    };

    const auto bishopMask = [](Square sq) 
    {
        auto result = 0ULL;
        const auto rk = rank(sq), fl = file(sq);
        for (auto r = rk + 1, f = fl + 1; r <= 6 && f <= 6; ++r, ++f) result |= (mBits[f + r * 8]);
        for (auto r = rk + 1, f = fl - 1; r <= 6 && f >= 1; ++r, --f) result |= (mBits[f + r * 8]);
        for (auto r = rk - 1, f = fl + 1; r >= 1 && f <= 6; --r, ++f) result |= (mBits[f + r * 8]);
        for (auto r = rk - 1, f = fl - 1; r >= 1 && f >= 1; --r, --f) result |= (mBits[f + r * 8]);
        return result;
    };

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        magic[sq].mMagic = magicInit[sq].mMagic;
        magic[sq].mData = &mLookupTable[magicInit[sq].mIndex];
        auto bb = magic[sq].mMask = ((shift == 64 - 12) ? rookMask(sq) : bishopMask(sq));
        const auto sq88 = sq + (sq & ~7);

        squares.clear();
        while (bb)
        {
            squares.push_back(popLsb(bb));
        }
        
        // Loop through all possible occupations within the mask and calculate the corresponding attack sets.
        for (auto k = 0; k < (1 << squares.size()); ++k)
        {
            auto bb2 = bb = 0;
            for (auto j = 0; j < static_cast<int>(squares.size()); ++j)
            {
                if (testBit(k, j))
                {
                    setBit(bb, squares[j]);
                }
            }
            for (auto j = 0; j < 4; ++j)
            {
                for (auto d = 1; !((sq88 + d * dir[j][1]) & 0x88); ++d)
                {
                    setBit(bb2, sq + d * dir[j][0]);
                    if (testBit(bb, sq + d * dir[j][0]))
                    {
                        break;
                    }
                }
            }
            const auto j = ((bb * magic[sq].mMagic) >> shift);
            magic[sq].mData[j] = bb2;
        }
    }
}

