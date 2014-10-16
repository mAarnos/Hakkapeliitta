#ifndef TUNING_HPP_
#define TUNING_HPP_

#include <vector>
#include "..\position.hpp"

class Tuning
{
public:
    Tuning();

    void tune();
private:
    void calculateScalingConstant();

    double sigmoid(double x) const;
    double evalError() const;

    double scalingConstant;
    std::vector<Position> positions;
    std::vector<double> results;
    std::vector<int> evalTerms;

    int evaluate(const Position & pos) const;

    void prunePositions();
};

#endif