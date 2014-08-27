#ifndef TUNING_HPP_
#define TUNING_HPP_

#include <vector>
#include "position.hpp"

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

    static std::vector<int> evalTerms;
    static int evaluate(const Position & pos);
};

#endif