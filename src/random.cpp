#include "random.hpp"
#include <chrono>

// TODO: nothing

MT19937::MT19937()
{
	// First we get the amount of nanoseconds since epoch and get the first 32 bits of that.
	auto seed = (uint32_t)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	// Then we xor that with some random 32-bit value and we have our seed, hopefully of good quality.
	seed ^= 0x8BD3DF2F;
	rng.seed(seed);
}

MT19937::MT19937(uint32_t seed) :
rng(seed)
{
}
