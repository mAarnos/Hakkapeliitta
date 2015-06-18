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

/// @file clamp.hpp
/// @author Mikko Aarnos

#ifndef CLAMP_HPP_
#define CLAMP_HPP_

#include <algorithm>
#include <cassert>

/// @brief Forces the input value between the given lower and upper bounds.
/// @param value The value.
/// @param lowerBound The lower bound.
/// @param upperBound The upper bound.
/// @return If value is smaller than lower bound then lower bound, if greater than upper bound then upperbound, else value.
template <class T>
T clamp(T value, T lowerBound, T upperBound)
{
    return std::max(lowerBound, std::min(value, upperBound));
}

#endif
