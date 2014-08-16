#include "zobrist.hpp"
#include <random>
#include "square.hpp"
#include "bitboard.hpp"

std::array<HashKey, 64> Zobrist::pieceHash[12];
std::array<HashKey, 8> Zobrist::materialHash[12];
std::array<HashKey, 16> Zobrist::castlingRightsHash;
std::array<HashKey, 64> Zobrist::enPassantHash;
HashKey Zobrist::turnHash;

void Zobrist::initialize()
{
    std::mt19937_64 rng(123456789);

    for (auto p = 0; p < 12; ++p)
    {
        for (Square sq = Square::A1; sq <= Square::H8; ++sq)
        {
            pieceHash[p][sq] = rng();
        }
        for (auto j = 0; j < 8; ++j)
        {
            materialHash[p][j] = rng();
        }
    }

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        enPassantHash[sq] = rng();
    }

    for (auto cr = 0; cr < 16; ++cr)
    {
        Bitboard castlingRight = cr;
        while (castlingRight)
        {
            auto key = castlingRightsHash[1 << Bitboards::lsb(castlingRight)];
            castlingRight &= castlingRight - 1;
            castlingRightsHash[cr] ^= key ? key : rng();
        }
    }

    turnHash = rng();
}
