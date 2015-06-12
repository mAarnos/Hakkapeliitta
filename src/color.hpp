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

/// @file color.hpp
/// @author Mikko Aarnos

#ifndef COLOR_HPP_
#define COLOR_HPP_

#include <cstdint>

/// @brief Represents a single color.
///
/// This color is encoded as a int8_t, of which only three values are legal (0, 1 and 2) and two well-defined (0, 1).
/// We don't use bool due to the fact that we often want to do something like "for (Color c = Color::White; c <= Color::Black; ++c)".
class Color
{
public:
    /// @brief Default constructor.
    Color() noexcept;

    /// @brief Constructs a Color from a given int8_t. 
    /// @param color The int8_t.
    Color(int8_t color) noexcept;

    enum : int8_t
    {
        White = 0, Black = 1, NoColor = 2
    };

    /// @return The int8_t.
    operator int8_t() const noexcept;
    /// @return A reference to the int8_t.
    operator int8_t&() noexcept;

    /// @brief Used for flipping the color from white to black and vice versa.
    /// @return The flipped color.
    Color operator!() const noexcept;

    /// @brief Debugging function, used for checking if the color is well-defined.
    /// @return True if the color is well-defined, false otherwise.
    bool isOk() const noexcept;

private:
    int8_t mColor;
}; 

inline Color::Color() noexcept : mColor(NoColor)
{
};

inline Color::Color(int8_t color) noexcept : mColor(color)
{
};

inline Color::operator int8_t() const noexcept
{ 
    return mColor;
}

inline Color::operator int8_t&() noexcept
{ 
    return mColor;
}

inline Color Color::operator!() const noexcept
{ 
    // ! has a branch normally, in our case we can eliminate it by just using a xor since we only have two well-defined values.
    // And yes, the difference in speed could actually be noticed.
    return mColor ^ 1; 
} 

inline bool Color::isOk() const noexcept
{
    return (mColor == Color::Black || mColor == Color::White);
}

#endif
