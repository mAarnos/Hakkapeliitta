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

#include "evaluation.hpp"
#include "piece.hpp"
#include "color.hpp"
#include "square.hpp"
#include "utils/clamp.hpp"

std::array<std::array<short, 64>, 12> Evaluation::mPieceSquareTableOpening;
std::array<std::array<short, 64>, 12> Evaluation::mPieceSquareTableEnding;

std::array<std::array<int, 64>, 64> Evaluation::mChebyshevDistance;

const std::array<short, 6> pieceValuesOpening = {
    79, 248, 253, 355, 847, 0
};

const std::array<short, 6> pieceValuesEnding = {
    127, 275, 292, 526, 939, 0
};

const std::array<std::array<short, 64>, 6> openingPST = {{
    {
        0, 0, 0, 0, 0, 0, 0, 0, -35, -28, -32, -38, -17, -3, -4, -39, -35, -23, -31, -25, -11, -1, -10, -26, -31, -17, -16, -7, 2, 1, -18, -21, -22, -6, -9, 6, 20, 14, -1, -15, 4, -7, 27, 27, 52, 60, 51, 15, 55, 44, 89, 100, 87, 64, -11, -24, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        -76, -28, -41, -37, -24, -19, -16, -48, -34, -34, -21, -5, -9, 0, -12, -16, -25, -14, 0, 24, 21, 6, 9, -14, -20, 6, 10, 13, 27, 25, 26, -8, 3, 3, 26, 47, 25, 40, 26, 30, 4, 21, 60, 77, 100, 87, 48, 20, 9, -5, 37, 71, 52, 61, 3, 26, -105, -37, 16, 40, -4, -20, -35, -63
    },
    {
        -9, 7, -11, -15, -10, -4, -10, -10, 3, -6, 10, -12, -3, 0, 11, -13, -16, 10, -3, 4, -2, -2, 4, -1, -4, -7, 3, 3, 15, -7, -7, 11, -12, -2, 10, 37, 7, 31, 0, -5, -2, 14, 16, 16, 35, 63, 60, 11, -13, -4, -2, -12, -12, 20, -27, 2, -38, -78, -29, -34, -63, -24, -7, -25
    },
    {
        -14, -9, -3, 3, 4, 3, 6, -17, -29, -22, -8, -9, -13, -21, -1, -38, -44, -23, -14, -5, 0, -16, -4, -9, -38, -20, -35, -6, -13, -17, 18, -29, -21, -7, -13, 14, 5, 7, 37, 24, -8, 17, 5, 19, 56, 56, 66, 29, -14, 7, 31, 49, 47, 72, 27, 69, 23, 19, 26, 13, 19, 43, 59, 66
    },
    {
        7, -2, 10, 8, 15, -4, -15, 1, -7, 11, 6, 14, 12, 20, 19, 11, 0, 1, 13, -6, 11, 7, 9, 10, -9, -9, -11, -10, -6, 4, 6, -7, -3, 0, 4, -12, -6, -31, -33, -10, 3, -19, -5, -1, 7, 34, 13, 1, -6, -30, 10, -3, -16, -4, 3, 52, -22, 14, -6, -10, -6, 28, 46, 32
    },
    {
        17, 45, 6, -54, -12, -35, 17, 9, 56, 22, 0, -20, -22, -10, 17, 1, -15, 41, 12, -44, -16, -29, -24, -48, -34, 7, 6, -16, -13, -29, -38, -79, -28, -4, -4, -43, -13, -3, 7, -45, -55, 29, 12, -43, -35, 15, 28, -10, -3, 16, 4, -20, -17, 4, 19, -20, -11, -6, -18, -11, -34, 7, 9, -47
    }
}};

