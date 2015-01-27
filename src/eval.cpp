#include "eval.hpp"
#include "piece.hpp"
#include "color.hpp"
#include "square.hpp"
#include "utils/clamp.hpp"

std::array<std::array<short, 64>, 12> Evaluation::pieceSquareTableOpening;
std::array<std::array<short, 64>, 12> Evaluation::pieceSquareTableEnding;

const std::array<short, 6> pieceValuesOpening = {
    75, 228, 242, 301, 713, 0
};

const std::array<short, 6> pieceValuesEnding = {
    131, 269, 281, 530, 933, 0
};

const std::array<std::array<short, 64>, 6> openingPST = {{
    {
        0, 0, 0, 0, 0, 0, 0, 0, -34, -25, -26, -35, -11, -4, -7, -36, -33, -22, -26, -16, -8, -4, -16, -25, -30, -15, -12, -5, 1, -8, -23, -20, -17, -6, -7, 10, 20, 11, 0, -9, 12, -5, 26, 33, 53, 78, 55, 26, 51, 49, 97, 107, 89, 39, -33, -47, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        -101, -31, -41, -29, -17, -19, -19, -49, -33, -33, -15, 0, -4, -1, -15, -21, -29, -11, 4, 26, 26, 8, 7, -11, -17, 8, 16, 18, 26, 24, 26, -5, -1, 8, 24, 58, 30, 42, 28, 33, -7, 27, 58, 76, 102, 106, 79, 35, -3, -1, 25, 70, 54, 87, -5, 47, -153, -101, -3, 53, -7, -41, -65, -63
    },
    {
        -9, 6, -9, -16, -14, -5, -14, 3, 2, -1, 10, -11, -1, 2, 11, -18, -11, 8, 1, 6, 0, -3, 0, 3, -2, -5, 6, 11, 11, -4, -9, 6, -12, 2, 6, 39, 9, 30, 0, 2, -3, 14, 11, 24, 42, 65, 62, 17, -12, -7, -4, -29, -3, 4, -21, 4, -41, -134, -29, -80, -44, -46, -2, -69
    },
    {
        -18, -14, -6, 1, 1, 0, 4, -20, -32, -26, -8, -15, -15, -20, 6, -28, -40, -28, -18, -11, -9, -12, -2, -6, -42, -28, -30, -9, -21, -18, 10, -22, -24, -12, -12, 15, 3, 12, 30, 32, -8, 16, 4, 17, 59, 82, 104, 52, -4, 0, 20, 51, 45, 78, 44, 88, 30, 46, 30, 45, 19, 86, 88, 114
    },
    {
        8, -5, 8, 7, 15, -8, 1, 20, -16, 1, 4, 11, 7, 12, 19, 18, -3, -4, 7, -6, 8, -1, 4, 9, -10, -13, -10, -15, -17, -6, 1, -6, -6, -7, -12, -19, -23, -30, -37, -18, -7, -18, -17, -24, -20, 21, 14, -3, -26, -35, -10, -27, -45, -2, -25, 54, -53, -10, -23, 8, -16, 39, 78, 61
    },
    {
        18, 39, 9, -63, -11, -37, 17, 11, 46, 22, 7, -28, -22, -21, 12, -6, -28, 24, 2, -47, -35, -50, -36, -76, -67, 11, 9, -21, -41, -47, -87, -183, -67, 32, 11, -73, -17, -7, 0, -149, -137, 109, 93, -3, 33, 105, 139, -29, 77, -51, 87, 93, 25, 69, 67, 39, 67, 27, -65, -55, -93, 55, 113, 109
    }
}};

