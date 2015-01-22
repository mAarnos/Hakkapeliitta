#ifndef MOVELIST_HPP_
#define MOVELIST_HPP_

#include <cassert>
#include "move.hpp"

// Holds generated moves. Maximum amount of moves supported is 218.
// The slightly peculiar design is caused by the desire to avoid calling constructors on 218 Move objects.
class MoveList
{
public:
    MoveList() : numberOfMoves(0) {};

    Move& operator[](const int index);
    const Move& operator[](const int index) const;

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

inline Move& MoveList::operator[](const int index)
{
    assert(index >= 0 && index < numberOfMoves);
    return (reinterpret_cast<Move*>(&moveList[0]))[index];
}

inline const Move& MoveList::operator[](const int index) const
{
    assert(index >= 0 && index < numberOfMoves);
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
