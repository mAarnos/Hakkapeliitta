/*
    Hakkapeliitta - A UCI chess engine. Copyright (C) 2013-2015 Mikko Aarnos.

    Hakkapeliitta is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Hakkapeliitta is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Hakkapeliitta. If not, see <http://www.gnu.org/licenses/>.
*/

#include "eval.hpp"
#include "piece.hpp"
#include "color.hpp"
#include "square.hpp"
#include "utils/clamp.hpp"

std::array<std::array<short, 64>, 12> Evaluation::pieceSquareTableOpening;
std::array<std::array<short, 64>, 12> Evaluation::pieceSquareTableEnding;

const std::array<short, 6> pieceValuesOpening = {
    74, 223, 217, 297, 711, 0
};

const std::array<short, 6> pieceValuesEnding = {
    137, 302, 325, 589, 1068, 0
};

const std::array<std::array<short, 64>, 6> openingPST = {{
    {
        0, 0, 0, 0, 0, 0, 0, 0, -40, -28, -35, -43, -17, -2, -1, -32, -39, -23, -32, -26, -9, 0, -7, -20, -35, -17, -18, -8, -1, -1, -16, -17, -22, -6, -9, 6, 21, 15, 4, -7, 10, -4, 26, 36, 56, 83, 52, 31, 32, 39, 99, 107, 90, 45, -49, -59, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        -112, -45, -54, -43, -32, -32, -34, -57, -44, -47, -30, -16, -17, -12, -26, -33, -43, -23, -11, 12, 13, -4, -3, -22, -31, -3, 4, 5, 12, 9, 12, -20, -12, -5, 13, 43, 18, 30, 13, 18, -17, 17, 46, 63, 88, 100, 63, 25, -13, -8, 13, 62, 38, 73, -15, 38, -166, -99, 7, 54, -13, -57, -89, -75
    },
    {
        -16, 4, -18, -22, -22, -8, -21, 2, 1, -10, 8, -19, -5, -3, 7, -22, -19, 6, -8, 0, -7, -5, -4, 0, -5, -10, 2, 3, 7, -12, -13, 0, -18, -2, 2, 35, 3, 23, -7, -1, -4, 10, 10, 21, 36, 63, 53, 11, -13, -10, -6, -25, -1, 0, -32, -5, -31, -117, -23, -66, -35, -27, 7, -65
    },
    {
        -24, -20, -15, -6, -4, -7, -1, -24, -39, -33, -16, -20, -21, -24, 1, -33, -46, -32, -25, -18, -14, -16, -7, -10, -47, -30, -36, -16, -27, -22, 5, -27, -26, -15, -18, 12, 1, 5, 20, 22, -13, 8, -5, 13, 48, 75, 93, 40, -10, -4, 14, 48, 36, 71, 25, 73, 24, 39, 21, 43, 14, 74, 79, 104
    },
    {
        10, -1, 10, 7, 17, -3, 2, 25, -16, 5, 6, 11, 11, 20, 25, 20, -3, -2, 6, -6, 9, 3, 8, 17, -8, -11, -10, -15, -13, -5, 4, -3, -1, -3, -11, -15, -17, -32, -40, -18, -1, -13, -9, -19, -15, 11, 6, -13, -19, -33, -10, -17, -40, -10, -31, 49, -48, -5, -25, 20, -18, 33, 59, 58
    },
    {
        32, 54, 10, -62, -13, -36, 17, 13, 55, 26, 10, -27, -20, -20, 13, -3, -24, 29, 7, -43, -33, -46, -34, -71, -71, 8, 14, -8, -38, -42, -86, -174, -47, 23, 14, -23, 9, -6, -6, -150, -117, 81, 66, -3, 8, 53, 77, -25, 42, -31, 37, 42, 1, 25, 35, 17, 59, 25, -64, -58, -64, 8, 71, -9
    }
}};

