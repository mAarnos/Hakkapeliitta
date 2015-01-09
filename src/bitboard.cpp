#include "bitboard.hpp"
#include <vector>
#include <iostream>
#include <cstring>
#include "color.hpp"
#include "utils/synchronized_ostream.hpp"

std::array<Bitboard, 64> Bitboards::bits;
std::array<Bitboard, 64> Bitboards::kingAttack;
std::array<Bitboard, 64> Bitboards::knightAttack;
std::array<std::array<Bitboard, 64>, 2> Bitboards::pawnAttack;
std::array<std::array<Bitboard, 64>, 2> Bitboards::pawnSingleMove;
std::array<std::array<Bitboard, 64>, 2> Bitboards::pawnDoubleMove;
std::array<std::array<Bitboard, 64>, 8> Bitboards::rays;
std::array<std::array<Bitboard, 64>, 64> Bitboards::between;
std::array<std::array<Bitboard, 64>, 64> Bitboards::line;

std::array<std::array<Bitboard, 64>, 2> Bitboards::passed;
std::array<std::array<Bitboard, 64>, 2> Bitboards::backward;
std::array<Bitboard, 64> Bitboards::isolated;

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

std::array<std::array<Bitboard, 64>, 2> Bitboards::kingZone;

bool Bitboards::hardwarePopcntSupported;

