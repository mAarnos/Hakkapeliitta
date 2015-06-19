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

#include "test.hpp"
#include <cctype>
#include <sstream>
#include "position.hpp"
#include "evaluation.hpp"
#include "movelist.hpp"
#include "movegen.hpp"

Testing::Testing(const std::string& fileName)
{
    std::ifstream file(fileName);
    std::string s;

    while (std::getline(file, s))
    {
        positions.push_back(s);
    }
}

char switchCase(unsigned char c)
{
    return static_cast<char>(std::isupper(c) ? std::tolower(c) : std::toupper(c));
}

std::string flipFenString(const std::string& fen)
{
    std::string f, token;
    std::stringstream ss(fen);

    for (auto i = 0; i < 8; i++)
    {
        std::getline(ss, token, i < 7 ? '/' : ' ');
        std::transform(token.begin(), token.end(), token.begin(), switchCase);
        f.insert(0, token + (i ? "/" : " "));
    }

    ss >> token; // Side to move
    f += (token == "w" ? "b " : "w ");

    ss >> token; // Castling flags
    std::transform(token.begin(), token.end(), token.begin(), switchCase);
    f += token + " ";

    ss >> token; // En-passant square
    f += (token == "-" ? token : token.replace(1, 1, token[1] == '3' ? "6" : "3"));

    std::getline(ss, token); // Full and half moves
    f += token;

    return f;
}

bool Testing::testReversedEval()
{
    Evaluation evaluation;

    for (const auto& s : positions)
    {
        Position pos(s);
        Position pos2(flipFenString(s));
        const auto score = evaluation.evaluate(pos);
        const auto flippedScore = evaluation.evaluate(pos2);

        if (score != flippedScore)
        {
            return false;
        }
    }

    return true;
}

bool Testing::testPseudoLegal()
{
    for (const auto& s : positions)
    {
        Position pos(s);
        MoveList moveList, moveList2;
        const auto inCheck = pos.inCheck();

        inCheck ? MoveGen::generateLegalEvasions(pos, moveList) 
                : MoveGen::generatePseudoLegalMoves(pos, moveList);

        for (uint16_t i = 0; i < std::numeric_limits<uint16_t>::max(); ++i)
        {
            if (pos.pseudoLegal(i, inCheck))
            {
                moveList2.emplace_back(i);
            }
        }

        if (moveList.size() != moveList2.size())
        {
            return false;
        }
    }

    return true;
}