const std::array<std::array<short, 64>, 6> endingPST = {{
    {
        0, 0, 0, 0, 0, 0, 0, 0, -11, -1, -13, -9, 5, -7, -13, -19, -18, -4, -20, -22, -14, -20, -12, -24, -11, 1, -23, -33, -23, -15, -11, -21, 10, 4, -8, -28, -18, -14, 4, -4, 29, 39, 11, -3, -13, -7, 21, 11, 45, 61, 55, 21, 35, 19, 27, 47, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        -31, -43, -21, -15, -15, -33, -43, -25, -23, -11, -7, -8, 4, -3, -21, -25, -33, -1, 10, 32, 28, 4, -3, -9, -13, 24, 44, 40, 48, 36, 24, -3, 15, 18, 48, 52, 50, 50, 36, 15, 3, 19, 34, 32, 32, 34, 25, 5, -17, 15, 17, 28, 24, -11, 23, -3, -3, 31, 29, 1, 19, 5, 19, -3
    },
    {
        -16, -5, -16, -5, -11, -2, -7, -36, -9, -20, -7, -4, -2, -15, -4, -13, -6, 11, 10, 11, 13, 2, 3, -6, -3, 12, 11, 12, 16, 3, 0, -21, 9, 12, 11, 16, 20, 5, 2, 13, 14, 11, 10, -3, 3, 8, 11, 24, 1, 14, 5, 16, -2, 1, 10, -15, 42, 37, 34, 29, 31, 18, 11, 20
    },
    {
        -9, -11, -9, -13, -19, -17, -25, -19, -9, -17, -11, -11, -19, -23, -39, -13, 1, 1, -9, -13, -15, -13, -15, -17, 23, 11, 13, 1, -1, 9, -3, 7, 27, 23, 25, 11, 3, 5, -1, 1, 19, 17, 23, 15, -7, -11, -15, -7, 23, 31, 29, 23, 21, -7, 9, -7, 9, 7, 13, 11, 29, -1, -9, -21
    },
    {
        -50, -31, -42, -15, -41, -60, -107, -42, -15, -30, -19, -20, -22, -43, -60, -93, -22, -17, -8, -5, -9, 4, -3, -18, -19, 8, 9, 44, 30, 9, -6, 1, -7, 12, 41, 64, 68, 57, 54, 31, 4, 29, 42, 63, 79, 54, 55, 50, 19, 36, 37, 60, 80, 45, 56, 7, 30, 23, 22, 5, 37, 28, 1, -18
    },
    {
        -59, -23, -19, -17, -39, -23, -39, -81, -27, -11, -5, 4, 0, -5, -19, -33, 1, -1, 14, 28, 26, 12, -1, -15, 13, 28, 34, 42, 38, 34, 28, 21, 31, 44, 50, 50, 38, 38, 32, 39, 43, 53, 52, 40, 26, 40, 43, 23, -19, 81, 41, 24, 32, 47, 65, -9, -35, 35, 59, 65, 67, 55, 59, -45
    }
}};

const std::array<std::vector<int>, 6> mobilityOpening = {{
    {},
    { -7, -9, -1, -1, -1, -3, -3, -3, 0 },
    { -21, -15, -11, -9, -5, 1, 5, 7, 5, 7, 8, 16, 25, 49 },
    { -19, -15, -9, -9, -9, -5, -5, -1, -3, -4, -2, -2, 0, 5, 17 },
    { -7, -14, -13, -10, -9, -8, -7, -6, -3, -2, -1, 0, 0, 0, 0, 0, -2, -4, -2, -2, 6, 14, 26, 52, 58, 106, 54, 166 },
    {}
}};

const std::array<std::vector<int>, 6> mobilityEnding = {{
    {},
    { -59, -1, 7, 19, 21, 27, 25, 23, 16 },
    { -25, -39, -21, -5, 9, 21, 31, 37, 45, 43, 40, 36, 33, 1 },
    { -25, -22, -14, -8, 6, 13, 15, 19, 29, 37, 41, 44, 44, 43, 29 },
    { -55, -49, -37, -67, -45, -49, -31, -23, -19, -14, -5, 2, 16, 20, 24, 30, 38, 48, 48, 46, 40, 34, 30, 0, -10, -50, -12, -90 },
    {}
}};

