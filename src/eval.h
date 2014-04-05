#ifndef EVAL_H
#define EVAL_H

#include "defs.h"
#include "position.h"

const int mateScore = 32767;
const int maxMateScore = 32767 - maxGameLength;
// Is the value here enough?
const int infinity = mateScore + 1;

// Watch out with Infinite as it is bigger than mateScore therefore it could be detected as an mate score even though it is not.
inline bool isMateScore(int64_t score)
{
	return (score <= -maxMateScore || score >= maxMateScore);
}

const array<int, Pieces> pieceValuesOpening = {
	88, 235, 263, 402, 892, 0
};

const array<int, Pieces> pieceValuesEnding = {
	112, 258, 286, 481, 892, 0
};

const array<int, Squares> openingPST[Pieces] = {
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		-33,-18,-13,-18,-18,-13,-18,-33,
		-28,-23,-13, -3, -3,-13,-23,-28,
		-33,-18,-13, 12, 12,-13,-18,-33,
		-18,-13, -8, 17, 17, -8,-13,-18,
		7, 12, 17, 22, 22, 17, 12, 7,
		42, 42, 42, 42, 42, 42, 42, 42,
		0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		-36, -26, -16, -6, -6, -16, -26, -36,
		-26, -16, -6, 9, 9, -6, -16, -26,
		-16, -6, 9, 29, 29, 9, -6, -16,
		-6, 9, 29, 39, 39, 29, 9, -6,
		-6, 9, 39, 49, 49, 39, 9, -6,
		-16, -6, 19, 59, 59, 19, -6, -16,
		-26, -16, -6, 9, 9, -6, -16, -26,
		-36, -26, -16, -6, -6, -16, -26, -36
	},
	{
		-24, -19, -14, -9, -9, -14, -19, -24,
		-9, 6, 1, 6, 6, 1, 6, -9,
		-4, 1, 6, 11, 11, 6, 1, -4,
		1, 6, 11, 16, 16, 11, 6, 1,
		1, 11, 11, 16, 16, 11, 11, 1,
		-4, 1, 6, 11, 11, 6, 1, -4,
		-9, -4, 1, 6, 6, 1, -4, -9,
		-14, -9, -4, 1, 1, -5, -9, -14
	},
	{
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		7, 7, 9, 12, 12, 9, 7, 7,
		-3, -3, -1, 2, 2, -1, -3, -3,
	},
	{
		-19, -14, -9, -4, -4, -9, -14, -19,
		-9, -4, 1, 6, 6, 1, -4, -9,
		-4, 1, 6, 11, 11, 6, 1, -4,
		1, 6, 11, 16, 16, 11, 6, 1,
		1, 6, 11, 16, 16, 11, 6, 1,
		-4, 1, 6, 11, 11, 6, 1, -4,
		-9, -4, 1, 6, 6, 1, -4, -9,
		-14, -9, -4, 1, 1, -4, -9, -14
	},
	{
		5, 10, 2, 0, 0, 6, 10, 4,
		5, 5, 0, -5, -5, 0, 5, 5,
		-5, -5, -5, -10, -10, -5, -5, -5,
		-10, -10, -20, -30, -30, -20, -10, -10,
		-20, -25, -30, -40, -40, -30, -25, -20,
		-40, -40, -50, -60, -60, -50, -40, -40,
		-50, -50, -60, -60, -60, -60, -50, -50,
		-60, -60, -60, -60, -60, -60, -60, -60
	}
};

