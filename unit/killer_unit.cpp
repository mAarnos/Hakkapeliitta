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

#include "..\src\killer.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(KillerTest)
{
    KillerTable killerTable;
    const Move m1(Square::C3, Square::E4, Piece::Empty);
    const Move m2(Square::E8, Square::G8, Piece::King);

    killerTable.update(m1, 7);
    killerTable.update(m2, 7);
    killerTable.update(m2, 7); // Try to add the same move twice, should not do anything.

    const auto killers = killerTable.getKillers(7);
    BOOST_CHECK(killers.first == m2);
    BOOST_CHECK(killers.second == m1);
}


