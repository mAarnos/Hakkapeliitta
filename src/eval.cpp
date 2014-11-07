#include "eval.hpp"
#include "hash.hpp"
#include "magic.hpp"
#include "ttable.hpp"

int drawScore = 0;

array<int, Squares> pieceSquareTableOpening[12];
array<int, Squares> pieceSquareTableEnding[12];

map<uint64_t, int> knownEndgames;

const std::array<int, 8> passedBonusOpening = {
    0, 0, 5, 10, 20, 25, 50, 0
};

const std::array<int, 8> passedBonusEnding = {
    0, 0, 10, 15, 30, 40, 80, 0
};

const std::array<int, 8> doubledPenaltyOpening = {
    8, 12, 16, 16, 16, 16, 12, 8
};

const std::array<int, 8> doubledPenaltyEnding = {
    16, 20, 24, 24, 24, 24, 20, 16
};

const std::array<int, 8> isolatedPenaltyOpening = {
    8, 12, 16, 16, 16, 16, 12, 8
};

const std::array<int, 8> isolatedPenaltyEnding = {
    12, 16, 20, 20, 20, 20, 16, 12
};

const std::array<int, 8> backwardPenaltyOpening = {
    6, 10, 12, 12, 12, 12, 10, 6
};

const std::array<int, 8> backwardPenaltyEnding = {
    12, 14, 16, 16, 16, 16, 14, 12
};

void initializeKnownEndgames()
{
	// King vs king: draw
	uint64_t matHash = materialHash[WhiteKing][0] ^ materialHash[BlackKing][0];
	knownEndgames[matHash] = 0;

	// King and a minor piece vs king: draw
	for (int i = White; i <= Black; i++)
	{
		for (int j = Knight; j <= Bishop; j++)
		{
			knownEndgames[matHash ^ materialHash[j + i * 6][0]] = 0;
		}
	}

	// King and two knights vs king: draw
	for (int i = White; i <= Black; i++)
	{
		knownEndgames[matHash ^ materialHash[Knight + i * 6][0] ^ materialHash[Knight + i * 6][1]] = 0;
	}

	// King and a minor piece vs king and a minor piece: draw
	for (int i = Knight; i <= Bishop; i++)
	{
		for (int j = Knight; j <= Bishop; j++)
		{
			knownEndgames[matHash ^ materialHash[White + i][0] ^ materialHash[Black * 6 + j][0]] = 0;
		}
	}

	// King and two bishops vs king and a bishop: draw
	for (int i = White; i <= Black; i++)
	{
		knownEndgames[matHash ^ materialHash[Bishop + i * 6][0] ^ materialHash[Bishop + i * 6][1] ^ materialHash[Bishop + !i * 6][0]] = 0;
	}

	// King and either two knights or a knight and a bishop vs king and a minor piece: draw
	for (int i = White; i <= Black; i++)
	{
		for (int j = Knight; j <= Bishop; j++)
		{
			for (int k = Knight; k <= Bishop; k++)
			{
				knownEndgames[matHash ^ materialHash[Knight + i * 6][0] ^ materialHash[j + i * 6][j == Knight] ^ materialHash[k + !i * 6][0]] = 0;
			}
		}
	}
}

void initializeEval()
{
	initializeKnownEndgames();

	array<int, Squares> flip = {
		56, 57, 58, 59, 60, 61, 62, 63,
		48, 49, 50, 51, 52, 53, 54, 55,
		40, 41, 42, 43, 44, 45, 46, 47,
		32, 33, 34, 35, 36, 37, 38, 39,
		24, 25, 26, 27, 28, 29, 30, 31,
		16, 17, 18, 19, 20, 21, 22, 23,
		8, 9, 10, 11, 12, 13, 14, 15,
		0, 1, 2, 3, 4, 5, 6, 7
	};

	for (int i = Pawn; i <= King; i++)
	{
		for (int sq = A1; sq <= H8; sq++)
		{
			pieceSquareTableOpening[i][sq] = openingPST[i][sq] + pieceValuesOpening[i];
			pieceSquareTableEnding[i][sq] = endingPST[i][sq] + pieceValuesEnding[i];

			pieceSquareTableOpening[i + Black * 6][sq] = -(openingPST[i][flip[sq]] + pieceValuesOpening[i]);
			pieceSquareTableEnding[i + Black * 6][sq] = -(endingPST[i][flip[sq]] + pieceValuesEnding[i]);
		}
	}
}