const std::array<int, 8> passedBonusOpening = {
    0, 3, -20, -11, 15, 40, 55, 0
};

const std::array<int, 8> passedBonusEnding = {
    0, 5, 19, 36, 51, 75, 95, 0
};

const std::array<int, 8> doubledPenaltyOpening = {
    29, 3, 9, 21, 21, 17, -3, 13
};

const std::array<int, 8> doubledPenaltyEnding = {
    51, 35, 27, 17, 19, 21, 29, 49
};

const std::array<int, 8> isolatedPenaltyOpening = {
    -3, 5, 13, 15, 23, 15, 15, 23
};

const std::array<int, 8> isolatedPenaltyEnding = {
    3, 15, 19, 27, 25, 15, 9, 5
};

const std::array<int, 8> backwardPenaltyOpening = {
    -1, 3, -1, 19, 5, 3, 5, -7
};

const std::array<int, 8> backwardPenaltyEnding = {
    1, 3, 5, -7, 1, -3, 5, 1
};

const int bishopPairBonusOpening = 25;
const int bishopPairBonusEnding = 75;
const int sideToMoveBonus = 6;

const std::array<int, 6> attackWeight = {
    0, 2, 2, 3, 5, 0
};

const std::array<int, 8> openFilePenalty = {
    6, 6, 6, 6, 6, 6, 6, 6
};

const std::array<int, 8> halfOpenFilePenalty = {
    4, 4, 4, 4, 4, 4, 4, 4
};

