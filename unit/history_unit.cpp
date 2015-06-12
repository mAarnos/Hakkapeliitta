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

#include "..\src\history.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(HistoryTest)
{
    HistoryTable historyTable;
    const Position pos("2r5/R1p2r1p/3pR1n1/2pN3k/2P2p1P/1P3P2/5KP1/8 b - - 0 43");
    const Move m1(Square::C8, Square::B8, Piece::Empty);
    const Move m2(Square::G6, Square::F8, Piece::Empty);
    const Move m3(Square::G6, Square::E5, Piece::Empty);

    historyTable.addCutoff(pos, m1, 7);
    historyTable.addCutoff(pos, m1, 8);
    historyTable.addCutoff(pos, m1, 9);

    historyTable.addCutoff(pos, m2, 8);
    historyTable.addNotCutoff(pos, m2, 10);

    historyTable.addCutoff(pos, m3, 7);
    historyTable.addNotCutoff(pos, m3, 9);
    historyTable.addNotCutoff(pos, m3, 11);

    const auto s1 = historyTable.getScore(pos, m1);
    const auto s2 = historyTable.getScore(pos, m2);
    const auto s3 = historyTable.getScore(pos, m3);

    BOOST_CHECK(s1 > s2);
    BOOST_CHECK(s2 > s3);

    historyTable.age();

    const auto s1_aged = historyTable.getScore(pos, m1);
    const auto s2_aged = historyTable.getScore(pos, m2);
    const auto s3_aged = historyTable.getScore(pos, m3);

    BOOST_CHECK(s1_aged > s2_aged);
    BOOST_CHECK(s2_aged > s3_aged);
}


