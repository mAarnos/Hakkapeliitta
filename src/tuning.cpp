#include "tuning.hpp"
#include <iostream>
#include <fstream>
#include <string>

Tuning::Tuning():
scalingConstant(1.0)
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


