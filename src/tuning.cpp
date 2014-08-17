#include "tuning.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "stopwatch.hpp"
#include "eval.hpp"

Tuning::Tuning():
scalingConstant(0.68)
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
	return (1.0 / (1.0 + pow(10.0, -scalingConstant * x / 400.0)));
}

double Tuning::evalError() const
{
    Evaluation::initialize();
    auto sum = 0.0;

    for (auto i = 0; i < positions.size(); ++i)
    {
        auto v = Evaluation::evaluate(positions[i]);
        if (positions[i].getSideToMove() == Color::Black) v = -v; // correct the negamax evaluation back
        sum += pow((results[i] - sigmoid(v)), 2); // least squares
    }

    return (sum / static_cast<double>(positions.size()));
}

void Tuning::calculateScalingConstant()
{
    auto best = evalError();
    auto step = 0.01;
    auto improved = true;

    while (improved)
    {
        improved = false;
        scalingConstant += step;
        auto error = evalError();
        if (error >= best)
        {
            scalingConstant -= 2 * step;
            error = evalError();
            if (error >= best)
            {
                scalingConstant += step;
            }
            else
            {
                improved = true;
                best = error;
            }
        }
        else
        {
            improved = true;
            best = error;
        }
    }

    std::cout << "Scaling constant optimized, result = " << scalingConstant << std::endl;
}

void Tuning::tune()
{
    calculateScalingConstant();
}

