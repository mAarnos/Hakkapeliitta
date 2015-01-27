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

#ifndef COLOR_HPP_
#define COLOR_HPP_

#include <cstdint>

// Represents a single color.
class Color
{
public:
    Color() : color(NoColor) {};
    Color(const int8_t newColor) : color(newColor) {};

    enum : int8_t
    {
        White = 0, Black = 1, NoColor = 2
    };

    operator int8_t() const { return color; }
    operator int8_t&() { return color; }
    Color operator!() const { return color ^ 1; } // ! has a branch normally, in our case we can eliminate it by just using a xor.
private:
    int8_t color;
}; 

// Checks if the color is okay, i.e. black or white. 
inline bool colorIsOk(const Color c)
{
    return (c == Color::Black || c == Color::White);
}

#endif