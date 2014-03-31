#ifndef EVAL_CPP
#define EVAL_CPP

#include "eval.h"
#include "hash.h"
#include "magic.h"

int drawScore = 0;

map<uint64_t, int> knownEndgames;

array<int, Squares> queenTropism[Squares];
array<int, Squares> rookTropism[Squares];
array<int, Squares> knightTropism[Squares];
array<int, Squares> bishopTropism[Squares];

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

void initializeKingTropism()
{
	int Distance[64][64];
	int DistanceNW[64] = {
		0, 1, 2, 3, 4, 5, 6, 7,
		1, 2, 3, 4, 5, 6, 7, 8,
		2, 3, 4, 5, 6, 7, 8, 9,
		3, 4, 5, 6, 7, 8, 9,10,
		4, 5, 6, 7, 8, 9, 10,11,
		5, 6, 7, 8, 9,10, 11,12,
		6, 7, 8, 9,10,11,12,13,
		7, 8, 9,10,11,12,13,14
	};
	int DistanceNE[64] = {
		 7, 6, 5, 4, 3, 2, 1, 0,
		 8, 7, 6, 5, 4, 3, 2, 1,
		 9, 8, 7, 6, 5, 4, 3, 2,
		10, 9, 8, 7, 6, 5, 4, 3,
		11,10, 9, 8, 7, 6, 5, 4,
		12,11,10, 9, 8, 7, 6, 5,
		13,12,11,10, 9, 8, 7, 6,
		14,13,12,11,10, 9, 8, 7
	};
	int nTropism[15] = { 14, 22, 29, 28, 19, -1, -6, -10, -11, -12, -13, -14, -15, -16, -17 };
	int bTropism[15] = { 6, -12, 4, -16, -11, -17, -6, -14, -9, -17, -1, -17, -4, 3, 7 };
	int rTropism[15] = { 7, 22, 23, 22, 22, 16, -2, -5, -14, -10, -8, -15, -16, -17, -17 };
	int qTropism[15] = { 35, 49, 47, 44, 40, 14, 3, 0, -2, 1, 4, -3, -5, -6, -9 };
	int i, j;

	for (i = A1; i <= H8; i++)
	{
		for (j = A1; j <= H8; j++)
		{
			Distance[i][j] = (abs((i % 8) - (j % 8)) + abs((i / 8) - (j / 8)));
		}
	}
	for (i = A1; i <= H8; i++)
	{
		for (j = A1; j <= H8; j++)
		{
			queenTropism[i][j] = qTropism[Distance[i][j]];
			rookTropism[i][j] = rTropism[Distance[i][j]];
			knightTropism[i][j] = nTropism[Distance[i][j]];
			bishopTropism[i][j] += bTropism[abs(DistanceNE[i] - DistanceNE[j])];
			bishopTropism[i][j] += bTropism[abs(DistanceNW[i] - DistanceNW[j])];
		}
	}
}

int mobilityEval(Position & pos, int & kingTropismScore)
{
	int scoreOp = 0;
	int scoreEd = 0;
	int from, count;
	uint64_t occupied = pos.getOccupiedSquares();
	uint64_t tempPiece, tempMove;

	// White
	uint64_t targetBitboard = ~pos.getPieces(White);
	int kingLocation = bitScanForward(pos.getBitboard(Black, King));

	tempPiece = pos.getBitboard(White, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = knightAttacks[from] & targetBitboard;

		count = popcnt(tempMove);
		scoreOp += knightMobilityOpening[count];
		scoreEd += knightMobilityEnding[count];
		kingTropismScore += knightTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(White, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = bishopAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp += bishopMobilityOpening[count];
		scoreEd += bishopMobilityEnding[count];
		kingTropismScore += bishopTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(White, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = rookAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp += rookMobilityOpening[count];
		scoreEd += rookMobilityEnding[count];
		kingTropismScore += rookTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(White, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = queenAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp += queenMobilityOpening[count];
		scoreEd += queenMobilityEnding[count];
		kingTropismScore += queenTropism[from][kingLocation];
	}

	// Black
	kingLocation = bitScanForward(pos.getBitboard(White, King));
	targetBitboard = ~pos.getPieces(Black);

	tempPiece = pos.getBitboard(Black, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = knightAttacks[from] & targetBitboard;

		count = popcnt(tempMove);
		scoreOp -= knightMobilityOpening[count];
		scoreEd -= knightMobilityEnding[count];
		kingTropismScore -= knightTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(Black, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = bishopAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp -= bishopMobilityOpening[count];
		scoreEd -= bishopMobilityEnding[count];
		kingTropismScore -= bishopTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(Black, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = rookAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp -= rookMobilityOpening[count];
		scoreEd -= rookMobilityEnding[count];
		kingTropismScore -= rookTropism[from][kingLocation];
	}

	tempPiece = pos.getBitboard(Black, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = queenAttacks(from, occupied) & targetBitboard;

		count = popcnt(tempMove);
		scoreOp -= queenMobilityOpening[count];
		scoreEd -= queenMobilityEnding[count];
		kingTropismScore -= queenTropism[from][kingLocation];
	}

	return ((scoreOp * (256 - pos.getPhase())) + (scoreEd * pos.getPhase())) / 256;
}

int eval(Position & pos)
{
	int score = 0, kingTropismScore = 0;

	// Checks if we are in a known endgame.
	// If we are we can straight away return the score for the endgame.
	// At the moment only detects draws, if wins will be included this must be made to return things in negamax fashion.
	if (knownEndgames.count(pos.getMaterialHash()))
	{
		return knownEndgames[pos.getMaterialHash()];
	}

	score += mobilityEval(pos, kingTropismScore);

	if (pos.getSideToMove())
	{
		return -score;
	}
	else
	{
		return score;
	}
}

#endif