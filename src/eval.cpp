#include "eval.hpp"
#include "piece.hpp"
#include "color.hpp"
#include "square.hpp"

PawnHashTable Evaluation::pawnHashTable;

std::array<int, 64> Evaluation::pieceSquareTableOpening[12];
std::array<int, 64> Evaluation::pieceSquareTableEnding[12];

const std::array<int, 6> Evaluation::pieceValuesOpening = {
    88, 235, 263, 402, 892, 0
};

const std::array<int, 6> Evaluation::pieceValuesEnding = {
    112, 258, 286, 481, 892, 0
};

const std::array<int, 64> Evaluation::openingPST[6] = {
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

const std::array<int, 64> Evaluation::endingPST[6] = {
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

const std::vector<int> Evaluation::mobilityOpening[6] = {
    {},
    { -12, -8, -4, 0, 4, 8, 10, 12, 13 },
    { -12, -6, 0, 6, 12, 18, 22, 26, 28, 30, 31, 31, 32, 32 },
    { -6, -4, -2, 0, 2, 4, 6, 8, 10, 11, 11, 11, 11, 12, 12 },
    { -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 },
    {}
};

const std::vector<int> Evaluation::mobilityEnding[6] = {
    {},
    { -12, -8, -4, 0, 4, 8, 10, 12, 13 },
    { -12, -6, 0, 6, 12, 18, 22, 26, 28, 30, 31, 31, 32, 32 },
    { -12, -7, -3, 3, 7, 12, 16, 20, 24, 28, 30, 31, 31, 32, 32 },
    { -8, -6, -4, -2, 0, 2, 4, 6, 8, 9, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 },
    {}
};

const std::array<int, 8> Evaluation::passedBonusOpening = {
    0, 0, 5, 10, 20, 25, 50, 0
};

const std::array<int, 8> Evaluation::passedBonusEnding = {
    0, 0, 10, 15, 30, 40, 80, 0
};

const std::array<int, 8> Evaluation::doubledPenaltyOpening = {
    8, 12, 16, 16, 16, 16, 12, 8
};

const std::array<int, 8> Evaluation::doubledPenaltyEnding = {
    16, 20, 24, 24, 24, 24, 20, 16
};

const std::array<int, 8> Evaluation::isolatedPenaltyOpening = {
    8, 12, 16, 16, 16, 16, 12, 8
};

const std::array<int, 8> Evaluation::isolatedPenaltyEnding = {
    12, 16, 20, 20, 20, 20, 16, 12
};

const std::array<int, 8> Evaluation::backwardPenaltyOpening = {
    6, 10, 12, 12, 12, 12, 10, 6
};

const std::array<int, 8> Evaluation::backwardPenaltyEnding = {
    12, 14, 16, 16, 16, 16, 14, 12
};

const int Evaluation::bishopPairBonusOpening = 32;
const int Evaluation::bishopPairBonusEnding = 32;
const int Evaluation::sideToMoveBonus = 5;

const std::array<int, 6> Evaluation::attackWeight = {
    0, 2, 2, 3, 5, 0
};

const std::array<int, 100> Evaluation::kingSafetyTable = {
    0, 0, 1, 2, 3, 5, 7, 9, 12, 15,
    18, 22, 26, 30, 35, 39, 44, 50, 56, 62,
    68, 75, 82, 85, 89, 97, 105, 113, 122, 131,
    140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
    260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
    377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
    494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

std::unordered_map<HashKey, int> Evaluation::knownEndgames;

void Evaluation::initializeKnownEndgames()
{
    knownEndgames.clear();

    // King vs king: draw
    auto matHash = Zobrist::materialHash[Piece::WhiteKing][0] ^ Zobrist::materialHash[Piece::BlackKing][0];
    knownEndgames[matHash] = 0;

    // King and a minor piece vs king: draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            knownEndgames[matHash ^ Zobrist::materialHash[j + i * 6][0]] = 0;
        }
    }

    // King and two knights vs king: draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        knownEndgames[matHash ^ Zobrist::materialHash[Piece::Knight + i * 6][0] ^ 
                      Zobrist::materialHash[Piece::Knight + i * 6][1]] = 0;
    }

    // King and a minor piece vs king and a minor piece: draw
    for (Piece i = Piece::Knight; i <= Piece::Bishop; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            knownEndgames[matHash ^ Zobrist::materialHash[Color::White + i][0] ^ 
                          Zobrist::materialHash[Color::Black * 6 + j][0]] = 0;
        }
    }

    // King and two bishops vs king and a bishop: draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        knownEndgames[matHash ^ Zobrist::materialHash[Piece::Bishop + i * 6][0] ^ 
                      Zobrist::materialHash[Piece::Bishop + i * 6][1] ^ 
                      Zobrist::materialHash[Piece::Bishop + !i * 6][0]] = 0;
    }

    // King and either two knights or a knight and a bishop vs king and a minor piece: draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            for (Piece k = Piece::Knight; k <= Piece::Bishop; ++k)
            {
                knownEndgames[matHash ^ Zobrist::materialHash[Piece::Knight + i * 6][0] ^ 
                              Zobrist::materialHash[j + i * 6][j == Piece::Knight] ^ 
                              Zobrist::materialHash[k + !i * 6][0]] = 0;
            }
        }
    }
}

