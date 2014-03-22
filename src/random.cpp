#ifndef RANDOM_CPP
#define RANDOM_CPP

#include "random.h"

WELL512::WELL512(uint32_t seed) :
index(0)
{
	for (int i = 0; i < 16; i++)
	{
		if (!seed)
		{
			seed = (uint32_t)time(0);
		};
		uint32_t mask = ~0u;
		state[0] = seed & mask;
		for (int j = 1; j < 16; ++j)
			state[j] = (1812433253UL * (state[j - 1] ^ (state[j - 1] >> 30)) + j) & mask;
	}
}

uint32_t WELL512::rand()
{
	uint32_t a, b, c, d;
	a = state[index];
	c = state[(index + 13) & 15];
	b = a ^ c ^ (a << 16) ^ (c << 15);
	c = state[(index + 9) & 15];
	c ^= (c >> 11);
	a = state[index] = b ^ c;
	d = a ^ ((a << 5) & 0xDA442D24UL);
	index = (index + 15) & 15;
	a = state[index];
	state[index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
	return state[index];
};

#endif