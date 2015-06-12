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

#include "..\src\piece.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(RandomNormalPiece)
{
    const Piece p(Piece::WhiteRook);

    BOOST_CHECK(p.isOk());
    BOOST_CHECK(p.canRepresentPieceType());
    BOOST_CHECK(p.getPieceType() == Piece::Rook);
    BOOST_CHECK(p.getPieceType().pieceTypeIsOk());
}

BOOST_AUTO_TEST_CASE(Empty)
{
    const Piece p(Piece::Empty);

    BOOST_CHECK(p.isOk());
    BOOST_CHECK(!p.canRepresentPieceType());
}

BOOST_AUTO_TEST_CASE(NoPiece)
{
    const Piece p;

    BOOST_CHECK(!p.isOk());
}



