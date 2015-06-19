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

/// @file test.hpp
/// @author Mikko Aarnos

#ifndef TEST_HPP_
#define TEST_HPP_

#include <fstream>
#include <vector>

/// @brief Contains tests which take a long time to run even in release mode and as such cannot be included in the unit tests.
class Testing
{
public:
    /// @brief Default constructor.
    Testing(const std::string& fileName);

    /// @brief Used for checking that the eval is okay even with colors flipped.
    /// @return True if everything is okay, false otherwise.
    bool testReversedEval() const;

    /// @brief Used for checking that the function for checking if a move is pseudo legal is correct.
    /// @return True if everything is okay, false otherwise.
    bool testPseudoLegal() const;

private:
    std::vector<std::string> positions;
};

#endif
