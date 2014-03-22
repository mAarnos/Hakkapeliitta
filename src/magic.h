#ifndef MAGIC_H
#define MAGIC_H

#include "defs.h"

// TODO: test that the fixed shift magics work.

class Magic
{
	public:
		uint64_t *data;
		uint64_t mask;
		uint64_t magic;
};

class MagicInit
{
	public:
		uint64_t magic;
		int index;
};

extern uint64_t lookupTable[97264];
extern Magic bishopMagic[64];
extern Magic rookMagic[64];

void initializeMagics();

inline uint64_t bishopAttacks(int sq, uint64_t occupied)
{
	Magic * mag = &bishopMagic[sq];
	return mag->data[((occupied & mag->mask) * mag->magic) >> (64 - 9)];
}

inline uint64_t rookAttacks(int sq, uint64_t occupied)
{
	Magic * mag = &rookMagic[sq];
	return mag->data[((occupied & mag->mask) * mag->magic) >> (64 - 12)];
}

#endif