int mobilityEval(Position & pos, int phase, std::array<int, 2> & kingSafetyScore, bool & zugzwangLikely)
{
	int scoreOp = 0;
	int scoreEd = 0;
	int from, count;
	uint64_t occupied = pos.getOccupiedSquares();
	uint64_t tempPiece, tempMove;
    auto attackUnits = 0;
    std::array<uint64_t, 2> attacks = { 0, 0 };

	// White
	uint64_t targetBitboard = ~pos.getPieces(White);
	auto opponentKingZone = kingZone[Black][bitScanForward(pos.getBitboard(Black, King))];

	tempPiece = pos.getBitboard(White, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = knightAttacks[from] & targetBitboard;
        attacks[White] |= tempMove;

		count = popcnt(tempMove);
		scoreOp += mobilityOpening[Knight][count];
		scoreEd += mobilityEnding[Knight][count];

        attackUnits += attackWeight[Knight] * popcnt(tempMove & opponentKingZone);
	}

	tempPiece = pos.getBitboard(White, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = bishopAttacks(from, occupied) & targetBitboard;
        attacks[White] |= tempMove;

		count = popcnt(tempMove);
		scoreOp += mobilityOpening[Bishop][count];
		scoreEd += mobilityEnding[Bishop][count];

        tempMove = bishopAttacks(from, occupied ^ pos.getBitboard(White, Queen)) & targetBitboard;
        attackUnits += attackWeight[Bishop] * popcnt(tempMove & opponentKingZone);
	}

	tempPiece = pos.getBitboard(White, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = rookAttacks(from, occupied) & targetBitboard;
        attacks[White] |= tempMove;

		count = popcnt(tempMove);
		scoreOp += mobilityOpening[Rook][count];
		scoreEd += mobilityEnding[Rook][count];

        tempMove = rookAttacks(from, occupied ^ pos.getBitboard(White, Queen) ^ pos.getBitboard(White, Rook)) & targetBitboard;
        attackUnits += attackWeight[Rook] * popcnt(tempMove & opponentKingZone);

		if (!(files[File(from)] & pos.getBitboard(White, Pawn)))
		{
			if (!(files[File(from)] & pos.getBitboard(Black, Pawn)))
			{
				scoreOp += rookOnOpenFileBonus;
			}
			else
			{
				scoreOp += rookOnSemiOpenFileBonus;
			}
		}
	}

	tempPiece = pos.getBitboard(White, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = queenAttacks(from, occupied) & targetBitboard;
        attacks[White] |= tempMove;

		count = popcnt(tempMove);
		scoreOp += mobilityOpening[Queen][count];
		scoreEd += mobilityEnding[Queen][count];

        attackUnits += attackWeight[Queen] * popcnt(tempMove & opponentKingZone);
	}

    kingSafetyScore[White] = attackUnits;

	// Black
    opponentKingZone = kingZone[White][bitScanForward(pos.getBitboard(White, King))];
	targetBitboard = ~pos.getPieces(Black);
    attackUnits = 0;

	tempPiece = pos.getBitboard(Black, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = knightAttacks[from] & targetBitboard;
        attacks[Black] |= tempMove;

		count = popcnt(tempMove);
		scoreOp -= mobilityOpening[Knight][count];
		scoreEd -= mobilityEnding[Knight][count];

        attackUnits += attackWeight[Knight] * popcnt(tempMove & opponentKingZone);
	}

	tempPiece = pos.getBitboard(Black, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = bishopAttacks(from, occupied) & targetBitboard;
        attacks[Black] |= tempMove;

		count = popcnt(tempMove);
		scoreOp -= mobilityOpening[Bishop][count];
		scoreEd -= mobilityEnding[Bishop][count];

        tempMove = bishopAttacks(from, occupied ^ pos.getBitboard(Black, Queen)) & targetBitboard;
        attackUnits += attackWeight[Bishop] * popcnt(tempMove & opponentKingZone);
	}

	tempPiece = pos.getBitboard(Black, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = rookAttacks(from, occupied) & targetBitboard;
        attacks[Black] |= tempMove;

		count = popcnt(tempMove);
		scoreOp -= mobilityOpening[Rook][count];
		scoreEd -= mobilityEnding[Rook][count];

        tempMove = rookAttacks(from, occupied ^ pos.getBitboard(Black, Queen) ^ pos.getBitboard(Black, Rook)) & targetBitboard;
        attackUnits += attackWeight[Rook] * popcnt(tempMove & opponentKingZone);

		if (!(files[File(from)] & pos.getBitboard(Black, Pawn)))
		{
			if (!(files[File(from)] & pos.getBitboard(White, Pawn)))
			{
				scoreOp -= rookOnOpenFileBonus;
			}
			else
			{
				scoreOp -= rookOnSemiOpenFileBonus;
			}
		}
	}

	tempPiece = pos.getBitboard(Black, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);

		tempMove = queenAttacks(from, occupied) & targetBitboard;
        attacks[Black] |= tempMove;

		count = popcnt(tempMove);
		scoreOp -= mobilityOpening[Queen][count];
		scoreEd -= mobilityEnding[Queen][count];

        attackUnits += attackWeight[Queen] * popcnt(tempMove & opponentKingZone);
	}

    kingSafetyScore[Black] = attackUnits;

    // Detect likely zugzwangs statically.
    // Idea shamelessly stolen from Ivanhoe.
    zugzwangLikely = !attacks[pos.getSideToMove()];
    // zugzwangLikely = !(attacks[pos.getSideToMove()] & ~attacks[!pos.getSideToMove()]);

	return ((scoreOp * (64 - phase)) + (scoreEd * phase)) / 64;
}

