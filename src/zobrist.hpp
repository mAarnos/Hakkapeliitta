#ifndef ZOBRIST_HPP_
#define ZOBRIST_HPP_

#include <cstdint>
#include <array>
#include "piece.hpp"
#include "square.hpp"

// No further improvement necessary.

using HashKey = uint64_t;

class Zobrist
{
public:
    static void initialize();

    static HashKey pieceHashKey(Piece p, Square sq) { return piece[p][sq]; }
    static HashKey materialHashKey(Piece p, int amount) { return material[p][amount]; }
    static HashKey castlingRightsHashKey(int castlingRight) { return castling[castlingRight]; }
    static HashKey enPassantHashKey(Square enPassant) { return ep[enPassant]; }
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