const std::array<std::array<short, 64>, 6> endingPST = {{
    {
        0, 0, 0, 0, 0, 0, 0, 0, -12, -3, -10, 0, 4, -9, -13, -20, -19, -8, -17, -16, -16, -19, -14, -22, -11, -3, -21, -29, -24, -19, -10, -20, 9, 0, -6, -26, -21, -15, 1, -5, 42, 44, 11, 7, -7, 10, 31, 20, 56, 71, 62, 14, 52, 17, 16, 42, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        -38, -42, -15, -8, -12, -26, -46, -29, -15, -2, 1, -6, 8, 1, -16, -15, -27, 3, 12, 27, 30, 2, -8, 4, -6, 22, 45, 39, 45, 30, 21, -2, 21, 25, 45, 53, 50, 46, 26, 16, -6, 24, 34, 31, 29, 40, 44, 9, -20, 22, 10, 32, 32, 8, 27, 1, -43, 14, 12, 5, 0, 6, 7, -23
    },
    {
        -18, -10, -10, -3, -10, 2, -17, -40, -10, -16, -7, -2, 0, -12, -5, -13, -1, 15, 10, 13, 14, 1, 0, -1, 4, 14, 14, 15, 16, 9, -4, -18, 9, 14, 9, 19, 22, 6, -1, 6, 14, 17, 6, 1, 14, 5, 15, 23, 3, 13, -1, 18, -1, 0, 9, -17, 21, 27, 35, 5, 34, 3, 16, -9
    },
    {
        -10, -12, -7, -8, -17, -13, -21, -15, -1, -14, -3, -10, -15, -16, -25, 1, 9, 0, -4, -10, -14, -6, -8, -7, 26, 10, 20, 4, 6, 14, 5, 18, 33, 28, 28, 19, 8, 18, 6, 1, 30, 25, 26, 24, 14, 17, 13, 14, 28, 32, 28, 30, 33, 9, 21, 1, 17, 15, 16, 24, 32, 20, 12, 10
    },
    {
        -24, -25, -23, -2, -26, -47, -50, -28, -8, -25, -12, -3, -13, -41, -44, -56, -20, -12, -8, -1, 0, 4, 0, -19, -11, 2, 11, 39, 20, 2, -12, -5, -3, 15, 26, 51, 47, 46, 27, -11, 2, 38, 33, 42, 52, 42, 34, 28, 12, 35, 25, 53, 51, 29, 15, 36, 17, 18, 10, 5, 24, 34, 36, 13
    },
    {
        -67, -22, -20, -28, -36, -26, -37, -75, -32, -10, -2, 1, 3, -5, -18, -33, -5, -3, 15, 24, 22, 8, -1, -26, 7, 26, 36, 43, 37, 34, 15, -11, 11, 47, 51, 49, 48, 48, 32, 17, 28, 61, 76, 66, 48, 76, 61, 25, -17, 51, 56, 58, 66, 61, 51, 4, 10, 31, 51, 51, 58, 51, 41, -49
    }
}};

const std::array<std::vector<int>, 6> mobilityOpening = {{
    {},
    { -1, 6, 12, 16, 17, 16, 16, 15, 18 },
    { -12, -7, -2, 1, 6, 14, 17, 22, 19, 23, 21, 34, 34, 29 },
    { -15, -12, -5, -3, -4, -1, 0, 6, 3, 1, 4, 4, 7, 14, 20 },
    { 35, -5, -8, -8, -6, -4, -6, -3, 1, -1, 5, 7, 6, 6, 8, 4, 2, 6, 4, 7, 17, 21, 30, 24, 41, 41, 36, 34 },
    {}
}};

const std::array<std::vector<int>, 6> mobilityEnding = {{
    {},
    { -30, 1, 4, 11, 16, 25, 23, 23, 19 },
    { -22, -32, -19, -6, 5, 20, 29, 33, 42, 40, 36, 38, 28, 13 },
    { -12, -12, -8, -5, 7, 16, 20, 22, 31, 42, 43, 47, 47, 47, 37 },
    { -2, 5, -2, -23, -28, -29, -14, -12, -10, -3, -5, -1, 6, 14, 17, 22, 36, 34, 43, 37, 39, 33, 36, 22, 12, -4, 38, 26 },
    {}
}};

