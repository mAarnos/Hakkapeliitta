#ifndef SEARCH_CPP
#define SEARCH_CPP

#include "search.h"
#include "movegen.h"
#include "ttable.h"

uint64_t perft(Position & pos, int depth)
{
	uint64_t nodes = 0;

	if (depth == 0)
	{
		return 1;
	}

	Move moveStack[256];
	int generatedMoves = generateMoves(pos, moveStack);
	for (int i = 0; i < generatedMoves; i++)
	{
		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}
		nodes += perft(pos, depth - 1);
		pos.unmakeMove(moveStack[i]);
	}

	return nodes;
}

uint64_t perftHash(Position & pos, int depth)
{
	uint64_t nodes = 0;
	uint64_t value;

	if (depth == 0)
	{
		return 1;
	}

	if ((value = perftTTProbe(pos, depth)) != probeFailed)
	{
		return value;
	}

	Move moveStack[256];
	int generatedMoves = generateMoves(pos, moveStack);
	for (int i = 0; i < generatedMoves; i++)
	{
		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}
		nodes += perftHash(pos, depth - 1);
		pos.unmakeMove(moveStack[i]);
	}

	perftTTSave(pos, nodes, depth);

	return nodes;
}

#endif