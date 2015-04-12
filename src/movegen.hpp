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

#ifndef MOVEGEN_HPP_
#define MOVEGEN_HPP_

#include <vector>
#include "position.hpp"
#include "move.hpp"
#include "movelist.hpp"

class MoveGen
{
public:
    void generatePseudoLegalMoves(const Position& pos, MoveList& moves);
    void generatePseudoLegalCaptures(const Position& pos, MoveList& moves, bool underPromotions);
    void generateLegalEvasions(const Position& pos, MoveList& moves);
    void generatePseudoLegalCapturesAndQuietChecks(const Position& pos, MoveList& moves);
    void generatePseudoLegalQuietChecks(const Position& pos, MoveList& moves);
    void generatePseudoLegalQuietMoves(const Position& pos, MoveList& moves);
private:
    template <bool side> 
    void generatePseudoLegalMoves(const Position& pos, MoveList& moves);

    template <bool side>
    void generatePseudoLegalCaptures(const Position& pos, MoveList& moves, bool underPromotions);

    template <bool side>
    void generateLegalEvasions(const Position& pos, MoveList& moves);

    template <bool side>
    void generatePseudoLegalCapturesAndQuietChecks(const Position& pos, MoveList& moves);

    template <bool side>
    void generatePseudoLegalQuietChecks(const Position& pos, MoveList& moves);

    template <bool side>
    void generatePseudoLegalQuietMoves(const Position& pos, MoveList& moves);
};

#endif
