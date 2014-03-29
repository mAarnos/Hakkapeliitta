#ifndef EVAL_CPP
#define EVAL_CPP

#include "eval.h"
#include "hash.h"

int drawScore = 0;
map<uint64_t, int> knownEndgames;

void initializeKnownEndgames()
{
	// King vs king: draw
	uint64_t matHash = materialHash[WhiteKing][0] ^ materialHash[BlackKing][0];
	knownEndgames[matHash] = drawScore;

	// King and a minor piece vs king: draw
	for (int i = White; i <= Black; i++)
	{
		for (int j = Knight; j <= Bishop; j++)
		{
			knownEndgames[matHash ^ materialHash[j + i * 6][0]] = drawScore;
		}
	}

	// King and two knights vs king: draw
	for (int i = White; i <= Black; i++)
	{
		knownEndgames[matHash ^ materialHash[Knight + i * 6][0] ^ materialHash[Knight + i * 6][1]] = drawScore;
	}

	// King and a minor piece vs king and a minor piece: draw
	for (int i = Knight; i <= Bishop; i++)
	{
		for (int j = Knight; j <= Bishop; j++)
		{
			knownEndgames[matHash ^ materialHash[White + i][0] ^ materialHash[Black * 6 + j][0]] = drawScore;
		}
	}

	// King and two bishops vs king and a bishop: draw
	for (int i = White; i <= Black; i++)
	{
		knownEndgames[matHash ^ materialHash[Bishop + i * 6][0] ^ materialHash[Bishop + i * 6][1] ^ materialHash[Bishop + !i * 6][0]] = drawScore;
	}

	// King and either two knights or a knight and a bishop vs king and a minor piece: draw
	for (int i = White; i <= Black; i++)
	{
		for (int j = Knight; j <= Bishop; j++)
		{
			for (int k = Knight; k <= Bishop; k++)
			{
				knownEndgames[matHash ^ materialHash[Knight + i * 6][0] ^ materialHash[j + i * 6][j == Knight] ^ materialHash[k + !i * 6][0]] = drawScore;
			}
		}
	}
}

int eval(Position & pos)
{
	int score = 0, kingSafetyScore = 0;

	if (knownEndgames.count(pos.getMaterialHash()))
	{
		return knownEndgames.at(pos.getMaterialHash());
	}

	return score;
}

#endif