#include "square.hpp"
#include <cassert>

Square::Square():
square(NoSquare)
{
}

Square::Square(int newSquare) :
square(newSquare)
{
}

bool isSquareOk(Square sq)
{
	return ((sq >= Square::A1) && (sq <= Square::H8));
}

int file(Square sq)
{
    assert(isSquareOk(sq));
    return (sq % 8);
}

int rank(Square sq)
{
    assert(isSquareOk(sq));
    return (sq / 8);
}

