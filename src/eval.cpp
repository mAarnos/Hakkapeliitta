#ifndef EVAL_CPP
#define EVAL_CPP

#include "eval.h"
#include "board.h"
#include "movegen.h"
#include "bitboard.h"
#include "magic.h"
#include "hash.h"
#include "ttable.h"
#include <intrin.h>

int drawscore = 0;

int queenTropism[64][64];
int rookTropism[64][64];
int knightTropism[64][64];
int bishopTropism[64][64];

void initializeKingTropism()
{
	int Distance[64][64];
	int DistanceNW[64] = {
		0, 1, 2, 3, 4, 5, 6, 7,
		1, 2, 3, 4, 5, 6, 7, 8,
		2, 3, 4, 5, 6, 7, 8, 9,
		3, 4, 5, 6, 7, 8, 9,10,
		4, 5, 6, 7, 8, 9,10,11,
		5, 6, 7, 8, 9,10,11,12,
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
	int bonusDiagonalDistance[14] = {5, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i, j;

	for (i = 0; i < 64; ++i) 
	{
		for (j = 0; j < 64; ++j) 
		{
		    Distance[i][j] = 14 - (abs(File(i) - File(j)) + abs(Rank(i) - Rank(j)));
		}
	}
	for (i = 0; i < 64; ++i) 
	{
		for (j = 0; j < 64; ++j) 
		{
		    queenTropism[i][j] = (Distance[i][j] * 5) / 2;
			rookTropism[i][j] = Distance[i][j] / 2;
			knightTropism[i][j] = Distance[i][j];
			bishopTropism[i][j] += bonusDiagonalDistance[abs(DistanceNE[i] - DistanceNE[j])];
			bishopTropism[i][j] += bonusDiagonalDistance[abs(DistanceNW[i] - DistanceNW[j])];
		}
	}
}

int mobilityEval(int &kingSafetyScore)
{
	int scoreOp = 0;
	int scoreEd = 0;
	int from, databaseIndex, count;
	U64 occupied = occupiedSquares;
	U64 targetBitboard;
	U64 tempPiece, tempMove, blockers;

	targetBitboard = ~whitePieces;
	tempPiece = whiteKnights;
	int kingLocation = bitScanForward(blackKing);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempMove = knightAttacks[from] & targetBitboard;
		count = (int)popcnt(tempMove);
		scoreOp += knightMobilityOpening[count];
		scoreEd += knightMobilityEnding[count];
		kingSafetyScore += knightTropism[from][kingLocation];
		tempPiece ^= setMask[from];
	}
		
	tempPiece = whiteBishops;
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		blockers = occupied & occupancyMaskBishop[from];
		databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
		tempMove = magicMovesBishop[from][databaseIndex] & targetBitboard;
		count = (int)popcnt(tempMove);
		scoreOp += bishopMobilityOpening[count];
		scoreEd += bishopMobilityEnding[count];
		kingSafetyScore += bishopTropism[from][kingLocation];
		tempPiece ^= setMask[from];
	}

	tempPiece = whiteRooks;
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		blockers = occupied & occupancyMaskRook[from];
		databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
		tempMove = magicMovesRook[from][databaseIndex] & targetBitboard;
		count = (int)popcnt(tempMove);
		scoreOp += rookMobilityOpening[count];
		scoreEd += rookMobilityEnding[count];
		kingSafetyScore += rookTropism[from][kingLocation];
		if (!(RayN[File(from)] & whitePawns))
		{
			if (!(RayN[File(from)] & blackPawns))
				scoreOp += rookOnOpenFileBonus;
			else
				scoreOp += rookOnSemiOpenFileBonus;
		}

		U64 temp = RayN[from] & tempMove & whitePawns;
		U64 temp2 = RayS[from] & tempMove & blackPawns;
		if (temp)
		{
			if (!(blackPawns & passedWhite[bitScanForward(temp)]))
				scoreEd += rookBehindPassedBonus;
		}
		if (temp2)
		{
			if (!(whitePawns & passedBlack[bitScanForward(temp2)]))
				scoreEd += rookBehindPassedBonus;
		}

		tempPiece ^= setMask[from];
	}
		
	tempPiece = whiteQueens;
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		blockers = occupied & occupancyMaskRook[from];
		databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
		tempMove = magicMovesRook[from][databaseIndex] & targetBitboard;
		blockers = occupied & occupancyMaskBishop[from];
		databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
		tempMove |= magicMovesBishop[from][databaseIndex] & targetBitboard;
		count = (int)popcnt(tempMove);
		scoreOp += queenMobilityOpening[count];
		scoreEd += queenMobilityEnding[count];
		kingSafetyScore += queenTropism[from][kingLocation];
		tempPiece ^= setMask[from];
	}

	targetBitboard = ~blackPieces;
	tempPiece = blackKnights;
	kingLocation = bitScanForward(whiteKing);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempMove = knightAttacks[from] & targetBitboard;
		count = (int)popcnt(tempMove);
		scoreOp -= knightMobilityOpening[count];
		scoreEd -= knightMobilityEnding[count];
		kingSafetyScore -= knightTropism[from][kingLocation];
		tempPiece ^= setMask[from];
	}
		
	tempPiece = blackBishops;
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		blockers = occupied & occupancyMaskBishop[from];
		databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
		tempMove = magicMovesBishop[from][databaseIndex] & targetBitboard;
		count = (int)popcnt(tempMove);
		scoreOp -= bishopMobilityOpening[count];
		scoreEd -= bishopMobilityEnding[count];
		kingSafetyScore -= bishopTropism[from][kingLocation];
		tempPiece ^= setMask[from];
	}

	tempPiece = blackRooks;
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		blockers = occupied & occupancyMaskRook[from];
		databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
		tempMove = magicMovesRook[from][databaseIndex] & targetBitboard;
		count = (int)popcnt(tempMove);
		scoreOp -= rookMobilityOpening[count];
		scoreEd -= rookMobilityEnding[count];
		kingSafetyScore -= rookTropism[from][kingLocation];
		if (!(RayN[File(from)] & blackPawns))
		{
			if (!(RayN[File(from)] & whitePawns))
				scoreOp -= rookOnOpenFileBonus;
			else
				scoreOp -= rookOnSemiOpenFileBonus;
		}
		U64 temp3 = RayN[from] & tempMove & whitePawns;
		U64 temp4 = RayS[from] & tempMove & blackPawns;
		if (temp3)
		{
			if (!(blackPawns & passedWhite[bitScanForward(temp3)]))
				scoreEd -= rookBehindPassedBonus;
		}
		if (temp4)
		{
			if (!(whitePawns & passedBlack[bitScanForward(temp4)]))
				scoreEd -= rookBehindPassedBonus;
		}
		tempPiece ^= setMask[from];
	}
		
	tempPiece = blackQueens;
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		blockers = occupied & occupancyMaskRook[from];
		databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
		tempMove = magicMovesRook[from][databaseIndex] & targetBitboard;
		blockers = occupied & occupancyMaskBishop[from];
		databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
		tempMove |= magicMovesBishop[from][databaseIndex] & targetBitboard;
		count = (int)popcnt(tempMove);
		scoreOp -= queenMobilityOpening[count];
		scoreEd -= queenMobilityEnding[count];
		kingSafetyScore -= queenTropism[from][kingLocation];
		tempPiece ^= setMask[from];
	}
	
	return ((scoreOp * (256 - phase)) + (scoreEd * phase)) / 256;
}

