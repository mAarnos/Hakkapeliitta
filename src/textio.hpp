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

/// @file textio.hpp
/// @author Mikko Aarnos

#ifndef TEXTIO_HPP_
#define TEXTIO_HPP_

#include <sstream>
#include "position.hpp"
#include "constants.hpp"

/// @brief Used for printin a Position into a ostream.
/// @param out The ostream to print to.
/// @param pos The position to print.
/// @return A reference to the ostream to allow chaining.
inline std::ostream& operator<<(std::ostream &out, const Position& pos)
{
    static const auto pieceToMark = "PNBRQKpnbrqk.";
    std::stringstream ss;

    ss << "  +-----------------------+" << std::endl;
    for (auto i = 7; i >= 0; --i)
    {
        ss << i + 1 << " ";
        for (auto j = 0; j < 8; ++j)
        {
            ss << "|" << pieceToMark[pos.getBoard(i * 8 + j)] << " ";
        }
        ss << "|" << std::endl << "  +--+--+--+--+--+--+--+--+" << std::endl;
    }
    ss << "   A  B  C  D  E  F  G  H" << std::endl;

    out << ss.str() << std::endl;
    return out;
}

/// @brief Used for getting the FEN-string of a position.
/// @param pos The position.
/// @return The FEN-string.
inline std::string positionToFen(const Position& pos)
{
    std::string fen;

    for (auto r = 7; r >= 0; --r)
    {
        auto emptySquares = 0;
        for (auto f = 0; f < 8; ++f)
        {
            int piece = pos.getBoard(r * 8 + f);
            if (piece == Piece::Empty)
            {
                ++emptySquares;
            }
            else
            {
                if (emptySquares > 0)
                {
                    fen += static_cast<char>('0' + emptySquares);
                    emptySquares = 0;
                }

                switch (piece)
                {
                case Piece::BlackPawn: fen += "p"; break;
                case Piece::BlackRook: fen += "r"; break;
                case Piece::BlackKnight: fen += "n"; break;
                case Piece::BlackBishop: fen += "b"; break;
                case Piece::BlackQueen: fen += "q"; break;
                case Piece::BlackKing: fen += "k"; break;
                case Piece::WhitePawn: fen += "P"; break;
                case Piece::WhiteRook: fen += "R"; break;
                case Piece::WhiteKnight: fen += "N"; break;
                case Piece::WhiteBishop: fen += "B"; break;
                case Piece::WhiteQueen: fen += "Q"; break;
                case Piece::WhiteKing: fen += "K"; break;
                default: return "";
                }
            }
        }

        if (emptySquares > 0)
        {
            fen += static_cast<char>('0' + emptySquares);
        }
        if (rank > 0)
        {
            fen += '/';
        }
    }

    fen += (pos.getSideToMove() ? " b " : " w ");

    const auto sizeBeforeCastling = fen.size();
    if (pos.getCastlingRights() & CastlingRights::WhiteOO)
    {
        fen += "K";
    }
    if (pos.getCastlingRights() & CastlingRights::WhiteOOO)
    {
        fen += "Q";
    }
    if (pos.getCastlingRights() & CastlingRights::BlackOO)
    {
        fen += "k";
    }
    if (pos.getCastlingRights() & CastlingRights::BlackOOO)
    {
        fen += "q";
    }
    if (sizeBeforeCastling == fen.size())
    {
        fen += "-";
    }

    fen += " ";
    if (pos.getEnPassantSquare() != Square::NoSquare)
    {
        fen += static_cast<char>('a' + file(pos.getEnPassantSquare()));
        fen += static_cast<char>('1' + rank(pos.getEnPassantSquare()));
    }
    else
    {
        fen += "-";
    }

    fen += " ";
    fen += std::to_string(pos.getFiftyMoveDistance());
    fen += " ";
    fen += std::to_string((pos.getGamePly() + 1) / 2);

    return fen;
}

/// @brief Used for converting a move into UCI-format.
/// @param move The move.
/// @return The move as a string.
inline std::string moveToUciFormat(const Move& move)
{
    static const std::array<std::string, 64> squareToNotation = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    };

    static const std::array<std::string, 64> flagToNotation = {
        "p", "n", "b", "r", "q", "k"
    };

    std::string s;
    const auto from = move.getFrom();
    const auto to = move.getTo();
    const auto flags = move.getFlags();

    s += squareToNotation[from] + squareToNotation[to];
    if (flags != Piece::Empty && flags != Piece::King && flags != Piece::Pawn)
    {
        s += flagToNotation[flags];
    }

    return s;
}

/// @brief Used for converting multiple moves into UCI-format.
/// @param moves A list of moves.
/// @return The moves as a string. 
inline std::string movesToUciFormat(const std::vector<Move>& moves)
{
    std::string s;

    for (auto& move : moves)
    {
        s += moveToUciFormat(move) + " ";
    }

    return s;
}

#endif
