#ifndef HASH_H_
#define HASH_H_

#include "defs.hpp"

extern array<uint64_t, Squares> pieceHash[12];
extern array<uint64_t, 8> materialHash[12];
extern array<uint64_t, 16> castlingRightsHash;
extern array<uint64_t, Squares> enPassantHash;
extern uint64_t turnHash;

void initializeHash();

#endif
