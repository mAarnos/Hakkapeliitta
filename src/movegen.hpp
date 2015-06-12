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

/// @file movegen.hpp
/// @author Mikko Aarnos

#ifndef MOVEGEN_HPP_
#define MOVEGEN_HPP_

#include <vector>
#include "position.hpp"
#include "move.hpp"
#include "movelist.hpp"

/// @brief Contains functions for generating moves.
///
/// Everything is static due to convenience.
class MoveGen
{
public:
    /// @brief Generates pseudo-legal moves.
    /// @param pos The position for which to generate moves.
    /// @param moveList The movelist into which we should put the generated moves.
    static void generatePseudoLegalMoves(const Position& pos, MoveList& moveList);

    /// @brief Generates legal evasion moves. Should NOT be called if not in check.
    /// @param pos The position for which to generate moves.
    /// @param moveList The movelist into which we should put the generated moves.
    static void generateLegalEvasions(const Position& pos, MoveList& moveList);

    /// @brief Generates pseudo-legal quiet (i.e. non-capture) moves.
    /// @param pos The position for which to generate moves.
    /// @param moveList The movelist into which we should put the generated moves.
    static void generatePseudoLegalQuietMoves(const Position& pos, MoveList& moveList);

    /// @brief Generates pseudo-legal capture moves, promotions and quiet checks.
    /// @param pos The position for which to generate moves.
    /// @param moveList The movelist into which we should put the generated moves.
    static void generatePseudoLegalCapturesAndQuietChecks(const Position& pos, MoveList& moveList);

    /// @brief Generates pseudo-legal capture moves and promotions.
    /// @param pos The position for which to generate moves.
    /// @param moveList The movelist into which we should put the generated moves.
    /// @param underPromotions Whether we should generate underpromotions or not.
    ///
    /// In the quiescence search generating underpromotions is a waste of time.
    /// On the other hand, in the main search NOT generating underpromotions could potentially have disastrous effects.
    static void generatePseudoLegalCaptures(const Position& pos, MoveList& moveList, bool underPromotions);

private:
    template <bool side> 
    static void generatePseudoLegalMoves(const Position& pos, MoveList& moveList);

    template <bool side>
    static void generateLegalEvasions(const Position& pos, MoveList& moveList);

    template <bool side>
    static void generatePseudoLegalQuietMoves(const Position& pos, MoveList& moveList);

    template <bool side>
    static void generatePseudoLegalCapturesAndQuietChecks(const Position& pos, MoveList& moveList);

    template <bool side>
    static void generatePseudoLegalCaptures(const Position& pos, MoveList& moveList, bool underPromotions);
};

#endif