int pawnEval()
{
	U64 tempPiece;
	int from;
	int scoreOp = 0;
	int scoreEd = 0;
	int value = 0;

	_mm_prefetch((char *)&ptt[pHash % pttSize], _MM_HINT_NTA);

	if ((value = pttProbe(pHash)) != Invalid)
	{
		return value;
	}

	tempPiece = whitePawns;
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		// doubled pawns
		if (whitePawns & RayN[from])
		{
			scoreOp -= doubledPenaltyOpening;
			scoreEd -= doubledPenaltyEnding;
		}
		// passed pawns
		if (!(blackPawns & passedWhite[from]))
		{
			scoreOp += passedBonusOpening;
			scoreEd += passedBonusEnding;
		}
		// isolated pawns
		if (!(whitePawns & isolated[from]))
		{
			scoreOp -= isolatedPenaltyOpening;
			scoreEd -= isolatedPenaltyEnding;
		}
		// backward pawns
		if (whitePawnAttacks[from + 8] & blackPawns)
		{
			if (!(whitePawns & backwardWhite[from]))
			{
				scoreOp -= backwardPenaltyOpening;
				scoreEd -= backwardPenaltyEnding;
			}
		}
		tempPiece ^= setMask[from];
	}

	tempPiece = blackPawns;
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		// doubled pawns
		if (blackPawns & RayS[from])
		{
			scoreOp += doubledPenaltyOpening;
			scoreEd += doubledPenaltyEnding;
		}
		// passed pawns
		if (!(whitePawns & passedBlack[from]))
		{
			scoreOp -= passedBonusOpening;
			scoreEd -= passedBonusEnding;
		}
		// isolated pawns
		if (!(blackPawns & isolated[from]))
		{
			scoreOp += isolatedPenaltyOpening;
			scoreEd += isolatedPenaltyEnding;
		}
		// backward pawns
		if (blackPawnAttacks[from - 8] & whitePawns)
		{
			if (!(blackPawns & backwardBlack[from]))
			{
				scoreOp += backwardPenaltyOpening;
				scoreEd += backwardPenaltyEnding;
			}
		}
		tempPiece ^= setMask[from];
	}

	value = ((scoreOp * (256 - phase)) + (scoreEd * phase)) / 256;

	pttSave(pHash, value);

	return value;
}

