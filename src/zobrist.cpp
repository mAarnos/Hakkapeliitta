#include "zobrist.hpp"
#include <random>
#include "square.hpp"
#include "piece.hpp"
#include "bitboard.hpp"

// No further improvement necessary.

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
            auto key = castling[1ull << Bitboards::popLsb(castlingRight)];
            castling[cr] ^= key ? key : rng(); 
        }
    }

    turn = rng(); 
    mangle = rng();
}