void Evaluation::initialize()
{ 
    initializeKnownEndgames();

    for (Piece p = Piece::Pawn; p <= Piece::King; ++p)
    {
        for (Square sq = Square::A1; sq <= Square::H8; ++sq)
        {
            pieceSquareTableOpening[p][sq] = openingPST[p][sq] + pieceValuesOpening[p];
            pieceSquareTableEnding[p][sq] = endingPST[p][sq] + pieceValuesEnding[p];

            pieceSquareTableOpening[p + Color::Black * 6][sq ^ 56] = -(openingPST[p][sq] + pieceValuesOpening[p]);
            pieceSquareTableEnding[p + Color::Black * 6][sq ^ 56] = -(endingPST[p][sq] + pieceValuesEnding[p]);
        }
    }
}

int Evaluation::evaluate(const Position & pos)
{
    return (Bitboards::isHardwarePopcntSupported() ? evaluate<true>(pos) : evaluate<false>(pos));
}

template <bool hardwarePopcnt> 
int Evaluation::evaluate(const Position & pos)
{
    // Checks if we are in a known endgame.
    // If we are we can straight away return the score for the endgame.
    // At the moment only detects draws, if wins will be included this must be made to return things in negamax fashion.
    if (knownEndgames.count(pos.getMaterialHashKey()))
    {
        return knownEndgames[pos.getMaterialHashKey()];
    }

    auto phase = pos.calculateGamePhase();
    auto scoreOp = 0, scoreEd = 0;
    auto kingSafetyScore = 0;

    auto score = mobilityEval<hardwarePopcnt>(pos, kingSafetyScore, phase);
    score += pawnStructureEval(pos, phase);

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (pos.getBoard(sq) != Piece::Empty)
        {
            scoreOp += pieceSquareTableOpening[pos.getBoard(sq)][sq];
            scoreEd += pieceSquareTableEnding[pos.getBoard(sq)][sq];
        }
    }

    // Bishop pair bonus.
    for (Color c = Color::White; c <= Color::Black; ++c)
    {
        if (pos.getPieceCount(c, Piece::Bishop) == 2)
        {
            auto bishopPairBonus = interpolateScore(bishopPairBonusOpening, bishopPairBonusEnding, phase);
            score += (c ? -bishopPairBonus : bishopPairBonus);
        }
    }

    score += interpolateScore(scoreOp, scoreEd, phase);
    score += (pos.getSideToMove() ? -sideToMoveBonus : sideToMoveBonus);

    return (pos.getSideToMove() ? -score : score);
}

