#include "tuning.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include "stopwatch.hpp"
#include "eval.hpp"

std::vector<int> Tuning::evalTerms = {
    88, 235, 263, 402, 892, 0, // 0-5: materialOpening
    112, 258, 286, 481, 892, 0, // 6-11: materialEnding
    0, 0, 0, 0, 0, 0, 0, 0, // 12-75: pawnPSTOpening
    -33, -18, -13, -18, -18, -13, -18, -33,
    -28, -23, -13, -3, -3, -13, -23, -28,
    -33, -18, -13, 12, 12, -13, -18, -33,
    -18, -13, -8, 17, 17, -8, -13, -18,
    7, 12, 17, 22, 22, 17, 12, 7,
    42, 42, 42, 42, 42, 42, 42, 42,
    0, 0, 0, 0, 0, 0, 0, 0
    - 36, -26, -16, -6, -6, -16, -26, -36, // 76-139: knightPSTOpening
    -26, -16, -6, 9, 9, -6, -16, -26,
    -16, -6, 9, 29, 29, 9, -6, -16,
    -6, 9, 29, 39, 39, 29, 9, -6,
    -6, 9, 39, 49, 49, 39, 9, -6,
    -16, -6, 19, 59, 59, 19, -6, -16,
    -26, -16, -6, 9, 9, -6, -16, -26,
    -36, -26, -16, -6, -6, -16, -26, -36,
    -24, -19, -14, -9, -9, -14, -19, -24, // 140-203: bishopPSTOpening
    -9, 6, 1, 6, 6, 1, 6, -9,
    -4, 1, 6, 11, 11, 6, 1, -4,
    1, 6, 11, 16, 16, 11, 6, 1,
    1, 11, 11, 16, 16, 11, 11, 1,
    -4, 1, 6, 11, 11, 6, 1, -4,
    -9, -4, 1, 6, 6, 1, -4, -9,
    -14, -9, -4, 1, 1, -5, -9, -14,
    -3, -3, -1, 2, 2, -1, -3, -3, // 204-267: rookPSTOpening
    -3, -3, -1, 2, 2, -1, -3, -3,
    -3, -3, -1, 2, 2, -1, -3, -3,
    -3, -3, -1, 2, 2, -1, -3, -3,
    -3, -3, -1, 2, 2, -1, -3, -3,
    -3, -3, -1, 2, 2, -1, -3, -3,
    7, 7, 9, 12, 12, 9, 7, 7,
    -3, -3, -1, 2, 2, -1, -3, -3,
    -19, -14, -9, -4, -4, -9, -14, -19, // 268-331: queenPSTOpening
    -9, -4, 1, 6, 6, 1, -4, -9,
    -4, 1, 6, 11, 11, 6, 1, -4,
    1, 6, 11, 16, 16, 11, 6, 1,
    1, 6, 11, 16, 16, 11, 6, 1,
    -4, 1, 6, 11, 11, 6, 1, -4,
    -9, -4, 1, 6, 6, 1, -4, -9,
    -14, -9, -4, 1, 1, -4, -9, -14,
    5, 10, 2, 0, 0, 6, 10, 4, // 332-395: kingPSTOpening
    5, 5, 0, -5, -5, 0, 5, 5,
    -5, -5, -5, -10, -10, -5, -5, -5,
    -10, -10, -20, -30, -30, -20, -10, -10,
    -20, -25, -30, -40, -40, -30, -25, -20,
    -40, -40, -50, -60, -60, -50, -40, -40,
    -50, -50, -60, -60, -60, -60, -50, -50,
    -60, -60, -60, -60, -60, -60, -60, -60,
    0, 0, 0, 0, 0, 0, 0, 0, // 396-459: pawnPSTEnding
    -22, -22, -22, -22, -22, -22, -22, -22,
    -17, -17, -17, -17, -17, -17, -17, -17,
    -12, -12, -12, -12, -12, -12, -12, -12,
    -7, -7, -7, -7, -7, -7, -7, -7,
    18, 18, 18, 18, 18, 18, 18, 18,
    38, 38, 38, 38, 38, 38, 38, 38,
    0, 0, 0, 0, 0, 0, 0, 0, // 460-523: knightPSTEnding
    -34, -24, -14, -4, -4, -14, -24, -34,
    -24, -14, -4, 11, 11, -4, -14, -24,
    -14, -4, 11, 31, 31, 11, -4, -14,
    -4, 11, 31, 41, 41, 31, 11, -4,
    -4, 11, 31, 41, 41, 31, 11, -4,
    -14, -4, 11, 31, 31, 11, -4, -14,
    -24, -14, -4, 11, 11, -4, -14, -24,
    -34, -24, -14, -4, -4, -14, -24, -34,
    -15, -10, -5, 0, 0, -5, -10, -15, // 524-587: bishopPSTEnding
    -10, -5, 0, 5, 5, 0, -5, -10,
    -5, 0, 5, 10, 10, 5, 0, -5,
    0, 5, 10, 15, 15, 10, 5, 0,
    0, 5, 10, 15, 15, 10, 5, 0,
    -5, 0, 5, 10, 10, 5, 0, -5,
    -10, -5, 0, 5, 5, 0, -5, -10,
    -15, -10, -5, 0, 0, -5, -10, -15,
    0, 0, 0, 0, 0, 0, 0, 0, // 588-651: rookPSTEnding
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -15, -10, -5, 0, 0, -5, -10, -15, // 652-715: queenPSTEnding
    -10, -5, 0, 5, 5, 0, -5, -10,
    -5, 0, 5, 10, 10, 5, 0, -5,
    0, 5, 10, 15, 15, 10, 5, 0,
    0, 5, 10, 15, 15, 10, 5, 0,
    -5, 0, 5, 10, 10, 5, 0, -5,
    -10, -5, 0, 5, 5, 0, -5, -10,
    -15, -10, -5, 0, 0, -5, -10, -15,
    -38, -28, -18, -8, -8, -18, -28, -38, // 716-779: kingPSTEnding
    -28, -18, -8, 13, 13, -8, -18, -28,
    -18, -8, 13, 43, 43, 13, -8, -18,
    -8, 13, 43, 53, 53, 43, 13, -8,
    -8, 13, 43, 53, 53, 43, 13, -8,
    -18, -8, 13, 43, 43, 13, -8, -18,
    -28, -18, -8, 13, 13, -8, -18, -28,
    -38, -28, -18, -8, -8, -18, -28, -38,
    5, // 780: sideToMoveBonus
};

