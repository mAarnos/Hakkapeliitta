#ifndef RANDOM_H_
#define RANDOM_H_

#include "defs.hpp"

// Our PRNG of choice is WELL, or Well Equidistributed Long-period Linear. It is an improved version of Mersenne Twister. For more details see
// L'Ecuyer, Pierre; Panneton, Francois; Matsumoto, Makoto (2006), Improved Long-Period Generators Based on Linear Recurrences Modulo 2
// Our implementation is based on the code given by Chris Lomont.
class WELL512
{
	public:
		// Initializes the PRNG with either the current time(default) or a specific number as the seed.
		WELL512(uint32_t seed = 0);

		// Generates random 32-bit values.
		uint32_t rand();
	private:
		array<uint32_t, 16> state;
		uint32_t index;
};

#endif