int kingSafetyEval(int &kingSafetyScore)
{
	int kingLocation, zone1, zone2;
	int score = kingSafetyScore;

	kingLocation = bitScanForward(whiteKing);
	if (File(kingLocation) > 4)
	{
		// penalize pawns which have moved one square
		zone1 = popcnt(0x0000000000E00000 & whitePawns);
		score -= 10 * zone1;
		if (whitePawns & setMask[21])
			score += 5;

		// penalize pawns which have moved more than one square
		zone2 = popcnt(0x00E0E0E0E0000000 & whitePawns);
		score -= 20 * zone2;
		if (whitePawns & RayN[21])
			score += 10;

		// penalize missing pawns
		if (!(RayN[5] & whitePawns))
		{
			score -= 12;
		}
		if (!(RayN[6] & whitePawns))
		{
			score -= 25;
		}
		if (!(RayN[7] & whitePawns))
		{
			score -= 25;
		}

		// penalize missing opponent pawns
		if (!(RayN[5] & blackPawns))
		{
			score -= 15;
		}
		if (!(RayN[6] & blackPawns))
		{
			score -= 15;
		}
		if (!(RayN[7] & blackPawns))
		{
			score -= 15;
		}

		// pawn storm evaluation
		// penalize pawns on the 6th rank(from black's point of view)
		zone1 = popcnt(0x0000000000E00000 & blackPawns);
		score -= 10 * zone1;

		// penalize pawns on the 5th rank(from black's point of view)
		zone2 = popcnt(0x00000000E0000000 & blackPawns);
		score -= 5 * zone2;
	}
	else if (File(kingLocation) < 3)
	{
		// penalize pawns which have moved one square
		zone1 = popcnt(0x0000000000070000 & whitePawns);
		score -= 10 * zone1;
		if (whitePawns & setMask[18])
			score += 5;

		// penalize pawns which have moved more than one square
		zone2 = popcnt(0x0007070707000000 & whitePawns);
		score -= 20 * zone2;
		if (whitePawns & RayN[18])
			score += 10;

		// penalize missing pawns
		if (!(RayN[2] & whitePawns))
		{
			score -= 12;
		}
		if (!(RayN[1] & whitePawns))
		{
			score -= 25;
		}
		if (!(RayN[0] & whitePawns))
		{
			score -= 25;
		}

		// penalize missing opponent pawns
		if (!(RayN[2] & blackPawns))
		{
			score -= 15;
		}
		if (!(RayN[1] & blackPawns))
		{
			score -= 15;
		}
		if (!(RayN[0] & blackPawns))
		{
			score -= 15;
		}

		// pawn storm evaluation
		// penalize pawns on the 6th rank(from black's point of view)
		zone1 = popcnt(0x0000000000070000 & blackPawns);
		score -= 10 * zone1;

		// penalize pawns on the 5th rank(from black's point of view)
		zone2 = popcnt(0x0000000007000000 & blackPawns);
		score -= 5 * zone2;
	}
	else // if the king is in the center penalize open files near the king 
	{
		if (!(RayN[File(kingLocation)-1] & whitePawns & blackPawns))
			score -= 15;
		if (!(RayN[File(kingLocation)] & whitePawns & blackPawns))
			score -= 15;
		if (!(RayN[File(kingLocation)+1] & whitePawns & blackPawns))
			score -= 15;
	}

	kingLocation = bitScanForward(blackKing);
	if (File(kingLocation) > 4)
	{
		// penalize pawns which have moved one square
		zone1 = popcnt(0x0000E00000000000 & blackPawns);
		score += 10 * zone1;
		if (blackPawns & setMask[45])
			score -= 5;

		// penalize pawns which have moved more than one square
		zone2 = popcnt(0x000000E0E0E0E000 & blackPawns);
		score += 20 * zone2;
		if (blackPawns & RayS[45])
			score -= 10;

		// penalize missing pawns
		if (!(RayS[61] & blackPawns))
		{
			score += 12;
		}
		if (!(RayS[62] & blackPawns))
		{
			score += 25;
		}
		if (!(RayS[63] & blackPawns))
		{
			score += 25;
		}

		// penalize missing opponent pawns
		if (!(RayS[61] & whitePawns))
		{
			score += 15;
		}
		if (!(RayS[62] & whitePawns))
		{
			score += 15;
		}
		if (!(RayS[63] & whitePawns))
		{
			score += 15;
		}

		// pawn storm evaluation
		// penalize pawns on the 6th rank(from white's point of view)
		zone1 = popcnt(0x0000E00000000000 & whitePawns);
		score += 10 * zone1;

		// penalize pawns on the 5th rank(from white's point of view)
		zone2 = popcnt(0x000000E000000000 & whitePawns);
		score += 5 * zone2;
	}
	else if (File(kingLocation) < 3)
	{
		// penalize pawns which have moved one square
		zone1 = popcnt(0x0000070000000000 & blackPawns);
		score += 10 * zone1;
		if (blackPawns & setMask[42])
			score -= 5;

		// penalize pawns which have moved more than one square
		zone2 = popcnt(0x0000000707070700 & blackPawns);
		score += 20 * zone2;
		if (blackPawns & RayS[42])
			score -= 10;

		// penalize missing pawns
		if (!(RayS[58] & blackPawns))
		{
			score += 12;
		}
		if (!(RayS[57] & blackPawns))
		{
			score += 25;
		}
		if (!(RayS[56] & blackPawns))
		{
			score += 25;
		}

		// penalize missing opponent pawns
		if (!(RayS[58] & whitePawns))
		{
			score += 15;
		}
		if (!(RayS[57] & whitePawns))
		{
			score += 15;
		}
		if (!(RayS[56] & whitePawns))
		{
			score += 15;
		}

		// pawn storm evaluation
		// penalize pawns on the 6th rank(from white's point of view)
		zone1 = popcnt(0x0000070000000000 & whitePawns);
		score += 10 * zone1;

		// penalize pawns on the 5th rank(from white's point of view)
		zone2 = popcnt(0x0000000700000000 & whitePawns);
		score += 5 * zone2;
	}
	else // if the king is in the center penalize open files near the king 
	{
		if (!(RayN[File(kingLocation)-1] & whitePawns & blackPawns))
			score += 15;
		if (!(RayN[File(kingLocation)] & whitePawns & blackPawns))
			score += 15;
		if (!(RayN[File(kingLocation)+1] & whitePawns & blackPawns))
			score += 15;
	}

	return ((score * (256 - phase)) / 256);
}