#if !(defined _WIN64 || defined __x86_64__)
const std::array<int, 64> Bitboards::index = {
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

std::array<Bitboards::Magic, 64> Bitboards::bishopMagic;
std::array<Bitboards::Magic, 64> Bitboards::rookMagic;

std::array<Bitboard, 97264> Bitboards::lookupTable;

std::array<Bitboards::MagicInit, 64> Bitboards::bishopInit = { {
        { 0x007bfeffbfeffbffull, 16530 },
        { 0x003effbfeffbfe08ull, 9162 },
        { 0x0000401020200000ull, 9674 },
        { 0x0000200810000000ull, 18532 },
        { 0x0000110080000000ull, 19172 },
        { 0x0000080100800000ull, 17700 },
        { 0x0007efe0bfff8000ull, 5730 },
        { 0x00000fb0203fff80ull, 19661 },
        { 0x00007dff7fdff7fdull, 17065 },
        { 0x0000011fdff7efffull, 12921 },
        { 0x0000004010202000ull, 15683 },
        { 0x0000002008100000ull, 17764 },
        { 0x0000001100800000ull, 19684 },
        { 0x0000000801008000ull, 18724 },
        { 0x000007efe0bfff80ull, 4108 },
        { 0x000000080f9fffc0ull, 12936 },
        { 0x0000400080808080ull, 15747 },
        { 0x0000200040404040ull, 4066 },
        { 0x0000400080808080ull, 14359 },
        { 0x0000200200801000ull, 36039 },
        { 0x0000240080840000ull, 20457 },
        { 0x0000080080840080ull, 43291 },
        { 0x0000040010410040ull, 5606 },
        { 0x0000020008208020ull, 9497 },
        { 0x0000804000810100ull, 15715 },
        { 0x0000402000408080ull, 13388 },
        { 0x0000804000810100ull, 5986 },
        { 0x0000404004010200ull, 11814 },
        { 0x0000404004010040ull, 92656 },
        { 0x0000101000804400ull, 9529 },
        { 0x0000080800104100ull, 18118 },
        { 0x0000040400082080ull, 5826 },
        { 0x0000410040008200ull, 4620 },
        { 0x0000208020004100ull, 12958 },
        { 0x0000110080040008ull, 55229 },
        { 0x0000020080080080ull, 9892 },
        { 0x0000404040040100ull, 33767 },
        { 0x0000202040008040ull, 20023 },
        { 0x0000101010002080ull, 6515 },
        { 0x0000080808001040ull, 6483 },
        { 0x0000208200400080ull, 19622 },
        { 0x0000104100200040ull, 6274 },
        { 0x0000208200400080ull, 18404 },
        { 0x0000008840200040ull, 14226 },
        { 0x0000020040100100ull, 17990 },
        { 0x007fff80c0280050ull, 18920 },
        { 0x0000202020200040ull, 13862 },
        { 0x0000101010100020ull, 19590 },
        { 0x0007ffdfc17f8000ull, 5884 },
        { 0x0003ffefe0bfc000ull, 12946 },
        { 0x0000000820806000ull, 5570 },
        { 0x00000003ff004000ull, 18740 },
        { 0x0000000100202000ull, 6242 },
        { 0x0000004040802000ull, 12326 },
        { 0x007ffeffbfeff820ull, 4156 },
        { 0x003fff7fdff7fc10ull, 12876 },
        { 0x0003ffdfdfc27f80ull, 17047 },
        { 0x000003ffefe0bfc0ull, 17780 },
        { 0x0000000008208060ull, 2494 },
        { 0x0000000003ff0040ull, 17716 },
        { 0x0000000001002020ull, 17067 },
        { 0x0000000040408020ull, 9465 },
        { 0x00007ffeffbfeff9ull, 16196 },
        { 0x007ffdff7fdff7fdull, 6166 }
        }
};

std::array<Bitboards::MagicInit, 64> Bitboards::rookInit = { {
        { 0x00a801f7fbfeffffull, 85487 },
        { 0x00180012000bffffull, 43101 },
        { 0x0040080010004004ull, 0 },
        { 0x0040040008004002ull, 49085 },
        { 0x0040020004004001ull, 93168 },
        { 0x0020008020010202ull, 78956 },
        { 0x0040004000800100ull, 60703 },
        { 0x0810020990202010ull, 64799 },
        { 0x000028020a13fffeull, 30640 },
        { 0x003fec008104ffffull, 9256 },
        { 0x00001800043fffe8ull, 28647 },
        { 0x00001800217fffe8ull, 10404 },
        { 0x0000200100020020ull, 63775 },
        { 0x0000200080010020ull, 14500 },
        { 0x0000300043ffff40ull, 52819 },
        { 0x000038010843fffdull, 2048 },
        { 0x00d00018010bfff8ull, 52037 },
        { 0x0009000c000efffcull, 16435 },
        { 0x0004000801020008ull, 29104 },
        { 0x0002002004002002ull, 83439 },
        { 0x0001002002002001ull, 86842 },
        { 0x0001001000801040ull, 27623 },
        { 0x0000004040008001ull, 26599 },
        { 0x0000802000200040ull, 89583 },
        { 0x0040200010080010ull, 7042 },
        { 0x0000080010040010ull, 84463 },
        { 0x0004010008020008ull, 82415 },
        { 0x0000020020040020ull, 95216 },
        { 0x0000010020020020ull, 35015 },
        { 0x0000008020010020ull, 10790 },
        { 0x0000008020200040ull, 53279 },
        { 0x0000200020004081ull, 70684 },
        { 0x0040001000200020ull, 38640 },
        { 0x0000080400100010ull, 32743 },
        { 0x0004010200080008ull, 68894 },
        { 0x0000200200200400ull, 62751 },
        { 0x0000200100200200ull, 41670 },
        { 0x0000200080200100ull, 25575 },
        { 0x0000008000404001ull, 3042 },
        { 0x0000802000200040ull, 36591 },
        { 0x00ffffb50c001800ull, 69918 },
        { 0x007fff98ff7fec00ull, 9092 },
        { 0x003ffff919400800ull, 17401 },
        { 0x001ffff01fc03000ull, 40688 },
        { 0x0000010002002020ull, 96240 },
        { 0x0000008001002020ull, 91632 },
        { 0x0003fff673ffa802ull, 32495 },
        { 0x0001fffe6fff9001ull, 51133 },
        { 0x00ffffd800140028ull, 78319 },
        { 0x007fffe87ff7ffecull, 12595 },
        { 0x003fffd800408028ull, 5152 },
        { 0x001ffff111018010ull, 32110 },
        { 0x000ffff810280028ull, 13894 },
        { 0x0007fffeb7ff7fd8ull, 2546 },
        { 0x0003fffc0c480048ull, 41052 },
        { 0x0001ffffa2280028ull, 77676 },
        { 0x00ffffe4ffdfa3baull, 73580 },
        { 0x007ffb7fbfdfeff6ull, 44947 },
        { 0x003fffbfdfeff7faull, 73565 },
        { 0x001fffeff7fbfc22ull, 17682 },
        { 0x000ffffbf7fc2ffeull, 56607 },
        { 0x0007fffdfa03ffffull, 56135 },
        { 0x0003ffdeff7fbdecull, 44989 },
        { 0x0001ffff99ffab2full, 21479 }
        }
};

void Bitboards::initialize()
{
    static std::array<int, 8> rankDirection = {
        -1, -1, -1, 0, 0, 1, 1, 1
    };
    static std::array<int, 8> fileDirection = {
        -1, 0, 1, -1, 1, -1, 0, 1
    };

    static std::array<int, 2> bishopDirections[4] = {
        { -9, -17 }, { -7, -15 }, { 7, 15 }, { 9, 17 }
    };

    static std::array<int, 2> rookDirections[4] = {
        { -8, -16 }, { -1, -1 }, { 1, 1 }, { 8, 16 }
    };

	// Avoid excessive stack usage warnings by moving heading to heap.
    std::vector<int> heading[64];
	for (auto i = 0; i < 64; ++i)
	{
		heading[i].resize(64, -1);
	}

    // Single bits
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        bits[sq] = 1ull << sq;
    }

    // King attacks
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        auto kingSet = bits[sq];
        kingSet |= (bits[sq] << 1) & 0xFEFEFEFEFEFEFEFE;
        kingSet |= (bits[sq] >> 1) & 0x7F7F7F7F7F7F7F7F;
        kingSet = ((kingSet << 8) | (kingSet >> 8) | (kingSet ^ bits[sq]));
        kingAttack[sq] = kingSet;
    }

    // Knight attacks
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        auto knightSet = (bits[sq] << 17) & 0xFEFEFEFEFEFEFEFE;
        knightSet |= (bits[sq] << 10) & 0xFCFCFCFCFCFCFCFC;
        knightSet |= (bits[sq] << 15) & 0x7F7F7F7F7F7F7F7F;
        knightSet |= (bits[sq] << 6) & 0x3F3F3F3F3F3F3F3F;
        knightSet |= (bits[sq] >> 17) & 0x7F7F7F7F7F7F7F7F;
        knightSet |= (bits[sq] >> 10) & 0x3F3F3F3F3F3F3F3F;
        knightSet |= (bits[sq] >> 15) & 0xFEFEFEFEFEFEFEFE;
        knightSet |= (bits[sq] >> 6) & 0xFCFCFCFCFCFCFCFC;
        knightAttack[sq] = knightSet;
    }

    // Pawn attacks
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        pawnAttack[Color::White][sq] = ((bits[sq] << 9) & 0xFEFEFEFEFEFEFEFE) | ((bits[sq] << 7) & 0x7F7F7F7F7F7F7F7F);
        pawnAttack[Color::Black][sq] = ((bits[sq] >> 9) & 0x7F7F7F7F7F7F7F7F) | ((bits[sq] >> 7) & 0xFEFEFEFEFEFEFEFE);
    }

    // Pawn moves
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        pawnSingleMove[Color::White][sq] = (sq <= Square::H7 ? bits[sq + 8] : 0);
        pawnSingleMove[Color::Black][sq] = (sq >= Square::A2 ? bits[sq - 8] : 0);
        pawnDoubleMove[Color::White][sq] = ((sq >= Square::A2 && sq <= Square::H2) ? bits[sq + 16] : 0);
        pawnDoubleMove[Color::Black][sq] = ((sq >= Square::A7 && sq <= Square::H7) ? bits[sq - 16] : 0);
    }

    // Rays to all directions
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        auto r = rank(sq);
        auto f = file(sq);

        for (auto i = 0; i < 8; ++i)
        {
            rays[i][sq] = 0;
            for (auto j = 1; j < 8; ++j)
            {
                auto toRank = rankDirection[i] * j + r;
                auto toFile = fileDirection[i] * j + f;
                if (toRank < 0 || toRank > 7 || toFile < 0 || toFile > 7) // Check if we went over the side of the board.
                    break;
                heading[sq][toRank * 8 + toFile] = i;
                rays[i][sq] |= bits[toRank * 8 + toFile];
            }
        }
    }

    // All squares between two squares.
    for (Square i = Square::A1; i <= Square::H8; ++i)
    {
        for (Square j = Square::A1; j <= Square::H8; ++j)
        {
            auto h = heading[i][j];
            between[i][j] = (h != -1 ? rays[h][i] & rays[7 - h][j] : 0);
            line[i][j] = (h != -1 ? rays[h][i] | rays[7 - h][j] : 0);
        }
    }

    // Pawn evaluation bitboards: passed pawn, backward pawns, isolated pawns.
    // cppCheck(a static code analyzer) gives a false positive here about out of bounds array access.
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        isolated[sq] = 0;

        if (sq < Square::A2 || sq > Square::H7)
        {
            passed[Color::White][sq] = passed[Color::Black][sq] = 0;
            continue;
        }

        passed[Color::White][sq] = rays[6][sq];
        passed[Color::Black][sq] = rays[1][sq];

        auto f = file(sq);
        if (f != 7)
        {
            passed[Color::White][sq] |= rays[6][sq + 1];
            passed[Color::Black][sq] |= rays[1][sq + 1];
            backward[Color::White][sq] |= rays[1][sq + 9];
            backward[Color::Black][sq] |= rays[6][sq - 7];
            isolated[sq] |= files[f + 1];
        }

        if (f != 0)
        {
            passed[Color::White][sq] |= rays[6][sq - 1];
            passed[Color::Black][sq] |= rays[1][sq - 1];
            backward[Color::White][sq] |= rays[1][sq + 7];
            backward[Color::Black][sq] |= rays[6][sq - 9];
            isolated[sq] |= files[f - 1];
        }
    }

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        kingZone[Color::White][sq] = sq < Square::A8 ? kingAttack[sq] | kingAttack[sq + 8] : kingAttack[sq];
        kingZone[Color::Black][sq] = sq > Square::H1 ? kingAttack[sq] | kingAttack[sq - 8] : kingAttack[sq];
    }

    lookupTable.fill(0);
    initMagics(bishopInit, bishopMagic, bishopDirections, 64 - 9);
    initMagics(rookInit, rookMagic, rookDirections, 64 - 12);

