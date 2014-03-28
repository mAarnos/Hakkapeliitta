#ifndef EVAL_H
#define EVAL_H

#include "defs.h"
#include "position.h"

const int mateScore = 32767;
const int maxMateScore = 32767 - maxGameLength;

// Watch out with Infinite as it is bigger than mateScore therefore it could be detected as an mate score even though it is not.
inline bool isMateScore(int score)
{
	return (score <= -maxMateScore || score >= maxMateScore);
}

const array<int, Pieces> pieceValuesOpening = {
	88, 235, 263, 402, 892, mateScore
};

const array<int, Pieces> pieceValuesEnding = {
	112, 258, 286, 481, 892, mateScore
};

const array<int, Squares> openingPST[6] = {
	// Pawn.
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
	// Knight
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
	// Bishop
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
	// Rook
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
	// Queen
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
	// King
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
	// Pawn
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
	// Knight
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
	// Bishop
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
	// Rook
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
	// Queen
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
	// King
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

// The flip array is used to calculate the piece/square values for black pieces. 
// If the piece/square value of a white pawn is pst[sq] then the value of a black pawn is pst[flip[sq]] 
const array<int, Squares> flip = {
	56, 57, 58, 59, 60, 61, 62, 63,
	48, 49, 50, 51, 52, 53, 54, 55,
	40, 41, 42, 43, 44, 45, 46, 47,
	32, 33, 34, 35, 36, 37, 38, 39,
	24, 25, 26, 27, 28, 29, 30, 31,
	16, 17, 18, 19, 20, 21, 22, 23,
	8,  9,  10, 11, 12, 13, 14, 15,
	0,  1,  2,  3,  4,  5,  6,  7
};

const int knightMobilityOpening[9] = {
	-12, -8, -4, 0, 4, 8, 12, 16,
	20
};

const int knightMobilityEnding[9] = {
	-12, -8, -4, 0, 4, 8, 12, 16,
	20
};

const array<int, 14> bishopMobilityOpening = {
	-9, -6, -3, 0, 3, 6, 9, 12,
	15, 18, 21, 24, 27, 30
};

const array<int, 14> bishopMobilityEnding = {
	-12, -9, -6, -3, 0, 3, 6, 9,
	12, 15, 18, 21, 24, 27
};

const array<int, 15> rookMobilityOpening = {
	-6, -4, -2, 0, 2, 4, 6, 8,
	10, 12, 14, 16, 18, 20, 22
};

const array<int, 15> rookMobilityEnding = {
	-10, -8, -6, -4, -2, 0, 2, 4,
	6, 8, 10, 12, 14, 16, 18
};

const array<int, 28> queenMobilityOpening = {
	-7, -6, -5, -4, -3, -2, -1, 0,
	1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20
};

const array<int, 28> queenMobilityEnding = {
	-10, -9, -8, -7, -6, -5, -4, -3,
	-2, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17
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
const int tradeDownBonus = 3;

const int pawnShelterAdvancedPawnPenalty = 11;
const int pawnShelterMissingPawnPenalty = 38;
const int pawnShelterMissingOpponentPawnPenalty = -2;
const int pawnStormClosePenalty = 22;
const int pawnStormFarPenalty = 6;
const int kingInCenterOpenFilePenalty = 23;

const int rookOnOpenFileBonus = 10;
const int rookOnSemiOpenFileBonus = 5;
const int rookBehindPassedBonus = 20;

const int pawnPhase = 0;
const int knightPhase = 1;
const int bishopPhase = 1;
const int rookPhase = 2;
const int queenPhase = 4;
const int totalPhase = pawnPhase * 16 + knightPhase * 4 + bishopPhase * 4 + rookPhase * 4 + queenPhase * 2;

extern int drawScore;

extern int eval(Position & pos);

#endif