int eval()
{
	int score, kingSafetyScore = 0;

	// evaluate for draws due to insufficient material, remove after adding tablebase support
	if (!whitePawns && !blackPawns)
	{

		// king vs king
		if (popcnt(occupiedSquares) == 2)
		{
			return drawscore;
		}
		
		if (popcnt(occupiedSquares) == 3)
		{
			// king and minor piece vs king 
			if (whiteKnights || blackKnights || whiteBishops || blackBishops )
			{ 
				return drawscore;
			}
		}
		
		if (popcnt(occupiedSquares) == 4)
		{
			// king and two knights vs king 
			if (popcnt(whiteKnights) == 2 || popcnt(blackKnights) == 2)
			{
				return drawscore;
			}
			// each side has one bishop with the bishop being of the same color
			if (popcnt(whiteBishops) == 1 && popcnt(blackBishops) == 1)
			{
				if (!((whiteBishops | blackBishops) & 0x55AA55AA55AA55AA) ||
					!((whiteBishops | blackBishops) & 0xAA55AA55AA55AA55))
				{
					return drawscore;
				}
			}
		}
	}
	
	//  material balance
	score = ((materialOpening * (256 - phase)) + (materialEnding * phase)) / 256;
	if (popcnt(whiteBishops) == 2)
		score += bishopPairBonus;
	if (popcnt(blackBishops) == 2)
		score -= bishopPairBonus;

	// most important rule of king and pawn endgames - the side with more pawns wins
	// maybe add code dealing with doubled isolated pawns? Make them only worth one pawn?
	if (phase == 256)
	{
		score += popcnt(whitePawns) * pawnEnding;
		score -= popcnt(blackPawns) * pawnEnding;
	}
	
	// trade down bonus: 5 + 3 for every point of material we are up + trade down bonus which gets bigger as we get nearer the endgame
	if (score >= 100)
	{
		score += (5 + 3 * (score / 100) + (phase * tradeDownBonus) / 256);
	}
	else if (score <= -100)
	{
		score -= (5 + 3 * (score / -100) + (phase * tradeDownBonus) / 256);
	}
	
	// piece-square tables
	score += ((scoreOpeningPST * (256 - phase)) + (scoreEndingPST * phase)) / 256;

	// mobility
	score += mobilityEval(kingSafetyScore);

	// pawn structure
	score += pawnEval();

	// king safety
	score += kingSafetyEval(kingSafetyScore);

	if (sideToMove)
	{
		return -score;
	}
	else 
	{
		return score;
	}
}
	
#endif