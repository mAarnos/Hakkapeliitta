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

#include "..\src\move.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(RandomNormalMove)
{
    const Move m(Square::G1, Square::F3, Piece::Empty);

    BOOST_CHECK(m.getFrom() == Square::G1);
    BOOST_CHECK(m.getTo() == Square::F3);
    BOOST_CHECK(m.getFlags() == Piece::Empty);
    BOOST_CHECK(m.getRawMove() == 50502);
    BOOST_CHECK(!m.empty());
}

BOOST_AUTO_TEST_CASE(EmptyMove)
{
    const Move m;

    BOOST_CHECK(m.empty());
}

BOOST_AUTO_TEST_CASE(Comparision)
{
    const Move m(Square::G1, Square::F3, Piece::Empty);
    const Move m2(50502);
    const Move m3;

    BOOST_CHECK(m == m2);
    BOOST_CHECK(m != m3);
}