int pawnStructureEval(Position & pos, int phase)
{
    auto scoreOp = 0, scoreEd = 0;
    int score;

    if (pttProbe(pos, scoreOp, scoreEd))
    {
        return ((scoreOp * (64 - phase)) + (scoreEd * phase)) / 64;
    }

    for (int c = White; c <= Black; ++c)
    {
        auto scoreOpForColor = 0, scoreEdForColor = 0;
        auto ownPawns = pos.getBitboard(!!c, Pawn);
        auto tempPawns = ownPawns;
        auto opponentPawns = pos.getBitboard(!c, Pawn);

        while (tempPawns)
        {
            auto from = bitScanForward(tempPawns);
            tempPawns &= (tempPawns - 1);
            auto pawnFile = File(from);
            auto pawnRank = (c ? 7 - Rank(from) : Rank(from));

            auto passedPawn = !(opponentPawns & passed[c][from]);
            auto doubledPawn = (ownPawns & (c ? rays[1][from] : rays[6][from])) != 0;
            auto isolatedPawn = !(ownPawns & isolated[from]);
            // 1. The pawn must be able to move forward.
            // 2. The stop square must be controlled by an enemy pawn.
            // 3. There musn't be any own pawns capable of defending the pawn. 
            // TODO: Check that this is correct.
            // TODO: test is the empty condition helping?
            auto backwardPawn = ((pawnAttacks[c][from + 8 - 16 * c] & opponentPawns)
                && (pos.getPiece(from + 8 - 16 * c) == Empty)
                && !(ownPawns & backward[c][from]));

            if (passedPawn)
            {
                scoreOpForColor += passedBonusOpening[pawnRank];
                scoreEdForColor += passedBonusEnding[pawnRank];
            }

            if (doubledPawn)
            {
                scoreOpForColor -= doubledPenaltyOpening[pawnFile];
                scoreEdForColor -= doubledPenaltyEnding[pawnFile];
            }

            if (isolatedPawn)
            {
                scoreOpForColor -= isolatedPenaltyOpening[pawnFile];
                scoreEdForColor -= isolatedPenaltyEnding[pawnFile];
            }

            if (backwardPawn)
            {
                scoreOpForColor -= backwardPenaltyOpening[pawnFile];
                scoreEdForColor -= backwardPenaltyEnding[pawnFile];
            }
        }

        scoreOp += (c == Black ? -scoreOpForColor : scoreOpForColor);
        scoreEd += (c == Black ? -scoreEdForColor : scoreEdForColor);
    }

    score = ((scoreOp * (64 - phase)) + (scoreEd * phase)) / 64;
    pttSave(pos, scoreOp, scoreEd);

    return score;
}

