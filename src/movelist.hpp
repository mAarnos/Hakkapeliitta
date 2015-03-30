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

#ifndef MOVELIST_HPP_
#define MOVELIST_HPP_

#include <cassert>
#include "move.hpp"

// Holds generated moves. Maximum amount of moves supported is 218.
// The slightly peculiar design is caused by the desire to avoid calling constructors on 218 Move objects.
class MoveList
{
public:
    MoveList();

    Move& operator[](int index);
    const Move& operator[](int index) const;

    template<class... T>
    void emplace_back(T&&... args);
    void clear();
    void resize(const int newSize);
    int size() const;
    bool empty() const;
private:
    int32_t moveList[sizeof(Move[218]) / sizeof(int32_t)];
    int32_t numberOfMoves;
};

inline MoveList::MoveList() : numberOfMoves(0) 
{
};

inline Move& MoveList::operator[](const int index)
{
    return (reinterpret_cast<Move*>(&moveList[0]))[index];
}

inline const Move& MoveList::operator[](const int index) const
{
    return (reinterpret_cast<const Move*>(&moveList[0]))[index];
}

template<class... T>
inline void MoveList::emplace_back(T&&... args)
{
    (reinterpret_cast<Move*>(&moveList[0]))[numberOfMoves++] = Move(std::forward<T>(args)...);
}

inline void MoveList::clear() 
{ 
    numberOfMoves = 0; 
};

inline void MoveList::resize(const int newSize)
{ 
    numberOfMoves = newSize;
}

inline int MoveList::size() const
{ 
    return numberOfMoves; 
}

inline bool MoveList::empty() const 
{ 
    return numberOfMoves == 0;
}

#endif