Tuning::Tuning():
scalingConstant(254.48)
{
    std::ifstream whiteWins("C:\\whiteWins.txt");
    std::ifstream blackWins("C:\\blackWins.txt");
    std::ifstream draws("C:\\draws.txt");
    Position pos;
	std::string text;

	while (std::getline(whiteWins, text))
	{
        pos.initializeBoardFromFEN(text);
		positions.push_back(pos);
		results.push_back(1.0);
	}

	while (std::getline(blackWins, text))
	{
        pos.initializeBoardFromFEN(text);
        positions.push_back(pos);
		results.push_back(0.0);
	}

    while (std::getline(draws, text))
	{
        pos.initializeBoardFromFEN(text);
        positions.push_back(pos);
		results.push_back(0.5);
	}
}

double Tuning::sigmoid(double x) const
{
	return (1.0 / (1.0 + exp(x / -scalingConstant)));
}

double Tuning::evalError() const
{
    auto sum = 0.0;

    for (size_t i = 0; i < positions.size(); ++i)
    {
        auto v = Tuning::evaluate(positions[i]);
        sum += pow((results[i] - sigmoid(v)), 2); // least squares
    }

    return (sum / static_cast<double>(positions.size()));
}

void Tuning::calculateScalingConstant()
{
    auto best = evalError();
    auto step = 0.01;
    auto improved = true;
    auto direction = 1;

    while (improved)
    {
        improved = false;
        scalingConstant += step * direction;
        auto error = evalError();
        if (error >= best)
        {
            scalingConstant -= 2 * step * direction;
            error = evalError();
            if (error >= best)
            {
                scalingConstant += step * direction;
            }
            else
            {
                improved = true;
                best = error;
                direction *= -1; // Change the direction.
                std::cout << scalingConstant << std::endl;
            }
        }
        else
        {
            improved = true;
            best = error;
            std::cout << scalingConstant << std::endl;
        }
    }

    std::cout << "Scaling constant optimized, result = " << scalingConstant << std::endl;
}

void Tuning::tune()
{
    calculateScalingConstant();
}

int Tuning::evaluate(const Position & pos)
{
    auto phase = pos.calculateGamePhase();
    auto scoreOp = 0, scoreEd = 0;
    
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (pos.getBoard(sq) != Piece::Empty)
        {
            auto pieceType = pos.getBoard(sq) % 6;
            if (pos.getBoard(sq) > 5) // if the piece is black
            {
                scoreOp -= evalTerms[pieceType];
                scoreEd -= evalTerms[pieceType];
            }
            else
            {
                scoreOp += evalTerms[pieceType];
                scoreEd += evalTerms[pieceType];
            }
        }
    }
    
    auto score = ((scoreOp * (256 - phase)) + (scoreEd * phase)) / 256;

    // Side to move bonus.
    score += (pos.getSideToMove() ? -evalTerms[779] : evalTerms[779]);
}

