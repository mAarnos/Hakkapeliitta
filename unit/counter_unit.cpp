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

#include "..\src\counter.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(CounterTest)
{
    CounterMoveTable counterMoveTable;
    const Position pos("2r3k1/q4ppp/p3p3/pnNp4/2rP4/2P2P2/4R1PP/2R1Q1K1 b - - 0 1");
    const Move m1(Square::H8, Square::G8, Piece::Empty); // A fictional just-made move.
    const Move m2(Square::F3, Square::F4, Piece::Empty); // An even more fictional counter move to the previous move.

    counterMoveTable.update(pos, m2, m1);
    auto counter = counterMoveTable.getCounterMove(pos, m1);

    BOOST_CHECK(counter == m2);

    counterMoveTable.clear();
    counter = counterMoveTable.getCounterMove(pos, m1);

    BOOST_CHECK(counter.empty());
}