const std::array<int, 8> passedBonusOpening = {
    0, 4, -19, -8, 19, 48, 58, 0
};

const std::array<int, 8> passedBonusEnding = {
    0, 4, 17, 32, 51, 67, 90, 0
};

const std::array<int, 8> doubledPenaltyOpening = {
    36, 9, 2, 23, 18, 20, 0, 26
};

const std::array<int, 8> doubledPenaltyEnding = {
    46, 25, 31, 24, 21, 19, 29, 44
};

const std::array<int, 8> isolatedPenaltyOpening = {
    1, 5, 14, 13, 22, 14, 14, 20
};

const std::array<int, 8> isolatedPenaltyEnding = {
    5, 13, 21, 26, 22, 16, 10, 6
};

const std::array<int, 8> backwardPenaltyOpening = {
    -4, 3, 2, 21, 8, 7, 13, -1
};

const std::array<int, 8> backwardPenaltyEnding = {
    2, 7, 13, 14, 7, 1, 1, 3
};

const int bishopPairBonusOpening = 42;
const int bishopPairBonusEnding = 52;
const int sideToMoveBonus = 1;

const std::array<int, 6> attackWeight = {
    0, 2, 2, 3, 5, 0
};

const std::array<int, 100> kingSafetyTable = {
    21, 7, 11, 7, 7, 9, 5, 7, 10, 14, 15, 20, 19, 20, 25, 22, 28, 40, 45, 47, 46, 60, 56, 82, 86, 102, 98, 109, 107, 117, 125, 132, 159, 168, 181, 188, 211, 213, 234, 216, 265, 276, 288, 272, 308, 339, 351, 355, 374, 354, 370, 412, 420, 481, 439, 457, 478, 478, 441, 509, 494, 431, 517, 569, 562, 499, 500, 531, 523, 500, 500, 500, 522, 517, 500, 500, 500, 508, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

void Evaluation::staticInitialize()
{
    for (Piece p = Piece::Pawn; p <= Piece::King; ++p)
    {
        for (Square sq = Square::A1; sq <= Square::H8; ++sq)
        {
            mPieceSquareTableOpening[p][sq] = openingPST[p][sq] + pieceValuesOpening[p];
            mPieceSquareTableEnding[p][sq] = endingPST[p][sq] + pieceValuesEnding[p];

            mPieceSquareTableOpening[p + Color::Black * 6][sq ^ 56] = -(openingPST[p][sq] + pieceValuesOpening[p]);
            mPieceSquareTableEnding[p + Color::Black * 6][sq ^ 56] = -(endingPST[p][sq] + pieceValuesEnding[p]);
        }
    }

    for (Square sq1 = Square::A1; sq1 <= Square::H8; ++sq1)
    {
        for (Square sq2 = Square::A1; sq2 <= Square::H8; ++sq2)
        {
            const auto file1 = file(sq1);
            const auto file2 = file(sq2);
            const auto rank1 = rank(sq1);
            const auto rank2 = rank(sq2);
            const auto rankDistance = std::abs(rank2 - rank1);
            const auto fileDistance = std::abs(file2 - file1);
            mChebyshevDistance[sq1][sq2] = std::max(rankDistance, fileDistance);
        }
    }
}

int Evaluation::evaluate(const Position& pos)
{
    return (Bitboards::hardwarePopcntSupported() ? evaluate<true>(pos) : evaluate<false>(pos));
}

int interpolateScore(int scoreOp, int scoreEd, int phase)
{
    return ((scoreOp * (64 - phase)) + (scoreEd * phase)) / 64;
}

template <bool hardwarePopcnt> 
int Evaluation::evaluate(const Position& pos)
{
    if (mEndgameModule.drawnEndgame(pos.getMaterialHashKey()))
    {
        return 0;
    }

    std::array<int, 2> kingSafetyScore;
    const auto phase = clamp(static_cast<int>(pos.getGamePhase()), 0, 64); // The phase can be negative in some weird cases, guard against that.

    auto score = mobilityEval<hardwarePopcnt>(pos, kingSafetyScore, phase);
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
int Evaluation::mobilityEval(const Position& pos, std::array<int, 2>& kingSafetyScore, int phase)
{
    const auto occupied = pos.getOccupiedSquares();
    auto scoreOp = 0, scoreEd = 0;

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
            const auto tempMove = Bitboards::knightAttacks(from) & targetBitboard;
            const auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Knight][count];
            scoreEdForColor += mobilityEnding[Piece::Knight][count];
            attackUnits += attackWeight[Piece::Knight] * Bitboards::popcnt<hardwarePopcnt>(tempMove & opponentKingZone);
        }

        tempPiece = pos.getBitboard(c, Piece::Bishop);
        while (tempPiece)
        {
            const auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::bishopAttacks(from, occupied) & targetBitboard;
            const auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Bishop][count];
            scoreEdForColor += mobilityEnding[Piece::Bishop][count];
            tempMove = Bitboards::bishopAttacks(from, occupied ^ pos.getBitboard(c, Piece::Queen)) & targetBitboard;
            attackUnits += attackWeight[Piece::Bishop] * Bitboards::popcnt<hardwarePopcnt>(tempMove & opponentKingZone);
        }

        tempPiece = pos.getBitboard(c, Piece::Rook);
        while (tempPiece)
        {
            const auto from = Bitboards::popLsb(tempPiece);
            auto tempMove = Bitboards::rookAttacks(from, occupied) & targetBitboard;
            const auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Rook][count];
            scoreEdForColor += mobilityEnding[Piece::Rook][count];
            tempMove = Bitboards::rookAttacks(from, occupied ^ pos.getBitboard(c, Piece::Queen) ^ pos.getBitboard(c, Piece::Rook)) & targetBitboard;
            attackUnits += attackWeight[Piece::Rook] * Bitboards::popcnt<hardwarePopcnt>(tempMove & opponentKingZone);

            if (!(Bitboards::files[file(from)] & pos.getBitboard(c, Piece::Pawn)))
            {
                if (!(Bitboards::files[file(from)] & pos.getBitboard(!c, Piece::Pawn)))
                {
                    scoreOpForColor += 26;
                }
                else
                {
                    scoreOpForColor += 13;
                }
            }
        }

        tempPiece = pos.getBitboard(c, Piece::Queen);
        while (tempPiece)
        {
            const auto from = Bitboards::popLsb(tempPiece);
            const auto tempMove = Bitboards::queenAttacks(from, occupied) & targetBitboard;
            const auto count = Bitboards::popcnt<hardwarePopcnt>(tempMove);
            scoreOpForColor += mobilityOpening[Piece::Queen][count];
            scoreEdForColor += mobilityEnding[Piece::Queen][count];
            attackUnits += attackWeight[Piece::Queen] * Bitboards::popcnt<hardwarePopcnt>(tempMove & opponentKingZone);
        }
        
        kingSafetyScore[c] = attackUnits;
        scoreOp += (c ? -scoreOpForColor : scoreOpForColor);
        scoreEd += (c ? -scoreEdForColor : scoreEdForColor);
    }

    return interpolateScore(scoreOp, scoreEd, phase);
}

