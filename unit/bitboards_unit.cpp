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

#include "..\src\bitboards.hpp"
#include <boost\test\unit_test.hpp>

BOOST_AUTO_TEST_CASE(RandomBishopAttacks)
{
    BOOST_CHECK(Bitboards::bishopAttacks(Square::D4, 0) == 0x8041221400142241);
    BOOST_CHECK(Bitboards::bishopAttacks(Square::E6, 0) == 0x4428002844820100);
    BOOST_CHECK(Bitboards::bishopAttacks(Square::D2, 0x917D731812A4FF91) == 0x0000804020140014);
    BOOST_CHECK(Bitboards::bishopAttacks(Square::B3, 0x617e91505802c7a1) == 0x0000100805000500);
}

BOOST_AUTO_TEST_CASE(RandomRookAttacks)
{
    BOOST_CHECK(Bitboards::rookAttacks(Square::G3, 0) == 0x4040404040BF4040);
    BOOST_CHECK(Bitboards::rookAttacks(Square::C5, 0) == 0x040404FB04040404);
    BOOST_CHECK(Bitboards::rookAttacks(Square::B4, 0x00040883a2005000) == 0x000000023D020202);
    BOOST_CHECK(Bitboards::rookAttacks(Square::A1, 0x00e040000302c811) == 0x000000000101011E);
}

BOOST_AUTO_TEST_CASE(RandomQueenAttacks)
{
    BOOST_CHECK(Bitboards::queenAttacks(Square::A1, 0) == 0x81412111090503FE);
    BOOST_CHECK(Bitboards::queenAttacks(Square::D7, 0) == 0x1CF71C2A49880808);
    BOOST_CHECK(Bitboards::queenAttacks(Square::D8, 0x69FA3D003816C7A9) == 0x371C020100000000);
    BOOST_CHECK(Bitboards::queenAttacks(Square::H6, 0x60B8CA3E02E06150) == 0x20C040C0A0900804);
}

BOOST_AUTO_TEST_CASE(RandomBit)
{
    BOOST_CHECK(Bitboards::bit(0) == 1ULL);
    BOOST_CHECK(Bitboards::bit(4) == 1ULL << 4);
    BOOST_CHECK(Bitboards::bit(21) == 1ULL << 21);
    BOOST_CHECK(Bitboards::bit(46) == 1ULL << 46);
    BOOST_CHECK(Bitboards::bit(59) == 1ULL << 59);
}

BOOST_AUTO_TEST_CASE(RandomKingAttacks)
{
    BOOST_CHECK(Bitboards::kingAttacks(Square::A1) == 0x0000000000000302);
    BOOST_CHECK(Bitboards::kingAttacks(Square::F6) == 0x0070507000000000);
    BOOST_CHECK(Bitboards::kingAttacks(Square::H3) == 0x00000000C040C000);
}

BOOST_AUTO_TEST_CASE(RandomKnightAttacks)
{
    BOOST_CHECK(Bitboards::knightAttacks(Square::A1) == 0x0000000000020400);
    BOOST_CHECK(Bitboards::knightAttacks(Square::B3) == 0x0000000508000805);
    BOOST_CHECK(Bitboards::knightAttacks(Square::F6) == 0x5088008850000000);
    BOOST_CHECK(Bitboards::knightAttacks(Square::G2) == 0x00000000A0100010);
}

BOOST_AUTO_TEST_CASE(RandomPawnAttacks)
{
    BOOST_CHECK(Bitboards::pawnAttacks(Color::White, Square::A4) == 0x0000000200000000);
    BOOST_CHECK(Bitboards::pawnAttacks(Color::White, Square::H3) == 0x0000000040000000);
    BOOST_CHECK(Bitboards::pawnAttacks(Color::White, Square::E4) == 0x0000002800000000);
    BOOST_CHECK(Bitboards::pawnAttacks(Color::White, Square::F8) == 0); // Should not be necessary.
    BOOST_CHECK(Bitboards::pawnAttacks(Color::Black, Square::A5) == 0x0000000002000000);
    BOOST_CHECK(Bitboards::pawnAttacks(Color::Black, Square::H2) == 0x0000000000000040);
    BOOST_CHECK(Bitboards::pawnAttacks(Color::Black, Square::C7) == 0x00000A0000000000);
}

