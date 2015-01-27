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

#ifndef ENDGAME_HPP_
#define ENDGAME_HPP_

#include <unordered_set>
#include "zobrist.hpp"
#include "color.hpp"
#include "piece.hpp"

class EndgameModule 
{
public:
    EndgameModule();

    bool drawnEndgame(HashKey materialHashKey) const;
private:
    std::unordered_set<HashKey> fideDrawnEndgames;
    std::unordered_set<HashKey> otherDrawnEndgames;
};

inline EndgameModule::EndgameModule()
{
    // KK
    const auto matHash = Zobrist::materialHashKey(Piece::WhiteKing, 0) ^ Zobrist::materialHashKey(Piece::BlackKing, 0);
    fideDrawnEndgames.insert(matHash);

    // KBK, KNK, KKB, KKN
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            fideDrawnEndgames.insert(matHash ^ Zobrist::materialHashKey(j + i * 6, 0));
        }
    }

    // KNNK, KKNN
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        fideDrawnEndgames.insert(matHash ^ Zobrist::materialHashKey(Piece::Knight + i * 6, 0) ^
            Zobrist::materialHashKey(Piece::Knight + i * 6, 1));
    }

    // KBKB, KBKN, KNKB, KNKN
    for (Piece i = Piece::Knight; i <= Piece::Bishop; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            otherDrawnEndgames.insert(matHash ^ Zobrist::materialHashKey(Color::White + i, 0) ^
                Zobrist::materialHashKey(Color::Black * 6 + j, 0));
        }
    }

    // KBBKB, KBKBB
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        otherDrawnEndgames.insert(matHash ^ Zobrist::materialHashKey(Piece::Bishop + i * 6, 0) ^
            Zobrist::materialHashKey(Piece::Bishop + i * 6, 1) ^
            Zobrist::materialHashKey(Piece::Bishop + !i * 6, 0));
    }

    // KBNKB, KBNKN, KNNKB, KNNKN, KBKBN, KNKBN, KBKNN, KNKNN
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            for (Piece k = Piece::Knight; k <= Piece::Bishop; ++k)
            {
                otherDrawnEndgames.insert(matHash ^ Zobrist::materialHashKey(Piece::Knight + i * 6, 0) ^
                    Zobrist::materialHashKey(j + i * 6, j == Piece::Knight) ^
                    Zobrist::materialHashKey(k + !i * 6, 0));
            }
        }
    }
}

inline bool EndgameModule::drawnEndgame(const HashKey materialHashKey) const
{
    return fideDrawnEndgames.count(materialHashKey) > 0 || otherDrawnEndgames.count(materialHashKey) > 0;
}

#endif