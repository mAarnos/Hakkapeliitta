#ifndef SEARCH_CPP
#define SEARCH_CPP

#include "search.h"
#include "movegen.h"

uint64_t nodeCount = 0;

int perft(int depth, Position & pos) 
{
	int i;

	if (depth == 0)
	{
		return 1;
	}

	Move moveStack[256];
	int generatedMoves = generateMoves(pos, moveStack);
	for (i = 0; i < generatedMoves; i++)
	{
		if (!(pos.makeMove(moveStack[i])))
		{
			continue;
		}
		nodeCount += perft(depth - 1, pos);
		pos.unmakeMove(moveStack[i]);
	}
	return 0;
}

#endif