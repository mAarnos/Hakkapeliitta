#ifndef RANDOM_HPP_
#define RANDOM_HPP_

#include <random>
#include <cstdint>

class MT19937
{
public:
	MT19937();
	MT19937(uint32_t seed);

	inline uint32_t rand32() { return rng(); }
	inline uint64_t rand64() { return (((uint64_t)rng() << 32) | (uint64_t)rng()); }
private:
	std::mt19937 rng;
};

#endif