const std::array<int, 100> kingSafetyTable = {
    13, 0, 8, -13, 4, 2, 8, 2, 15, 8, 11, 15, 15, 19, 24, 20, 29, 35, 51, 39, 43, 58, 57, 70, 72, 74, 78, 112, 103, 100, 119, 109, 130, 149, 144, 151, 178, 180, 202, 199, 221, 223, 252, 258, 268, 260, 287, 283, 289, 291, 338, 326, 366, 377, 373, 335, 319, 404, 354, 334, 501, 377, 501, 365, 501, 501, 427, 501, 421, 501, 321, 501, 321, 501, 321, 501, 500, 501, 335, 500, 500, 500, 501, 501, 500, 501, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

Evaluation::Evaluation(PawnHashTable& pawnHashTable) : pawnHashTable(pawnHashTable)
{
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

int Evaluation::evaluate(const Position& pos, bool& zugzwangLikely)
{
    return (Bitboards::isHardwarePopcntSupported() ? evaluate<true>(pos, zugzwangLikely) : evaluate<false>(pos, zugzwangLikely));
}

int interpolateScore(const int scoreOp, const int scoreEd, const int phase)
{
    return ((scoreOp * (64 - phase)) + (scoreEd * phase)) / 64;
}

template <bool hardwarePopcnt> 
int Evaluation::evaluate(const Position& pos, bool& zugzwangLikely)
{
    if (endgameModule.drawnEndgame(pos.getMaterialHashKey()))
    {
        return 0;
    }

    std::array<int, 2> kingSafetyScore;
    const auto phase = clamp(static_cast<int>(pos.getGamePhase()), 0, 64); // The phase can be negative in some weird cases, guard against that.

    auto score = mobilityEval<hardwarePopcnt>(pos, kingSafetyScore, phase, zugzwangLikely);
    score += pawnStructureEval(pos, phase);
    score += kingSafetyEval(pos, phase, kingSafetyScore);
    // score += interpolateScore(pos.getPstMaterialScoreOpening(), pos.getPstMaterialScoreEnding(), phase);

    // Bishop pair bonus.
    for (Color c = Color::White; c <= Color::Black; ++c)
    {
        if (pos.getPieceCount(c, Piece::Bishop) == 2)
        {
            const auto bishopPairBonus = interpolateScore(bishopPairBonusOpening, bishopPairBonusEnding, phase);
            score += (c ? -bishopPairBonus : bishopPairBonus);
        }
    }

    score += (pos.getSideToMove() ? -sideToMoveBonus : sideToMoveBonus);

    return (pos.getSideToMove() ? -score : score);
}

template <bool hardwarePopcnt> 
int Evaluation::mobilityEval(const Position& pos, std::array<int, 2>& kingSafetyScore, const int phase, bool& zugzwangLikely)
{
    const auto occupied = pos.getOccupiedSquares();
    auto scoreOp = 0, scoreEd = 0;
    std::array<uint64_t, 2> attacks = { 0, 0 };

    for (Color c = Color::White; c <= Color::Black; ++c)
    {
        const auto targetBitboard = ~pos.getPieces(c);
        const auto opponentKingZone = Bitboards::kingSafetyZone(!c, Bitboards::lsb(pos.getBitboard(!c, Piece::King)));
        auto scoreOpForColor = 0, scoreEdForColor = 0;
        auto attackUnits = 0;

        auto tempPiece = pos.getBitboard(c, Piece::Knight);
        while (tempPiece)
        {
            const auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::knightAttacks(from) & targetBitboard;
            attacks[c] |= tempMove;
            const auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Knight][count];
            scoreEdForColor += mobilityEnding[Piece::Knight][count];
            tempMove &= opponentKingZone;
            attackUnits += attackWeight[Piece::Knight] * Bitboards::popcnt<hardwarePopcnt>(tempMove);
        }

        tempPiece = pos.getBitboard(c, Piece::Bishop);
        while (tempPiece)
        {
            const auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::bishopAttacks(from, occupied) & targetBitboard;
            attacks[c] |= tempMove;
            const auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Bishop][count];
            scoreEdForColor += mobilityEnding[Piece::Bishop][count];
			tempMove = Bitboards::bishopAttacks(from, occupied ^ pos.getBitboard(c, Piece::Queen)) & targetBitboard;
            tempMove &= opponentKingZone;
            attackUnits += attackWeight[Piece::Bishop] * Bitboards::popcnt<hardwarePopcnt>(tempMove);
        }

        tempPiece = pos.getBitboard(c, Piece::Rook);
        while (tempPiece)
        {
            const auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::rookAttacks(from, occupied) & targetBitboard;
            attacks[c] |= tempMove;
            const auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Rook][count];
            scoreEdForColor += mobilityEnding[Piece::Rook][count];
            tempMove = Bitboards::rookAttacks(from, occupied ^ pos.getBitboard(c, Piece::Queen) ^ pos.getBitboard(c, Piece::Rook)) & targetBitboard;
            tempMove &= opponentKingZone;
            attackUnits += attackWeight[Piece::Rook] * Bitboards::popcnt<hardwarePopcnt>(tempMove);

            if (!(Bitboards::files[file(from)] & pos.getBitboard(c, Piece::Pawn)))
            {
                if (!(Bitboards::files[file(from)] & pos.getBitboard(!c, Piece::Pawn)))
                {
                    scoreOpForColor += 27;
                }
                else
                {
                    scoreOpForColor += 14;
                }
            }
        }

        tempPiece = pos.getBitboard(c, Piece::Queen);
        while (tempPiece)
        {
            const auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::queenAttacks(from, occupied) & targetBitboard;
            attacks[c] |= tempMove;
            const auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Queen][count];
            scoreEdForColor += mobilityEnding[Piece::Queen][count];
            tempMove &= opponentKingZone;
            attackUnits += attackWeight[Piece::Queen] * Bitboards::popcnt<hardwarePopcnt>(tempMove);
        }
        
        kingSafetyScore[c] = attackUnits;
        scoreOp += (c ? -scoreOpForColor : scoreOpForColor);
        scoreEd += (c ? -scoreEdForColor : scoreEdForColor);
    }

    // Detect likely zugzwangs statically.
    // Idea shamelessly stolen from Ivanhoe.
    zugzwangLikely = !attacks[pos.getSideToMove()];
    // zugzwangLikely = !(attacks[pos.getSideToMove()] & ~attacks[!pos.getSideToMove()]);

    return interpolateScore(scoreOp, scoreEd, phase);
}

