#include "color.hpp"

Color::Color():
color(NoColor)
{
}

Color::Color(int newColor) :
color(newColor)
{
}

bool isColorOk(Color c)
{
	return (c == Color::Black || c == Color::White);
}