int Evaluation::pawnStructureEval(const Position& pos, int phase)
{
    const std::array<unsigned long, 2> kingLocations = { Bitboards::lsb(pos.getBitboard(Color::White, Piece::King)),
                                                         Bitboards::lsb(pos.getBitboard(Color::Black, Piece::King)) };
    // Add king locations to the pawn hash key so that we can cache knowledge which requires that kings stay on specific squares.
    const auto phk = pos.getPawnHashKey() ^ Zobrist::pieceHashKey(Piece::King, kingLocations[0])
                                          ^ Zobrist::pieceHashKey(Piece::King, kingLocations[1]);
    auto passers = 0ULL;
    auto scoreOp = 0, scoreEd = 0;

    if (mPawnHashTable.probe(phk, passers, scoreOp, scoreEd))
    {
        // No need to do the expensive calculations again.
    }
    else
    {
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
                // 1. There musn't be any own pawns capable of defending the pawn. 
                // 2. The pawn mustn't be blocked by a pawn.
                // 3. The stop-square of the pawn must be controlled by an enemy pawn.
                const auto backward = !(ownPawns & Bitboards::backwardPawn(c, from))
                    && pos.getBoard(from + 8 - 16 * c) != Piece::WhitePawn && pos.getBoard(from + 8 - 16 * c) != Piece::BlackPawn
                    && (Bitboards::pawnAttacks(c, from + 8 - 16 * c) & opponentPawns);

                if (passed)
                {
                    Bitboards::setBit(passers, from);
                    scoreOpForColor += passedBonusOpening[pawnRank];
                    scoreEdForColor += passedBonusEnding[pawnRank];
                    const auto distance1 = mChebyshevDistance[kingLocations[!c]][from + 8 - 16 * c];
                    const auto distance2 = mChebyshevDistance[kingLocations[c]][from + 8 - 16 * c];
                    scoreEdForColor += static_cast<int>(std::round((std::sqrt(distance1 + 1) - 1) * 30));
                    scoreEdForColor -= static_cast<int>(std::round((std::sqrt(distance2 + 1) - 1) * 28));
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

        mPawnHashTable.save(phk, passers, scoreOp, scoreEd);
    }

    return interpolateScore(scoreOp, scoreEd, phase);
}

int evaluatePawnShelter(const Position& pos, Color side)
{
    static const std::array<int, 8> openFilePenalty = { 6, 5, 4, 4, 4, 4, 5, 6 };
    static const std::array<int, 8> halfopenFilePenalty = { 5, 4, 3, 3, 3, 3, 4, 5 };
    static const std::array<int, 8> pawnStormPenalty = { 0, 0, 0, 1, 2, 3, 0, 0 };

    auto penalty = 0;
    const auto ownPawns = pos.getBitboard(side, Piece::Pawn);
    const auto enemyPawns = pos.getBitboard(!side, Piece::Pawn);
    // If the king is at the edge assume that it is a bit closer to the center.
    // This prevents all bugs related to the next loop and going off the board.
    const auto kingFile = clamp(file(Bitboards::lsb(pos.getBitboard(side, Piece::King))), 1, 6);

    for (auto f = kingFile - 1; f <= kingFile + 1; ++f)
    {
        const auto own = Bitboards::files[f] & ownPawns;
        const auto opponent = Bitboards::files[f] & enemyPawns;

        penalty += (own | opponent) ? 0 : openFilePenalty[f];
        penalty += (!own && opponent) ? halfopenFilePenalty[f] : 0;
        penalty += opponent ? pawnStormPenalty[side ? rank(Bitboards::msb(opponent)) : 7 - rank(Bitboards::lsb(opponent))] : 0;
    }

    return penalty;
}

int Evaluation::kingSafetyEval(const Position& pos, int phase, std::array<int, 2>& kingSafetyScore)
{
    kingSafetyScore[Color::Black] += evaluatePawnShelter(pos, Color::White);
    kingSafetyScore[Color::White] += evaluatePawnShelter(pos, Color::Black);
    kingSafetyScore[Color::White] = std::min(kingSafetyScore[Color::White], 99);
    kingSafetyScore[Color::Black] = std::min(kingSafetyScore[Color::Black], 99);

    const auto score = kingSafetyTable[kingSafetyScore[Color::White]] - kingSafetyTable[kingSafetyScore[Color::Black]];
    return ((score * (64 - phase)) / 64);
}
