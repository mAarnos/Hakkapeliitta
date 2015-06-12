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

#include "..\src\position.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(GENERAL_FUNCTIONS_1)
{
    const Position pos("r4rk1/1q1bbppp/2np1n2/1p2p3/p2PP3/4BN1P/PPBN1PP1/2RQR1K1 w - - 0 18");

    BOOST_CHECK(pos.getBoard(Square::G1) == Piece::WhiteKing);
    BOOST_CHECK(pos.getBoard(Square::D4) == Piece::WhitePawn);
    BOOST_CHECK(pos.getBoard(Square::C6) == Piece::BlackKnight);
    BOOST_CHECK(pos.getBoard(Square::B7) == Piece::BlackQueen);

    BOOST_CHECK(pos.getBitboard(Color::White, Piece::Bishop) == 0x0000000000100400);
    BOOST_CHECK(pos.getBitboard(Color::Black, Piece::Pawn) == 0x00E0081201000000);

    BOOST_CHECK(pos.getPieces(Color::White) == 0x0000000018B06F5C);
    BOOST_CHECK(pos.getPieces(Color::Black) == 0x61FA2C1201000000);

    BOOST_CHECK(pos.getOccupiedSquares() == 0x61FA2C1219B06F5C);
    BOOST_CHECK(pos.getFreeSquares() == 0x9E05D3EDE64F90A3);

    BOOST_CHECK(pos.getSideToMove() == Color::White);
    BOOST_CHECK(pos.getEnPassantSquare() == Square::NoSquare);
    BOOST_CHECK(!pos.getCastlingRights());
    BOOST_CHECK(!pos.getFiftyMoveDistance());
    BOOST_CHECK(!pos.getGamePhase());

    BOOST_CHECK(pos.getPieceCount(Color::White, Piece::Pawn) == 7);
    BOOST_CHECK(pos.getPieceCount(Color::White, Piece::Knight) == 2);
    BOOST_CHECK(pos.getPieceCount(Color::Black, Piece::King) == 1);
    BOOST_CHECK(pos.getPieceCount(Color::Black, Piece::Rook) == 2);

    BOOST_CHECK(pos.getNonPawnPieceCount(Color::White) == 7);
    BOOST_CHECK(pos.getNonPawnPieceCount(Color::Black) == 7);

    BOOST_CHECK(!pos.inCheck());
}

BOOST_AUTO_TEST_CASE(GENERAL_FUNCTIONS_2)
{
    const Position pos("8/2p5/1p1p4/KP5r/1R3P1k/8/4P1P1/8 w - - 15 65");

    BOOST_CHECK(pos.getFiftyMoveDistance() == 15);
    BOOST_CHECK(pos.getGamePly() == 129);
    BOOST_CHECK(pos.getGamePhase() == 54);

    BOOST_CHECK(pos.getPinnedPieces() == 0x0000000200000000);
    BOOST_CHECK(pos.getDiscoveredCheckCandidates() == 0x0000000020000000);

    BOOST_CHECK(pos.inCheck());
}

BOOST_AUTO_TEST_CASE(SEE_AND_MVVLVA)
{
    struct Test
    {
        std::string fen;
        Move move;
        int seeScore;
        int mvvLvaScore;
        bool captureOrPromotion;
    };

    const std::array<Test, 3> tests = {{
        { "r1bqk2r/2p1bppp/p1np1n2/1p2p3/4P3/1BP2N2/PP1P1PPP/RNBQR1K1 b kq - 0 8", Move(Square::E8, Square::G8, Piece::King), 0, -8, false },
        { "2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2RN1PPP/2R4K b - b3 0 23", Move(Square::C4, Square::B3, Piece::Pawn), 100, 13, true },
        { "8/4k3/8/8/RrR1N2r/8/5K2/8 b - - 11 1", Move(Square::H4, Square::E4, Piece::Empty), -200, 18, true }
    }};

    for (auto& test : tests)
    {
        Position pos(test.fen);
        BOOST_CHECK(pos.captureOrPromotion(test.move) == test.captureOrPromotion);
        BOOST_CHECK(pos.SEE(test.move) == test.seeScore);
        BOOST_CHECK(pos.mvvLva(test.move) == test.mvvLvaScore);
    }
}



