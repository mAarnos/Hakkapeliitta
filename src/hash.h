#ifndef HASH_H
#define HASH_H

#include "defs.h"
#include <boost/random.hpp>
#include <boost/random/uniform_int_distribution.hpp>

// the mersenne twister pseudo-random number generator
// use rng.seed(time(0)) to get different numbers every time
// the generator is called as follows: random64(rng) 
// random32 gives a random 32-bit integer
// random64 gives a random 64-bit integer
extern boost::random::mt19937_64 rng;   
const boost::random::uniform_int_distribution<int> random32(INT_MIN, INT_MAX); 
const boost::random::uniform_int_distribution<__int64> random64(_I64_MIN, _I64_MAX);

extern U64 pieceHash[16][64];
extern U64 materialHash[2][6][8];
extern U64 side;
extern U64 ep[64];
extern U64 castle[16];

extern void initializeHash();
extern U64 setHash();
extern U64 setPHash();

#endif