const array<int, Squares> endingPST[6] = {
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		-22, -22, -22, -22, -22, -22, -22, -22,
		-17, -17, -17, -17, -17, -17, -17, -17,
		-12, -12, -12, -12, -12, -12, -12, -12,
		-7, -7, -7, -7, -7, -7, -7, -7,
		18, 18, 18, 18, 18, 18, 18, 18,
		38, 38, 38, 38, 38, 38, 38, 38,
		0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		-34, -24, -14, -4, -4, -14, -24, -34,
		-24, -14, -4, 11, 11, -4, -14, -24,
		-14, -4, 11, 31, 31, 11, -4, -14,
		-4, 11, 31, 41, 41, 31, 11, -4,
		-4, 11, 31, 41, 41, 31, 11, -4,
		-14, -4, 11, 31, 31, 11, -4, -14,
		-24, -14, -4, 11, 11, -4, -14, -24,
		-34, -24, -14, -4, -4, -14, -24, -34
	},
	{
		-15, -10, -5, 0, 0, -5, -10, -15,
		-10, -5, 0, 5, 5, 0, -5, -10,
		-5, 0, 5, 10, 10, 5, 0, -5,
		0, 5, 10, 15, 15, 10, 5, 0,
		0, 5, 10, 15, 15, 10, 5, 0,
		-5, 0, 5, 10, 10, 5, 0, -5,
		-10, -5, 0, 5, 5, 0, -5, -10,
		-15, -10, -5, 0, 0, -5, -10, -15
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	},
	{
		-15, -10, -5, 0, 0, -5, -10, -15,
		-10, -5, 0, 5, 5, 0, -5, -10,
		-5, 0, 5, 10, 10, 5, 0, -5,
		0, 5, 10, 15, 15, 10, 5, 0,
		0, 5, 10, 15, 15, 10, 5, 0,
		-5, 0, 5, 10, 10, 5, 0, -5,
		-10, -5, 0, 5, 5, 0, -5, -10,
		-15, -10, -5, 0, 0, -5, -10, -15
	},
	{
		-38, -28, -18, -8, -8, -18, -28, -38,
		-28, -18, -8, 13, 13, -8, -18, -28,
		-18, -8, 13, 43, 43, 13, -8, -18,
		-8, 13, 43, 53, 53, 43, 13, -8,
		-8, 13, 43, 53, 53, 43, 13, -8,
		-18, -8, 13, 43, 43, 13, -8, -18,
		-28, -18, -8, 13, 13, -8, -18, -28,
		-38, -28, -18, -8, -8, -18, -28, -38
	}
};

const array<int, Squares> mobilityOpening[6] = {
	{},
	{ -12, -8, -4, 0, 4, 8, 12, 16, 20 },
	{ -9, -6, -3, 0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30 },
	{ -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22 },
	{ -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 },
	{}
};

const array<int, Squares> mobilityEnding[6] = {
	{},
	{ -12, -8, -4, 0, 4, 8, 12, 16, 20 },
	{ -12, -9, -6, -3, 0, 3, 6, 9, 12, 15, 18, 21, 24, 27 },
	{ -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18 },
	{ -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 },
	{}
};

const int doubledPenaltyOpening = 15;
const int doubledPenaltyEnding = 16;
const int passedBonusOpening = -4;
const int passedBonusEnding = 51;
const int isolatedPenaltyOpening = 13;
const int isolatedPenaltyEnding = 17;
const int backwardPenaltyOpening = 13;
const int backwardPenaltyEnding = 9;
const int bishopPairBonusOpening = 32;
const int bishopPairBonusEnding = 32;

const int pawnShelterAdvancedPawnPenalty = 11;
const int pawnShelterMissingPawnPenalty = 38;
const int pawnShelterMissingOpponentPawnPenalty = -2;
const int pawnStormClosePenalty = 22;
const int pawnStormFarPenalty = 6;
const int kingInCenterOpenFilePenalty = 23;

const int rookOnOpenFileBonus = 10;
const int rookOnSemiOpenFileBonus = 5;
const int rookBehindPassedBonus = 20;

extern int drawScore;

extern array<int, 64> pieceSquareTableOpening[12];
extern array<int, 64> pieceSquareTableEnding[12];

extern map<uint64_t, int> knownEndgames;

void initializeEval();
int eval(Position & pos);

#endif