BOOST_AUTO_TEST_CASE(RandomSquaresBetween)
{
    BOOST_CHECK(Bitboards::squaresBetween(Square::D2, Square::C7) == 0);
    BOOST_CHECK(Bitboards::squaresBetween(Square::C6, Square::H6) == 0x0000780000000000);
    BOOST_CHECK(Bitboards::squaresBetween(Square::A1, Square::G7) == 0x0000201008040200);
    BOOST_CHECK(Bitboards::squaresBetween(Square::E8, Square::H5) == 0x0020400000000000);
}

BOOST_AUTO_TEST_CASE(RandomLineFormedBySquares)
{
    BOOST_CHECK(Bitboards::lineFormedBySquares(Square::D5, Square::E7) == 0);
    BOOST_CHECK(Bitboards::lineFormedBySquares(Square::E2, Square::F3) == 0x0000008040201008);
    BOOST_CHECK(Bitboards::lineFormedBySquares(Square::B3, Square::B7) == 0x0202020202020202);
    BOOST_CHECK(Bitboards::lineFormedBySquares(Square::D4, Square::A4) == 0x00000000FF000000);
}

BOOST_AUTO_TEST_CASE(RandomRay)
{
    BOOST_CHECK(Bitboards::ray(0, Square::E7) == 0x0000080402010000);
    BOOST_CHECK(Bitboards::ray(1, Square::F5) == 0x0000000020202020);
    BOOST_CHECK(Bitboards::ray(2, Square::A6) == 0x0000000204081020);
    BOOST_CHECK(Bitboards::ray(3, Square::C4) == 0x0000000003000000);
    BOOST_CHECK(Bitboards::ray(4, Square::D8) == 0xF000000000000000);
    BOOST_CHECK(Bitboards::ray(5, Square::F3) == 0x0102040810000000);
    BOOST_CHECK(Bitboards::ray(6, Square::E3) == 0x1010101010000000);
    BOOST_CHECK(Bitboards::ray(7, Square::B5) == 0x1008040000000000);
}

BOOST_AUTO_TEST_CASE(RandomPassedPawn)
{
    BOOST_CHECK(Bitboards::passedPawn(Color::White, Square::C4) == 0x0E0E0E0E00000000);
    BOOST_CHECK(Bitboards::passedPawn(Color::White, Square::F7) == 0x7000000000000000);
    BOOST_CHECK(Bitboards::passedPawn(Color::White, Square::H2) == 0xC0C0C0C0C0C00000);
    BOOST_CHECK(Bitboards::passedPawn(Color::White, Square::C8) == 0); // Should never be necessary
    BOOST_CHECK(Bitboards::passedPawn(Color::Black, Square::H5) == 0x00000000C0C0C0C0);
    BOOST_CHECK(Bitboards::passedPawn(Color::Black, Square::D7) == 0x00001C1C1C1C1C1C);
}

BOOST_AUTO_TEST_CASE(RandomBackwardPawn)
{
    BOOST_CHECK(Bitboards::backwardPawn(Color::White, Square::A6) == 0x0000020202020202);
    BOOST_CHECK(Bitboards::backwardPawn(Color::White, Square::D5) == 0x0000001414141414);
    BOOST_CHECK(Bitboards::backwardPawn(Color::White, Square::F8) == 0); // Should never be necessary
    BOOST_CHECK(Bitboards::backwardPawn(Color::Black, Square::F4) == 0x5050505050000000);
    BOOST_CHECK(Bitboards::backwardPawn(Color::Black, Square::A3) == 0x0202020202020000);
    BOOST_CHECK(Bitboards::backwardPawn(Color::Black, Square::H5) == 0x4040404000000000);
}

BOOST_AUTO_TEST_CASE(RandomIsolatedPawn)
{
    BOOST_CHECK(Bitboards::isolatedPawn(Square::A4) == 0x0202020202020202);
    BOOST_CHECK(Bitboards::isolatedPawn(Square::D6) == 0x1414141414141414);
    BOOST_CHECK(Bitboards::isolatedPawn(Square::H7) == 0x4040404040404040);
    BOOST_CHECK(Bitboards::isolatedPawn(Square::F1) == 0); // Should never be necessary
}

