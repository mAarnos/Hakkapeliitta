#ifndef ZOBRIST_HPP_
#define ZOBRIST_HPP_

#include <cstdint>
#include <array>

using HashKey = uint64_t;

class Zobrist
{
public:
    static void initialize();

    static std::array<HashKey, 64> pieceHash[12];
    static std::array<HashKey, 8> materialHash[12];
	static std::array<HashKey, 16> castlingRightsHash;
	static std::array<HashKey, 64> enPassantHash;
	static HashKey turnHash;
};

#endif