template <bool hardwarePopcnt> 
int Evaluation::mobilityEval(const Position & pos, int & kingSafetyScore, int phase)
{
    auto scoreOp = 0, scoreEd = 0;
    auto occupied = pos.getOccupiedSquares();

    for (Color c = Color::White; c <= Color::Black; ++c)
    {
        auto targetBitboard = ~pos.getPieces(c);
        auto scoreOpForColor = 0, scoreEdForColor = 0;
        auto opponentKingZone = Bitboards::kingZone[!c][Bitboards::lsb(pos.getBitboard(!c, Piece::King))];
        auto attackUnits = 0;

        auto tempPiece = pos.getBitboard(c, Piece::Knight);
        while (tempPiece)
        {
            auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::knightAttacks[from] & targetBitboard;
            auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Knight][count];
            scoreEdForColor += mobilityEnding[Piece::Knight][count];

            tempMove &= opponentKingZone;
            attackUnits += attackWeight[Piece::Knight] * Bitboards::popcnt<hardwarePopcnt>(tempMove);
        }

        tempPiece = pos.getBitboard(c, Piece::Bishop);
        while (tempPiece)
        {
            auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::bishopAttacks(from, occupied) & targetBitboard;
            auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Bishop][count];
            scoreEdForColor += mobilityEnding[Piece::Bishop][count];

            tempMove &= opponentKingZone;
            attackUnits += attackWeight[Piece::Bishop] * Bitboards::popcnt<hardwarePopcnt>(tempMove);
        }

        tempPiece = pos.getBitboard(c, Piece::Rook);
        while (tempPiece)
        {
            auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::rookAttacks(from, occupied) & targetBitboard;
            auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Rook][count];
            scoreEdForColor += mobilityEnding[Piece::Rook][count];

            tempMove &= opponentKingZone;
            attackUnits += attackWeight[Piece::Rook] * Bitboards::popcnt<hardwarePopcnt>(tempMove);
        }

        tempPiece = pos.getBitboard(c, Piece::Queen);
        while (tempPiece)
        {
            auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::queenAttacks(from, occupied) & targetBitboard;
            auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Queen][count];
            scoreEdForColor += mobilityEnding[Piece::Queen][count];

            tempMove &= opponentKingZone;
            attackUnits += attackWeight[Piece::Queen] * Bitboards::popcnt<hardwarePopcnt>(tempMove);
        }
        
        kingSafetyScore += (c ? -kingSafetyTable[attackUnits] : kingSafetyTable[attackUnits]);
        scoreOp += (c ? -scoreOpForColor : scoreOpForColor);
        scoreEd += (c ? -scoreEdForColor : scoreEdForColor);
    }

    return interpolateScore(scoreOp, scoreEd, phase);
}


int Evaluation::pawnStructureEval(const Position & pos, int phase)
{
    auto scoreOp = 0, scoreEd = 0;

    if (pawnHashTable.probe(pos, scoreOp, scoreEd))
    {
        return interpolateScore(scoreOp, scoreEd, phase);
    } 

    for (Color c = Color::White; c <= Color::Black; ++c)
    {
        auto scoreOpForColor = 0, scoreEdForColor = 0;
        auto ownPawns = pos.getBitboard(c, Piece::Pawn);
        auto tempPawns = ownPawns;
        auto opponentPawns = pos.getBitboard(!c, Piece::Pawn);

        while (tempPawns)
        {
            auto from = Bitboards::popLsb(tempPawns);
            auto pawnFile = file(from);
            auto pawnRank = (c ? 7 - rank(from) : rank(from)); // rank is relative to side to move

            auto passed = !(opponentPawns & Bitboards::passed[c][from]);
            auto doubled = (ownPawns & (c ? Bitboards::rays[1][from] : Bitboards::rays[6][from])) != 0;
            auto isolated = !(ownPawns & Bitboards::isolated[from]);
            // 1. The pawn must be able to move forward.
            // 2. The stop square must be controlled by an enemy pawn.
            // 3. There musn't be any own pawns capable of defending the pawn. 
            // TODO: Check that this is correct.
            auto backward = ((pos.getBoard(from + 8 - 16 * c) == Piece::Empty) 
                          && (Bitboards::pawnAttacks[c][from + 8 - 16 * c] & opponentPawns)
                          && !(ownPawns & Bitboards::backward[c][from]));

            if (passed)
            {
                scoreOpForColor += passedBonusOpening[pawnRank];
                scoreEdForColor += passedBonusEnding[pawnRank];
            }

            if (doubled)
            {
                scoreOpForColor -= doubledPenaltyOpening[pawnFile];
                scoreEdForColor -= doubledPenaltyEnding[pawnFile];
            }

            if (isolated)
            {
                scoreOpForColor -= isolatedPenaltyOpening[pawnFile];
                scoreEdForColor -= isolatedPenaltyEnding[pawnFile];
            }

            if (backward)
            {
                scoreOpForColor -= backwardPenaltyOpening[pawnFile];
                scoreEdForColor -= backwardPenaltyEnding[pawnFile];
            }
        }

        scoreOp += (c == Color::Black ? -scoreOpForColor : scoreOpForColor);
        scoreEd += (c == Color::Black ? -scoreEdForColor : scoreEdForColor);
    }

    pawnHashTable.save(pos, scoreOp, scoreEd);

    return interpolateScore(scoreOp, scoreEd, phase);
}

