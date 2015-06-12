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

#include "..\src\endgame.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(EndgameModuleTest)
{
    const EndgameModule endgameModule;
    const auto kk = Zobrist::materialHashKey(Piece::WhiteKing, 0) ^ Zobrist::materialHashKey(Piece::BlackKing, 0);
    const auto knk = Zobrist::materialHashKey(Piece::WhiteKing, 0) ^ Zobrist::materialHashKey(Piece::WhiteKnight, 0) ^ Zobrist::materialHashKey(Piece::BlackKing, 0);
    const auto kbnk = Zobrist::materialHashKey(Piece::WhiteKing, 0) ^ Zobrist::materialHashKey(Piece::WhiteKnight, 0) ^ Zobrist::materialHashKey(Piece::WhiteBishop, 0) ^ Zobrist::materialHashKey(Piece::BlackKing, 0);

    BOOST_CHECK(endgameModule.drawnEndgame(kk));
    BOOST_CHECK(endgameModule.drawnEndgame(knk));
    BOOST_CHECK(!endgameModule.drawnEndgame(kbnk));
}

