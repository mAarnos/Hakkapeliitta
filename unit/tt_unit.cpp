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

#include "..\src\tt.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(AllCasesTT)
{
    TranspositionTable tt;
    Move m(Square::H4, Square::F5, Piece::Empty);

    tt.save(5770153743293125963, m, -23, 7, TranspositionTable::Flags::ExactScore);

    auto ttEntry = tt.probe(5770153743293125963);
    BOOST_CHECK(ttEntry);
    BOOST_CHECK(ttEntry->getBestMove() == m);
    BOOST_CHECK(ttEntry->getScore() == -23);
    BOOST_CHECK(ttEntry->getDepth() == 7);
    BOOST_CHECK(ttEntry->getFlags() == TranspositionTable::Flags::ExactScore);

    tt.clear();
    ttEntry = tt.probe(5770153743293125963);
    BOOST_CHECK(!ttEntry);
}



