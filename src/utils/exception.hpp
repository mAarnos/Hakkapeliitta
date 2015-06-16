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

/// @file exception.hpp
/// @author Mikko Aarnos

#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <stdexcept>

/// @brief An exception which signals that either the allocated time is up or we have been ordered to stop.
class StopSearchException : public std::runtime_error
{
public:
    /// @brief Default constructor.
    /// @param description A brief (or not so brief) description of the cause for the exception.
    /// 
    /// The description doesn't really matter for this exception, as the name signales the intent.
    StopSearchException(const std::string& description) : std::runtime_error(description) 
    {
    }
};

#endif
