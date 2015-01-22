#ifndef ZOBRIST_HPP_
#define ZOBRIST_HPP_

#include <cstdint>
#include <array>
#include "piece.hpp"
#include "square.hpp"

using HashKey = uint64_t;

class Zobrist
{
public:
    static void initialize();

    static HashKey pieceHashKey(const Piece p, const Square sq) { return piece[p][sq]; }
    static HashKey materialHashKey(const Piece p, const int amount) { return material[p][amount]; }
    static HashKey castlingRightsHashKey(const int castlingRight) { return castling[castlingRight]; }
    static HashKey enPassantHashKey(const Square enPassant) { return ep[enPassant]; }
    static HashKey turnHashKey() { return turn; }
    static HashKey mangleHashKey() { return mangle; }
private:
    static std::array<std::array<HashKey, 64>, 12> piece;
    static std::array<std::array<HashKey, 8>, 12> material;
    static std::array<HashKey, 16> castling;
    static std::array<HashKey, 64> ep;
    static HashKey turn;
    static HashKey mangle;
};

#endif