BOOST_AUTO_TEST_CASE(RandomKingSafetyZone)
{
    // The values we get are inconsistent, fix them.
    BOOST_CHECK(Bitboards::kingSafetyZone(Color::White, Square::G1) == 0x0000000000E0E0E0);
    BOOST_CHECK(Bitboards::kingSafetyZone(Color::White, Square::A1) == 0x0000000000030303);
    BOOST_CHECK(Bitboards::kingSafetyZone(Color::Black, Square::C7) == 0x0E0E0E0E00000000);
    BOOST_CHECK(Bitboards::kingSafetyZone(Color::Black, Square::F2) == 0x0000000000707070);
    BOOST_CHECK(Bitboards::kingSafetyZone(Color::Black, Square::D1) == 0x0000000000001C1C);
}

BOOST_AUTO_TEST_CASE(RandomSoftwarePopcnt)
{
    BOOST_CHECK(Bitboards::popcnt<false>(0) == 0);
    BOOST_CHECK(Bitboards::popcnt<false>(7) == 3);
    BOOST_CHECK(Bitboards::popcnt<false>(934) == 6);
    BOOST_CHECK(Bitboards::popcnt<false>(7991775816748) == 23);
}

BOOST_AUTO_TEST_CASE(RandomHardwarePopcnt)
{
    // Don't test this if it is not available on the test platform.
    if (Bitboards::hardwarePopcntSupported())
    {
        BOOST_CHECK(Bitboards::popcnt<true>(1) == 1);
        BOOST_CHECK(Bitboards::popcnt<true>(15) == 4);
        BOOST_CHECK(Bitboards::popcnt<true>(5973) == 8);
        BOOST_CHECK(Bitboards::popcnt<true>(981599164921886363) == 37);
    }
}

BOOST_AUTO_TEST_CASE(RandomLsb)
{
    BOOST_CHECK(Bitboards::lsb(4) == 2);
    BOOST_CHECK(Bitboards::lsb(631) == 0);
    BOOST_CHECK(Bitboards::lsb(7947168) == 5);
    BOOST_CHECK(Bitboards::lsb(153131200612505600) == 11);
}

BOOST_AUTO_TEST_CASE(RandomMsb)
{
    BOOST_CHECK(Bitboards::msb(9) == 3);
    BOOST_CHECK(Bitboards::msb(1568) == 10);
    BOOST_CHECK(Bitboards::msb(96816658716) == 36);
    BOOST_CHECK(Bitboards::msb(4507999823462401) == 52);
}

BOOST_AUTO_TEST_CASE(RandomPopLsb)
{
    Bitboard bb = 382808202783793668;
    const auto lsb = Bitboards::popLsb(bb);

    BOOST_CHECK(lsb == 2);
    BOOST_CHECK(bb == 382808202783793664);
}

BOOST_AUTO_TEST_CASE(RandomMoreThanOneBitSet)
{
    BOOST_CHECK(!Bitboards::moreThanOneBitSet(1));
    BOOST_CHECK(Bitboards::moreThanOneBitSet(5691));
    BOOST_CHECK(!Bitboards::moreThanOneBitSet(35184372088832));
    BOOST_CHECK(Bitboards::moreThanOneBitSet(1153348286935992994));
}

BOOST_AUTO_TEST_CASE(RandomTestBit)
{
    BOOST_CHECK(Bitboards::testBit(681961, 0));
    BOOST_CHECK(Bitboards::testBit(1125900510969856, 14));
    BOOST_CHECK(!Bitboards::testBit(5919867405614370892, 45));
}

BOOST_AUTO_TEST_CASE(RandomSetBit)
{
    Bitboard bb = 0;
    Bitboards::setBit(bb, 37);

    BOOST_CHECK(bb == 137438953472);
}

BOOST_AUTO_TEST_CASE(RandomClearBit)
{
    Bitboard bb = 4612112766411472964;
    Bitboards::clearBit(bb, 2);
    Bitboards::clearBit(bb, 37);
    Bitboards::clearBit(bb, 42);

    BOOST_CHECK(bb == 4612108230926008384);
}