int evaluatePawnShelter(Position & pos, bool side)
{
    static const std::array<int, 8> pawnStormPenalty = { 0, 0, 0, 1, 2, 3, 0, 0 };
    static const auto openFilePenalty = 6;
    static const auto halfOpenFilePenalty = 4;
    auto penalty = 0;
    auto ownPawns = pos.getBitboard(side, Pawn);
    auto enemyPawns = pos.getBitboard(!side, Pawn);
    auto kingFile = File(bitScanForward(pos.getBitboard(side, King)));
    // If the king is at the edge assume that it is a bit closer to the center so that the pawn shelter is evaluated properly.
    kingFile = std::max(1, std::min(kingFile, 6));

    for (auto file = kingFile - 1; file <= kingFile + 1; ++file)
    {
        if (!(files[file] & (ownPawns | enemyPawns))) // Open file.
        {
            penalty += openFilePenalty;
        }
        else
        {
            if (!(files[file] & ownPawns)) // Half-open file (our)
            {
                penalty += halfOpenFilePenalty;
            }
        }
    }

    return penalty;
}

int kingSafetyEval(Position & pos, int phase, std::array<int, 2> & kingSafetyScore)
{
    kingSafetyScore[Black] += evaluatePawnShelter(pos, White);
    kingSafetyScore[White] += evaluatePawnShelter(pos, Black);
    auto score = kingSafetyTable[kingSafetyScore[White]] - kingSafetyTable[kingSafetyScore[Black]];
	return ((score * (64 - phase)) / 64);
}

int eval(Position & pos, bool & zugzwangLikely)
{
	// Checks if we are in a known endgame.
	// If we are we can straight away return the score for the endgame.
	// At the moment only detects draws, if wins will be included this must be made to return things in negamax fashion.
	if (knownEndgames.count(pos.getMaterialHash()))
	{
		return knownEndgames[pos.getMaterialHash()];
	}

    auto phase = pos.getPhase();
	// Material + Piece-Square Tables
	auto score = ((pos.getScoreOp() * (64 - phase)) + (pos.getScoreEd() * phase)) / 64;

	if (popcnt(pos.getBitboard(White, Bishop)) == 2)
	{
		score += ((bishopPairBonusOpening * (64 - phase)) + (bishopPairBonusEnding * phase)) / 64;
	}
	if (popcnt(pos.getBitboard(Black, Bishop)) == 2)
	{
		score -= ((bishopPairBonusOpening * (64 - phase)) + (bishopPairBonusEnding * phase)) / 64;
	}

    std::array<int, 2> kingSafetyScore;
	// Mobility
    score += mobilityEval(pos, phase, kingSafetyScore, zugzwangLikely);

	// Pawn structure
	score += pawnStructureEval(pos, phase);

	// King safety
    score += kingSafetyEval(pos, phase, kingSafetyScore);

    score += (pos.getSideToMove() ? -5 : 5);

    return (pos.getSideToMove() ? -score : score);
}