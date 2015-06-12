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

#include "..\src\zobrist.hpp"
#include <numeric>
#include <boost\math\special_functions\gamma.hpp>
#include <boost\test\unit_test.hpp>
#include "..\src\bitboards.hpp"

struct ZobristFixture 
{
    ZobristFixture()
    { 
        std::vector<uint64_t> testData;

        for (Piece p = Piece::WhitePawn; p <= Piece::BlackKing; ++p)
        {
            for (Square sq = Square::A1; sq <= Square::H8; ++sq)
            {
                testData.push_back(Zobrist::pieceHashKey(p, sq));
            }
            for (auto j = 0; j < 8; ++j)
            {
                testData.push_back(Zobrist::materialHashKey(p, j));
            }
        }

        for (Square sq = Square::A1; sq <= Square::H8; ++sq)
        {
            testData.push_back(Zobrist::enPassantHashKey(sq));
        }

        for (auto cr = 0; cr < 16; ++cr)
        {
            testData.push_back(Zobrist::castlingRightsHashKey(cr));
        }

        testData.push_back(Zobrist::turnHashKey());
        testData.push_back(Zobrist::manglingHashKey());
        n = static_cast<int>(testData.size()) * 64;

        for (auto x : testData)
        {
            for (auto i = 0; i < 64; ++i)
            {
                bits.push_back(Bitboards::testBit(x, i));
            }
        }
    }

    std::vector<bool> bits;
    int n;
};

BOOST_FIXTURE_TEST_CASE(FREQUENCY_TEST, ZobristFixture)
{
    assert(n >= 100);

    auto sN = 0;
    for (auto x : bits)
    {
        sN += x ? 1 : -1;
    }

    const auto sObs = std::abs(sN) / std::sqrt(n);
    const auto pValue = std::erfc(sObs / std::sqrt(2));

    BOOST_CHECK(pValue >= 0.01);
}

BOOST_FIXTURE_TEST_CASE(BLOCK_FREQUENCY_TEST, ZobristFixture)
{
    const auto M = 64 * 22;
    const auto N = n / M;

    assert(n >= 100);
    assert(n >= M * N);
    assert(M >= 20);
    assert(M >= 0.01 * n);
    assert(N < 100);

    auto chiSquared = 0.0;
    for (auto i = 0; i < N; ++i)
    {
        auto setBitsInBlock = 0;
        for (auto j = 0; j < M; ++j)
        {
            setBitsInBlock += bits[i * 12 + j];
        }
        const auto pi = setBitsInBlock / static_cast<double>(M);
        chiSquared += std::pow(pi - 0.5, 2);
    }
    chiSquared *= 4 * M;

    const auto pValue = boost::math::gamma_q(N / 2.0, chiSquared / 2.0);

    BOOST_CHECK(pValue >= 0.01);
}

BOOST_FIXTURE_TEST_CASE(RUNS_TEST, ZobristFixture)
{
    assert(n >= 100);

    const auto setBits = std::accumulate(bits.begin(), bits.end(), 0);
    const auto pi = setBits / static_cast<double>(n);

    auto Vn = 1;
    for (auto i = 0; i < n - 1; ++i)
    {
        Vn += bits[i] != bits[i + 1];
    }

    const auto top = std::abs(Vn - 2.0 * n * pi * (1.0 - pi));
    const auto bottom = 2.0 * std::sqrt(2.0 * n) * pi * (1.0 - pi);
    const auto pValue = std::erfc(top / bottom);

    BOOST_CHECK(pValue >= 0.01);
}