int Evaluation::pawnStructureEval(const Position& pos, const int phase)
{
    auto scoreOp = 0, scoreEd = 0;

    if (pawnHashTable.probe(pos.getHashKey(), scoreOp, scoreEd))
    {
        return interpolateScore(scoreOp, scoreEd, phase);
    }

    for (Color c = Color::White; c <= Color::Black; ++c)
    {
        const auto ownPawns = pos.getBitboard(c, Piece::Pawn);
        const auto opponentPawns = pos.getBitboard(!c, Piece::Pawn);
        auto tempPawns = ownPawns;
        auto scoreOpForColor = 0, scoreEdForColor = 0;

        while (tempPawns)
        {
            const auto from = Bitboards::popLsb(tempPawns);
            const auto pawnFile = file(from);
            const auto pawnRank = (c ? 7 - rank(from) : rank(from)); // rank is relative to side to move

            const auto passed = !(opponentPawns & Bitboards::passedPawn(c, from));
            const auto doubled = (ownPawns & (c ? Bitboards::ray(1, from) : Bitboards::ray(6, from))) != 0;
            const auto isolated = !(ownPawns & Bitboards::isolatedPawn(from));
            // 1. The pawn must be able to move forward.
            // 2. The stop square must be controlled by an enemy pawn.
            // 3. There musn't be any own pawns capable of defending the pawn. 
            // TODO: Check that this is correct.
            const auto backward = ((pos.getBoard(from + 8 - 16 * c) == Piece::Empty) 
                          && (Bitboards::pawnAttacks(c, from + 8 - 16 * c) & opponentPawns)
                          && !(ownPawns & Bitboards::backwardPawn(c, from)));

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

    pawnHashTable.save(pos.getHashKey(), scoreOp, scoreEd);

    return interpolateScore(scoreOp, scoreEd, phase);
}

int evaluatePawnShelter(const Position& pos, const Color side)
{
    auto penalty = 0;
    const auto ownPawns = pos.getBitboard(side, Piece::Pawn);
    const auto enemyPawns = pos.getBitboard(!side, Piece::Pawn);
    // If the king is at the edge assume that it is a bit closer to the center.
    // This prevents all bugs related to the next loop and going off the board.
    const auto kingFile = clamp(file(Bitboards::lsb(pos.getBitboard(side, Piece::King))), 1, 6);

    for (auto f = kingFile - 1; f <= kingFile + 1; ++f)
    {
        if (!(Bitboards::files[f] & (ownPawns | enemyPawns))) // Open file.
        {
            penalty += openFilePenalty[f];
        }
        else
        {
            if (!(Bitboards::files[f] & ownPawns)) // Half-open file (our)
            {
                penalty += halfOpenFilePenalty[f];
            }
        }
    }

    return penalty;
}

int Evaluation::kingSafetyEval(const Position& pos, const int phase, std::array<int, 2>& kingSafetyScore)
{
    kingSafetyScore[Color::Black] += evaluatePawnShelter(pos, Color::White);
    kingSafetyScore[Color::White] += evaluatePawnShelter(pos, Color::Black);
    kingSafetyScore[Color::White] = std::min(kingSafetyScore[Color::White], 99);
    kingSafetyScore[Color::Black] = std::min(kingSafetyScore[Color::Black], 99);

    const auto score = kingSafetyTable[kingSafetyScore[Color::White]] - kingSafetyTable[kingSafetyScore[Color::Black]];
    return ((score * (64 - phase)) / 64);
}