const std::array<std::array<short, 64>, 6> endingPST = {{
    {
        0, 0, 0, 0, 0, 0, 0, 0, -9, 0, -8, -3, 8, -8, -13, -25, -18, -5, -19, -19, -15, -20, -13, -27, -11, 1, -22, -30, -20, -18, -9, -24, 11, 4, -9, -26, -21, -15, 5, -7, 27, 37, 6, -8, -18, -6, 28, 9, 63, 71, 58, 20, 36, 26, 48, 57, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        -28, -40, -14, -8, -9, -27, -35, -32, -14, -4, 1, -2, 7, 1, -18, -17, -25, 3, 16, 36, 33, 3, -1, -6, -7, 29, 44, 44, 51, 40, 29, 1, 20, 23, 49, 55, 50, 51, 41, 20, 5, 21, 36, 34, 32, 32, 29, 4, -12, 17, 22, 30, 31, -7, 27, -3, -15, 28, 22, -1, 19, 18, 27, -8
    },
    {
        -12, -3, -8, 2, -3, 2, 2, -34, -9, -15, -4, 2, 0, -11, -6, -9, 0, 14, 16, 14, 19, 5, 7, -4, 1, 17, 15, 18, 17, 10, 3, -17, 14, 15, 14, 18, 25, 10, 6, 12, 15, 14, 11, 0, 9, 10, 14, 26, 3, 16, 9, 16, 0, 4, 12, -15, 28, 33, 33, 27, 27, 9, 4, 14
    },
    {
        -7, -9, -3, -9, -16, -13, -23, -16, -3, -13, -6, -7, -15, -20, -39, -9, 4, 2, -3, -9, -10, -10, -13, -15, 26, 11, 17, 7, 4, 11, 1, 9, 30, 25, 28, 15, 7, 11, 3, 4, 24, 22, 29, 19, 1, -4, -9, 6, 28, 34, 33, 26, 27, -3, 18, 1, 14, 12, 19, 11, 31, 4, -1, -15
    },
    {
        -40, -28, -37, -12, -31, -58, -92, -39, -2, -27, -19, -13, -17, -42, -56, -84, -13, -12, 0, 5, 0, 8, 1, -22, -14, 13, 17, 50, 32, 18, 0, 7, -3, 17, 48, 63, 65, 63, 60, 29, 2, 33, 42, 64, 75, 66, 59, 65, 21, 43, 48, 60, 83, 56, 68, 11, 44, 35, 35, 8, 44, 45, 28, 4
    },
    {
        -80, -45, -25, -22, -42, -28, -46, -85, -39, -19, -10, 2, -4, -8, -26, -40, -6, -5, 13, 26, 25, 10, -4, -19, 8, 25, 34, 43, 40, 33, 26, 16, 25, 47, 57, 54, 48, 46, 38, 38, 43, 64, 75, 61, 49, 64, 62, 24, -11, 83, 68, 42, 51, 66, 72, -5, -54, 27, 54, 67, 59, 66, 59, -50
    }
}};

const std::array<std::vector<int>, 6> mobilityOpening = {{
    {},
    { 0, -1, 6, 8, 9, 7, 7, 6, 7 },
    { -15, -10, -5, -3, 2, 8, 12, 14, 13, 14, 15, 25, 28, 49 },
    { -19, -15, -10, -7, -8, -5, -5, -1, -2, -2, -1, -1, 0, 4, 14 },
    { -2, -7, -5, -3, -2, -1, -2, -1, 2, 2, 3, 4, 2, 4, 3, 1, 2, -1, -1, 0, 8, 9, 25, 44, 43, 60, 24, 40 },
    {}
}};

const std::array<std::vector<int>, 6> mobilityEnding = {{
    {},
    { -50, 16, 26, 33, 38, 44, 44, 45, 40 },
    { -6, -23, -1, 14, 26, 39, 47, 52, 63, 60, 57, 53, 53, 31 },
    { 3, 4, 12, 11, 26, 31, 36, 40, 47, 54, 58, 61, 62, 60, 50 },
    { -19, -24, -18, -64, -41, -43, -21, -12, -5, 3, 8, 18, 29, 32, 38, 44, 51, 58, 60, 58, 53, 53, 45, 21, 18, -2, 26, 10 },
    {}
}};

const std::array<int, 8> passedBonusOpening = {
    0, 2, -18, -7, 18, 41, 63, 0
};

const std::array<int, 8> passedBonusEnding = {
    0, 2, 16, 31, 50, 80, 93, 0
};

const std::array<int, 8> doubledPenaltyOpening = {
    25, 8, 9, 17, 23, 23, 2, 23
};

const std::array<int, 8> doubledPenaltyEnding = {
    47, 29, 28, 18, 17, 20, 31, 50
};

const std::array<int, 8> isolatedPenaltyOpening = {
    -1, 6, 14, 16, 20, 13, 13, 19
};

const std::array<int, 8> isolatedPenaltyEnding = {
    5, 15, 20, 28, 26, 17, 11, 3
};

const std::array<int, 8> backwardPenaltyOpening = {
    -3, 4, 0, 17, 6, 5, 8, -6
};

const std::array<int, 8> backwardPenaltyEnding = {
    1, 2, 3, -4, -2, -2, 2, 2
};

const int bishopPairBonusOpening = 45;
const int bishopPairBonusEnding = 45;
const int sideToMoveBonus = 5;

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
    17, 0, 18, -4, 11, 10, 16, 14, 22, 16, 19, 25, 24, 30, 32, 28, 38, 44, 58, 48, 54, 64, 68, 81, 83, 83, 92, 122, 115, 112, 135, 130, 149, 160, 161, 161, 200, 203, 220, 217, 234, 246, 270, 281, 289, 283, 321, 313, 316, 318, 375, 366, 409, 422, 421, 437, 363, 483, 449, 466, 603, 521, 521, 479, 530, 580, 496, 538, 502, 572, 522, 501, 458, 520, 470, 513, 500, 531, 492, 500, 500, 500, 500, 516, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

Evaluation::Evaluation()
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
    score += interpolateScore(pos.getPstScoreOp(), pos.getPstScoreEd(), phase);

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
                    scoreOpForColor += 28;
                }
                else
                {
                    scoreOpForColor += 16;
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

    if (pawnHashTable.probe(pos.getPawnHashKey(), scoreOp, scoreEd))
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

    pawnHashTable.save(pos.getPawnHashKey(), scoreOp, scoreEd);

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
