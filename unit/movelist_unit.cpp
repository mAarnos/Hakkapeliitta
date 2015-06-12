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

#include "..\src\movelist.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(MoveListAddingMoves)
{
    MoveList moveList;
    const Move m1(Square::H4, Square::F5, Piece::Empty);
    const Move m2(Square::C6, Square::D4, Piece::Empty);
    const Move m3(Square::E8, Square::C8, Piece::King);

    moveList.emplace_back(m1);
    moveList.emplace_back(m2.getRawMove());
    moveList.setScore(1, -76);
    moveList.emplace_back(m3.getFrom(), m3.getTo(), m3.getFlags());
    moveList.setScore(2, 981);

    BOOST_CHECK(moveList.size() == 3);
    BOOST_CHECK(moveList.getMove(0) == m1);
    BOOST_CHECK(moveList.getScore(0) == 0);
    BOOST_CHECK(moveList.getMove(1) == m2);
    BOOST_CHECK(moveList.getScore(1) == -76);
    BOOST_CHECK(moveList.getMove(2) == m3);
    BOOST_CHECK(moveList.getScore(2) == 981);

    const Move m4(Square::A1, Square::H8, Piece::Empty);
    moveList.setMove(2, m4);

    BOOST_CHECK(moveList.getMove(2) == m4);
}

BOOST_AUTO_TEST_CASE(AssignmentOperator)
{
    MoveList moveList;
    MoveList moveList2;
    const Move m(Square::H4, Square::F5, Piece::Empty);

    moveList.emplace_back(m);

    BOOST_CHECK(moveList2.empty());

    moveList2 = moveList;

    BOOST_CHECK(moveList2.size() == 1);
    BOOST_CHECK(moveList2.getMove(0) == m);
}

BOOST_AUTO_TEST_CASE(MoveListSizeManipulationFunctions)
{
    MoveList moveList;

    BOOST_CHECK(moveList.empty());

    moveList.resize(7);
    BOOST_CHECK(moveList.size() == 7);

    moveList.clear();
    BOOST_CHECK(moveList.empty());
}