#if !(defined _WIN64 || defined __x86_64__)
    hardwarePopcntSupported = false;
#else
    int regs[4];
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
    hardwarePopcntSupported = (regs[2] & (1 << 23)) != 0;
#endif
    if (hardwarePopcntSupported)
    {
        sync_cout << "Detected hardware POPCNT" << std::endl;
    }
}

void Bitboards::initMagics(std::array<MagicInit, 64>& magicInit, std::array<Magic, 64>& magic, std::array<int, 2> dir[], int shift)
{
    std::vector<int> squares;

    auto rookMask = [](int sq) 
    {
        auto result = 0ull;
        auto rk = rank(sq), fl = file(sq);
        for (auto r = rk + 1; r <= 6; r++) result |= (bits[fl + r * 8]);
        for (auto r = rk - 1; r >= 1; r--) result |= (bits[fl + r * 8]);
        for (auto f = fl + 1; f <= 6; f++) result |= (bits[f + rk * 8]);
        for (auto f = fl - 1; f >= 1; f--) result |= (bits[f + rk * 8]);
        return result;
    };

    auto bishopMask = [](int sq) 
    {
        auto result = 0ull;
        auto rk = rank(sq), fl = file(sq);
        for (auto r = rk + 1, f = fl + 1; r <= 6 && f <= 6; r++, f++) result |= (bits[f + r * 8]);
        for (auto r = rk + 1, f = fl - 1; r <= 6 && f >= 1; r++, f--) result |= (bits[f + r * 8]);
        for (auto r = rk - 1, f = fl + 1; r >= 1 && f <= 6; r--, f++) result |= (bits[f + r * 8]);
        for (auto r = rk - 1, f = fl - 1; r >= 1 && f >= 1; r--, f--) result |= (bits[f + r * 8]);
        return result;
    };

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        magic[sq].magic = magicInit[sq].magic;
        magic[sq].data = &lookupTable[magicInit[sq].index];
        auto bb = magic[sq].mask = ((shift == 64 - 12) ? rookMask(sq) : bishopMask(sq));
        auto sq88 = sq + (sq & ~7);

        squares.clear();
        while (bb)
        {
            squares.push_back(popLsb(bb));
        }

        // Loop through all possible occupations within the mask and calculate the corresponding attack sets.
        for (auto k = 0; k < (1 << squares.size()); ++k)
        {
            bb = 0;
            for (auto j = 0u; j < squares.size(); ++j)
            {
                if (k & (1 << j))
                    bb |= bits[squares[j]];
            }
            auto bb2 = 0ull;
            for (auto j = 0; j < 4; ++j)
            {
                for (auto d = 1; !((sq88 + d * dir[j][1]) & 0x88); ++d)
                {
                    bb2 |= bits[sq + d * dir[j][0]];
                    if (bb & 1ull << (sq + d * dir[j][0]))
                        break;
                }
            }
            auto j = ((bb * magic[sq].magic) >> shift);
            magic[sq].data[j] = bb2;
        }
    }
}

