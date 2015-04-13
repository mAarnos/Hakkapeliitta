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

#include "position.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "bitboard.hpp"
#include "zobrist.hpp"
#include "color.hpp"
#include "eval.hpp"
#include "square.hpp"
#include "utils/synchronized_ostream.hpp"

const std::array<int, 64> castlingMask = {
	2, 0, 0, 0, 3, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	8, 0, 0, 0, 12, 0, 0, 4
};

const std::array<int8_t, 6> piecePhase = {
    0, 3, 3, 5, 10, 0
};

const int8_t totalPhase = piecePhase[Piece::Pawn] * 16 
                        + piecePhase[Piece::Knight] * 4 
                        + piecePhase[Piece::Bishop] * 4 
                        + piecePhase[Piece::Rook] * 4 
                        + piecePhase[Piece::Queen] * 2;

Position::Position()
{
    bitboards.fill(0);
    board.fill(Piece::Empty);
    hashKey = pawnHashKey = materialHashKey = 0;
    sideToMove = Color::White;
    castlingRights = 0;
    enPassant = Square::NoSquare;
    fiftyMoveDistance = 0;
    totalPieceCount = 0;
    gamePly = 0;
    gamePhase = 0;
    pieceCount.fill(0);
    pstScoreOp = pstScoreEd = 0;
}

Position::Position(const std::string& fen)
{
    initializePositionFromFen(fen);
}

void Position::initializePositionFromFen(const std::string& fen)
{
	board.fill(Piece::Empty);

	// Split the FEN into parts.
	std::vector<std::string> strList;
	std::stringstream ss(fen);
	std::string item;
	while (getline(ss, item, ' ')) 
	{
		strList.push_back(item);
	}

	auto i = 0, j = 1;
	// Translate the FEN string into piece locations on the board.
	while ((j <= 64) && (i <= static_cast<int>(strList[0].length()))) 
	{
        const auto letter = strList[0][i++];
		const auto f = 1 + file(j - 1);
        const auto r = 8 - rank(j - 1);
		const auto sq = (((r - 1) * 8) + (f - 1));
		switch (letter)
		{
            case 'p': board[sq] = Piece::BlackPawn; break;
            case 'r': board[sq] = Piece::BlackRook; break;
            case 'n': board[sq] = Piece::BlackKnight; break;
            case 'b': board[sq] = Piece::BlackBishop; break;
            case 'q': board[sq] = Piece::BlackQueen; break;
            case 'k': board[sq] = Piece::BlackKing; break;
            case 'P': board[sq] = Piece::WhitePawn; break;
            case 'R': board[sq] = Piece::WhiteRook; break;
            case 'N': board[sq] = Piece::WhiteKnight; break;
            case 'B': board[sq] = Piece::WhiteBishop; break;
            case 'Q': board[sq] = Piece::WhiteQueen; break;
            case 'K': board[sq] = Piece::WhiteKing; break;
			case '/': --j; break;
			case '1': break;
			case '2': ++j; break;
			case '3': j += 2; break;
			case '4': j += 3; break;
			case '5': j += 4; break;
			case '6': j += 5; break;
			case '7': j += 6; break;
			case '8': j += 7; break;
            default: throw std::invalid_argument("fen string is incorrect: unknown character encountered");
		}
		++j;
	}

	// set the turn; default = White 
	sideToMove = Color::White;
	if (strList.size() >= 2)
	{
		if (strList[1] == "w")
		{
            sideToMove = Color::White;
		}
        else if (strList[1] == "b")
        {
            sideToMove = Color::Black;
        }
        else
        {
            throw std::invalid_argument("fen string is incorrect: side to move is corrupted");
        }
	}

	// Set castlingRights to default: no castling allowed.
	castlingRights = 0;
	// Add all given castling rights.
	if (strList.size() >= 3)
	{
		if (strList[2].find('K') != std::string::npos)
		{
			castlingRights += 1;
		}
        if (strList[2].find('Q') != std::string::npos)
		{
			castlingRights += 2;
		}
        if (strList[2].find('k') != std::string::npos)
		{
			castlingRights += 4;
		}
        if (strList[2].find('q') != std::string::npos)
		{
			castlingRights += 8;
		}
	}

	// Set the en passant square, if any.
	enPassant = Square::NoSquare;
	if ((strList.size() >= 4) && (strList[3].length() >= 2))
	{
		if ((strList[3].at(0) >= 'a') && (strList[3].at(0) <= 'h') && ((strList[3].at(1) == '3') || (strList[3].at(1) == '6')))
		{
			const auto f = strList[3][0] - 96; // ASCII 'a' = 97 
			const auto r = strList[3][1] - 48; // ASCII '1' = 49 
			enPassant = ((r - 1) * 8 + f - 1);
		}
        else
        {
            throw std::invalid_argument("fen string is incorrect: en passant square is corrupted");
        }
	}

	// Fifty move distance, we start at 0 by default.
	fiftyMoveDistance = 0;
	if (strList.size() >= 5)
	{
        fiftyMoveDistance = static_cast<int8_t>(std::stoi(strList[4]));
	}

	gamePly = 0;
	if (strList.size() >= 6)
	{
        gamePly = static_cast<short>(std::max(2 * stoi(strList[5]) - 1, 0));
		if (sideToMove == Color::Black)
		{
            ++gamePly;
		}
	}

    // Populate the bitboards.
	bitboards.fill(0);
    pieceCount.fill(0);
    totalPieceCount = 0;
    pstScoreOp = pstScoreEd = 0;
    nonPawnPieceCounts.fill(0);
	for (Square sq = Square::A1; sq <= Square::H8; ++sq) 
	{
		if (board[sq] != Piece::Empty)
		{
            Bitboards::setBit(bitboards[board[sq]], sq);
            pstScoreOp += Evaluation::pieceSquareTableOpening[board[sq]][sq];
            pstScoreEd += Evaluation::pieceSquareTableEnding[board[sq]][sq];
            ++pieceCount[board[sq]];
            ++totalPieceCount;
            if (getPieceType(board[sq]) != Piece::Pawn && getPieceType(board[sq]) != Piece::King)
            {
                ++nonPawnPieceCounts[board[sq] >= Piece::BlackPawn];
            }
		}
	}

    bitboards[12] = bitboards[Piece::WhiteKing] | bitboards[Piece::WhiteQueen] | bitboards[Piece::WhiteRook] 
                  | bitboards[Piece::WhiteBishop] | bitboards[Piece::WhiteKnight] | bitboards[Piece::WhitePawn];
    bitboards[13] = bitboards[Piece::BlackKing] | bitboards[Piece::BlackQueen] | bitboards[Piece::BlackRook] 
                  | bitboards[Piece::BlackBishop] | bitboards[Piece::BlackKnight] | bitboards[Piece::BlackPawn];

    // Calculate all the different hash keys for the position.
	hashKey = calculateHash();
	pawnHashKey = calculatePawnHash();
	materialHashKey = calculateMaterialHash();

    pinned = pinnedPieces(sideToMove);
    dcCandidates = discoveredCheckCandidates();

    // Calculate the phase of the game.
    gamePhase = totalPhase;
    for (Piece p = Piece::Knight; p < Piece::King; ++p)
    {
        gamePhase -= (pieceCount[Color::White + p] + pieceCount[Color::Black * 6 + p]) * piecePhase[p];
    }
}

std::string Position::displayPositionAsString() const
{
    const auto pieceToMark = "PNBRQKpnbrqk.";
    std::stringstream ss;

    ss << "  +-----------------------+" << std::endl;
    for (auto i = 7; i >= 0; --i)
    {
        ss << i + 1 << " ";
        for (auto j = 0; j < 8; ++j)
        {
            ss << "|" << pieceToMark[board[i * 8 + j]] << " ";
        }
        ss << "|" << std::endl << "  +--+--+--+--+--+--+--+--+" << std::endl;
    }
    ss << "   A  B  C  D  E  F  G  H" << std::endl;

    return ss.str();
}

std::string Position::toFen() const
{
    std::string fen;

    for (auto r = 7; r >= 0; --r)
    {
        auto emptySquares = 0;
        for (auto f = 0; f < 8; ++f)
        {
            int piece = board[r * 8 + f];
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

    fen += (sideToMove ? " b " : " w ");

    auto sizeBeforeCastling = fen.size();
    if (castlingRights & 1)
    {
        fen += "K";
    }
    if (castlingRights & 2)
    {
        fen += "Q";
    }
    if (castlingRights & 4)
    {
        fen += "k";
    }
    if (castlingRights & 8)
    {
        fen += "q";
    }
    if (sizeBeforeCastling == fen.size())
    {
        fen += "-";
    }

    fen += " ";
    if (enPassant != Square::NoSquare)
    {
        fen += static_cast<char>('a' + file(enPassant));
        fen += static_cast<char>('1' + rank(enPassant));
    }
    else
    {
        fen += "-";
    }

    fen += " ";
    fen += std::to_string(fiftyMoveDistance);
    fen += " ";
    fen += std::to_string((gamePly + 1) / 2);

    return fen;
}

void Position::makeMove(const Move& m)
{
    sideToMove ? makeMove<true>(m) : makeMove<false>(m);
}

bool Position::isAttacked(const Square sq, const Color side) const
{
    return (side ? isAttacked<true>(sq, getOccupiedSquares()) : isAttacked<false>(sq, getOccupiedSquares()));
}

bool Position::isAttacked(const Square sq, const Color side, Bitboard occupied) const
{
    return (side ? isAttacked<true>(sq, occupied) : isAttacked<false>(sq, occupied));
}

template <bool side> 
void Position::makeMove(const Move& m)
{
    assert(!m.empty());

    const auto from = m.getFrom();
    const auto to = m.getTo();
    const auto promotion = m.getPromotion();
    const auto piece = board[from];
    const auto captured = board[to];
    const auto fromToBB = Bitboards::bit(from) | Bitboards::bit(to);

    // First update the PST score for the piece moving.
    pstScoreOp += Evaluation::pieceSquareTableOpening[piece][to]
                - Evaluation::pieceSquareTableOpening[piece][from];
    pstScoreEd += Evaluation::pieceSquareTableEnding[piece][to]
                - Evaluation::pieceSquareTableEnding[piece][from];

    // If there was an en passant move get rid of it and its hash key.
    if (enPassant != Square::NoSquare)
    {
        hashKey ^= Zobrist::enPassantHashKey(enPassant);
        enPassant = Square::NoSquare;
    }

    board[to] = board[from];
    board[from] = Piece::Empty;
    bitboards[piece] ^= fromToBB;
    bitboards[12 + side] ^= fromToBB;
    ++fiftyMoveDistance;
    hashKey ^= Zobrist::pieceHashKey(piece, from) ^ Zobrist::pieceHashKey(piece, to);

    if (captured != Piece::Empty)
    {
        --totalPieceCount;
        Bitboards::clearBit(bitboards[captured], to);
        Bitboards::clearBit(bitboards[12 + !side], to);
        fiftyMoveDistance = 0;
        pstScoreOp -= Evaluation::pieceSquareTableOpening[captured][to];
        pstScoreEd -= Evaluation::pieceSquareTableEnding[captured][to];
        hashKey ^= Zobrist::pieceHashKey(captured, to);
        materialHashKey ^= Zobrist::materialHashKey(captured, --pieceCount[captured]);

        const auto pieceType = getPieceType(captured);
        gamePhase += piecePhase[pieceType];
        if (pieceType == Piece::Pawn)
        {
            pawnHashKey ^= Zobrist::pieceHashKey(captured, to);
        }
        else
        {
            --nonPawnPieceCounts[!side];
        }
    }

    if (getPieceType(piece) == Piece::Pawn)
    {
        fiftyMoveDistance = 0;
        pawnHashKey ^= Zobrist::pieceHashKey(piece, from) ^ Zobrist::pieceHashKey(piece, to);

        if ((to ^ from) == 16) // Double pawn move
        {
            enPassant = from ^ 24;
            hashKey ^= Zobrist::enPassantHashKey(enPassant);
        }
        else if (promotion == Piece::Pawn) // En passant
        {
            const auto enPassantSquare = to ^ 8;
            --totalPieceCount;
            Bitboards::clearBit(bitboards[Piece::Pawn + !side * 6], enPassantSquare);
            Bitboards::clearBit(bitboards[12 + !side], enPassantSquare);
            board[enPassantSquare] = Piece::Empty;
            hashKey ^= Zobrist::pieceHashKey(Piece::Pawn + !side * 6, enPassantSquare);
            pawnHashKey ^= Zobrist::pieceHashKey(Piece::Pawn + !side * 6, enPassantSquare);
            materialHashKey ^= Zobrist::materialHashKey(Piece::Pawn + !side * 6, --pieceCount[Piece::Pawn + !side * 6]);
            pstScoreOp -= Evaluation::pieceSquareTableOpening[Piece::Pawn + !side * 6][enPassantSquare];
            pstScoreEd -= Evaluation::pieceSquareTableEnding[Piece::Pawn + !side * 6][enPassantSquare];
        }
        else if (promotion != Piece::Empty) // Promotion
        {
            // This needs to be above the rest due to reasons. Try to fix that.
            materialHashKey ^= Zobrist::materialHashKey(promotion + side * 6, pieceCount[promotion + side * 6]++);
            Bitboards::clearBit(bitboards[Piece::Pawn + side * 6], to);
            Bitboards::setBit(bitboards[promotion + side * 6], to);
            board[to] = promotion + sideToMove * 6;
            hashKey ^= Zobrist::pieceHashKey(Piece::Pawn + side * 6, to) ^ Zobrist::pieceHashKey(promotion + side * 6, to);
            pawnHashKey ^= Zobrist::pieceHashKey(Piece::Pawn + side * 6, to);
            materialHashKey ^= Zobrist::materialHashKey(Piece::Pawn + side * 6, --pieceCount[Piece::Pawn + side * 6]);
            gamePhase -= piecePhase[promotion];
            ++nonPawnPieceCounts[side];
            pstScoreOp += Evaluation::pieceSquareTableOpening[promotion + side * 6][to]
                        - Evaluation::pieceSquareTableOpening[Piece::Pawn + side * 6][to];
            pstScoreEd += Evaluation::pieceSquareTableEnding[promotion + side * 6][to]
                        - Evaluation::pieceSquareTableEnding[Piece::Pawn + side * 6][to];
        }
    }
    else if (promotion == Piece::King)
    {
        const auto fromRook = (from > to ? (to - 2) : (to + 1));
        const auto toRook = (from + to) / 2;
        const auto fromToBBCastling = Bitboards::bit(fromRook) | Bitboards::bit(toRook);

        bitboards[Piece::Rook + side * 6] ^= fromToBBCastling;
        bitboards[12 + side] ^= fromToBBCastling;
        board[toRook] = board[fromRook];
        board[fromRook] = Piece::Empty;
        hashKey ^= Zobrist::pieceHashKey(Piece::Rook + side * 6, fromRook) 
                 ^ Zobrist::pieceHashKey(Piece::Rook + side * 6, toRook);
        pstScoreOp += Evaluation::pieceSquareTableOpening[Piece::Rook + side * 6][toRook]
                    - Evaluation::pieceSquareTableOpening[Piece::Rook + side * 6][fromRook];
        pstScoreEd += Evaluation::pieceSquareTableEnding[Piece::Rook + side * 6][toRook]
                    - Evaluation::pieceSquareTableEnding[Piece::Rook + side * 6][fromRook];
    }

    sideToMove = !sideToMove;
    hashKey ^= Zobrist::turnHashKey();
    ++gamePly;

    // Update castling rights if needed
    if (castlingRights && (castlingMask[from] | castlingMask[to]))
    {
        const auto cf = castlingMask[from] | castlingMask[to];
        hashKey ^= Zobrist::castlingRightsHashKey(castlingRights & cf);
        castlingRights &= ~cf;
    }

    pinned = pinnedPieces(sideToMove);
    dcCandidates = discoveredCheckCandidates();

    assert(verifyPsts());
    assert(verifyHashKeysAndPhase());
    assert(verifyPieceCounts());
    assert(verifyBoardAndBitboards());
    assert(pieceCount[Piece::WhiteKnight] + pieceCount[Piece::WhiteBishop] + pieceCount[Piece::WhiteRook] + pieceCount[Piece::WhiteQueen] == nonPawnPieceCounts[Color::White]);
    assert(pieceCount[Piece::BlackKnight] + pieceCount[Piece::BlackBishop] + pieceCount[Piece::BlackRook] + pieceCount[Piece::BlackQueen] == nonPawnPieceCounts[Color::Black]);
}

template void Position::makeMove<false>(const Move& m);
template void Position::makeMove<true>(const Move& m);

void Position::makeNullMove()
{
    sideToMove = !sideToMove;
    hashKey ^= Zobrist::turnHashKey();
    if (enPassant != Square::NoSquare)
    {
        hashKey ^= Zobrist::enPassantHashKey(enPassant);
        enPassant = Square::NoSquare;
    }
    ++fiftyMoveDistance;
    pinned = pinnedPieces(sideToMove);
    dcCandidates = discoveredCheckCandidates();
}

template <bool side>
bool Position::isAttacked(const Square sq, Bitboard occupied) const
{
    return (Bitboards::knightAttacks(sq) & getBitboard(side, Piece::Knight)
        || Bitboards::pawnAttacks(!side, sq) & getBitboard(side, Piece::Pawn)
        || Bitboards::bishopAttacks(sq, occupied) & (getBitboard(side, Piece::Bishop) | getBitboard(side, Piece::Queen))
        || Bitboards::rookAttacks(sq, occupied) & (getBitboard(side, Piece::Rook) | getBitboard(side, Piece::Queen))
        || Bitboards::kingAttacks(sq) & getBitboard(side, Piece::King));
}

template bool Position::isAttacked<false>(const Square sq, Bitboard occupied) const;
template bool Position::isAttacked<true>(const Square sq, Bitboard occupied) const;

HashKey Position::calculateHash() const
{
    auto h = 0ull;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (board[sq] != Piece::Empty)
        {
            h ^= Zobrist::pieceHashKey(board[sq], sq);
        }
    }

    if (enPassant != Square::NoSquare)
    {
        h ^= Zobrist::enPassantHashKey(enPassant);
    }
    if (sideToMove)
    {
        h ^= Zobrist::turnHashKey();
    }
    if (castlingRights)
    {
        h ^= Zobrist::castlingRightsHashKey(castlingRights);
    }

    return h;
}

HashKey Position::calculatePawnHash() const
{
    auto p = 0ull;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (board[sq] == Piece::WhitePawn || board[sq] == Piece::BlackPawn)
        {
            p ^= Zobrist::pieceHashKey(board[sq], sq);
        }
    }

    return p;
}

HashKey Position::calculateMaterialHash() const
{
    auto m = 0ull;

    for (Piece p = Piece::WhitePawn; p <= Piece::BlackKing; ++p)
    {
        for (auto i = 0; i < pieceCount[p]; ++i)
        {
            m ^= Zobrist::materialHashKey(p, i);
        }
    }

    return m;
}

Bitboard Position::checkBlockers(const Color c, const Color kingColor) const
{
    assert(colorIsOk(c) && colorIsOk(kingColor));

    auto result = 0ull;
    const auto kingSquare = Bitboards::lsb(getBitboard(kingColor, Piece::King));
    const auto rq = getBitboard(!kingColor, Piece::Rook) | getBitboard(!kingColor, Piece::Queen);
    const auto bq = getBitboard(!kingColor, Piece::Bishop) | getBitboard(!kingColor, Piece::Queen);
    auto pinners = (rq & Bitboards::rookAttacks(kingSquare, 0)) | (bq & Bitboards::bishopAttacks(kingSquare, 0));

    while (pinners)
    {
        const auto sq = Bitboards::popLsb(pinners);
        const auto skewered = Bitboards::squaresBetween(kingSquare, sq) & getOccupiedSquares();
        if (!Bitboards::moreThanOneBitSet(skewered) && (skewered & getPieces(c)))
        {
            result |= skewered;
        }
    }

    return result;
}

bool Position::pseudoLegal(const Move& move, bool inCheck) const
{
    const auto from = move.getFrom();
    const auto to = move.getTo();
    const auto promotion = move.getPromotion();

    // Check that the piece to be moved is the same color as the side to move.
    if (!Bitboards::testBit(getPieces(sideToMove), from))
    {
        return false;
    }

    // Destination square must not be occupied by a friendly piece.
    // Note that this also catches null moves as they have the same from and to squares.
    if (Bitboards::testBit(getPieces(sideToMove), to))
    {
        return false;
    }

    const auto pieceType = getPieceType(board[from]);
    // Make sure that only pawn moves can have flags which are only for pawn moves.
    if (pieceType != Piece::Pawn && ((promotion >= Piece::Pawn && promotion <= Piece::Queen)))
    {
        return false;
    }

    // Make sure that only king moves can have flags which are only for king moves.
    if (pieceType != Piece::King && promotion == Piece::King)
    {
        return false;
    }

    // Make sure that the promotion flags are within the specified limits.
    if (!(promotion >= Piece::Pawn && promotion <= Piece::King) && promotion != Piece::Empty)
    {
        return false;
    }

    // Check if the moving piece can actually move to the given location. Pawns need special handling.
    if (pieceType == Piece::Pawn)
    {
        // Check that the possible en passant flags are set correctly.
        if ((to == enPassant) == (promotion != Piece::Pawn))
        {
            return false;
        }
        // Check that the possible promotion flags are set correctly.
        else if ((sideToMove ? to <= Square::H1 : to >= Square::A8) == (promotion == Piece::Pawn || promotion == Piece::Empty))
        {
            return false;
        }

        const auto pawnMoveUp = (sideToMove ? -8 : 8);
        if (!(Bitboards::pawnAttacks(sideToMove, from) & Bitboards::bit(to) & (getPieces(!sideToMove) | (enPassant != Square::NoSquare ? Bitboards::bit(enPassant) : 0))) // Not a capture, ep or non-ep
            && !(board[to] == Piece::Empty && from + pawnMoveUp == to) // Not a single pawn move
            && !(from + 2 * pawnMoveUp == to && board[to] == Piece::Empty && board[to - pawnMoveUp] == Piece::Empty && (sideToMove ? 7 - rank(from) : rank(from)) == 1)) // Not a double pawn move.
        {
            return false;
        }
    }
    else
    {
        if (pieceType == Piece::King && promotion == Piece::King)
        {
            // If we are in check castling is not legal.
            if (inCheck)
            {
                return false;
            }

            if (to == (Square::C1 + 56 * sideToMove))
            {
                // Make sure that long castling is legal, disregarding checks.
                if (!(getCastlingRights() & (2 << (2 * sideToMove)) && !(getOccupiedSquares() & (0x000000000000000Eull << (56 * sideToMove)))))
                {
                    return false;
                }

                // Make sure we don't move through a check or into check.
                if (isAttacked(Square::D1 + 56 * sideToMove, !sideToMove) || isAttacked(Square::C1 + 56 * sideToMove, !sideToMove))
                {
                    return false;
                }
            }
            else if (to == (Square::G1 + 56 * sideToMove))
            {
                // Make sure that short castling is legal, disregarding checks.
                if (!(getCastlingRights() & (1 << (2 * sideToMove)) && !(getOccupiedSquares() & (0x0000000000000060ull << (56 * sideToMove)))))
                {
                    return false;
                }

                // Make sure we don't move through a check or into check.
                if (isAttacked(Square::F1 + 56 * sideToMove, !sideToMove) || isAttacked(Square::G1 + 56 * sideToMove, !sideToMove))
                {
                    return false;
                }
            }
            else // The move is not long or short castling -> it can't be legal
            {
                return false;
            }
        }
        else if (!Bitboards::testBit(Bitboards::pieceAttacks(sideToMove, pieceType, from, getOccupiedSquares()), to))
        {
            return false;
        }
    }

    // Since the function legal assumes that when in check all moves are generated by generateLegalEvasions and are thus legal,
    // we must verify their legality here.
    if (inCheck)
    {
        if (pieceType == Piece::King)
        {
            // Check if this king move moves the king out of check.
            if (isAttacked(to, !sideToMove, getOccupiedSquares() ^ Bitboards::bit(from)))
            {
                return false;
            }
        }
        else
        {
            const auto toFrom = Bitboards::bit(from) | Bitboards::bit(to);
            const auto newOccupied = getOccupiedSquares() ^ toFrom ^ (board[to] != Piece::Empty ? Bitboards::bit(to) : 0);
            const auto kingSquare = Bitboards::lsb(getBitboard(sideToMove, Piece::King));
            const auto exclusion = ~(Bitboards::bit(to) | (promotion == Piece::Pawn ? Bitboards::bit(to + (sideToMove ? 8 : -8)) : 0));
            // Check if the move stops our king from being in check.
            if (((Bitboards::knightAttacks(kingSquare) & getBitboard(!sideToMove, Piece::Knight) & exclusion
                || Bitboards::pawnAttacks(sideToMove, kingSquare) & getBitboard(!sideToMove, Piece::Pawn) & exclusion
                || Bitboards::bishopAttacks(kingSquare, newOccupied) & (getBitboard(!sideToMove, Piece::Bishop) | getBitboard(!sideToMove, Piece::Queen)) & exclusion
                || Bitboards::rookAttacks(kingSquare, newOccupied) & (getBitboard(!sideToMove, Piece::Rook) | getBitboard(!sideToMove, Piece::Queen)) & exclusion
                || Bitboards::kingAttacks(kingSquare) & getBitboard(!sideToMove, Piece::King) & exclusion)))
            {
                return false;
            }
        }
    }

    return true;
}


bool Position::legal(const Move& move, const bool inCheck) const
{
    if (inCheck) return true; // As said before, we assume that when in check we generate legal evasions so legality checking is useless.

    const auto from = move.getFrom();
    const auto to = move.getTo();

    if (move.getPromotion() == Piece::Pawn)
    {
        const auto kingSquare = Bitboards::lsb(getBitboard(sideToMove, Piece::King));
        const auto captureSquare = to ^ 8;
        const auto occupied = (getOccupiedSquares() ^ Bitboards::bit(from) ^ Bitboards::bit(captureSquare)) | Bitboards::bit(to);
        const auto rq = getBitboard(!sideToMove, Piece::Rook) | getBitboard(!sideToMove, Piece::Queen);
        const auto bq = getBitboard(!sideToMove, Piece::Bishop) | getBitboard(!sideToMove, Piece::Queen);
        return !(Bitboards::bishopAttacks(kingSquare, occupied) & bq) && !(Bitboards::rookAttacks(kingSquare, occupied) & rq);
    }

    if (getPieceType(board[from]) == Piece::King)
    {
        // Castling is checked for legality in move generation.
        // Otherwise a king move is legal if the target square is not attacked.
        return move.getPromotion() == Piece::King || !isAttacked(to, !sideToMove);
    }

    // Otherwise a move is legal if it is not pinned or it is moving along the ray towards or away from the king.
    return !Bitboards::testBit(pinned, from) 
         || Bitboards::testBit(Bitboards::lineFormedBySquares(from, to), Bitboards::lsb(getBitboard(sideToMove, Piece::King)));
}

int Position::givesCheck(const Move& move) const
{
    const auto from = move.getFrom();
    const auto to = move.getTo();
    const auto promotion = move.getPromotion();
    const auto kingSquare = Bitboards::lsb(getBitboard(!sideToMove, Piece::King)); 
    auto occupied = getOccupiedSquares();

    // Test for discovered check.
    if (Bitboards::testBit(dcCandidates, from) && !Bitboards::testBit(Bitboards::lineFormedBySquares(from, kingSquare), to))
    {
        return 2; 
    }

    // Test for direct checks.
    if (promotion == Piece::Empty || promotion == Piece::Pawn)
    {
        const auto piece = getPieceType(board[from]);
        // A king cannot give check so we can forget about that case. For the same reason the case promotion == Piece::King was unnecessary.
        if (piece != Piece::King)
        {
            const auto attacks = Bitboards::pieceAttacks(sideToMove, piece, to, occupied);
            if (Bitboards::testBit(attacks, kingSquare))
                return 1;
        }
    }

    if (promotion == Piece::King)
    {
        const auto rookTo = (from + to) / 2;
        auto rq = getBitboard(sideToMove, Piece::Rook) | getBitboard(sideToMove, Piece::Queen);
        Bitboards::clearBit(occupied, from);
        Bitboards::setBit(rq, rookTo);
        if (rq & Bitboards::rookAttacks(kingSquare, occupied))
            return 1; // Check by the castled rook
    }
    else if (promotion == Piece::Pawn) 
    {
        const auto rq = getBitboard(sideToMove, Piece::Rook) | getBitboard(sideToMove, Piece::Queen);
        const auto bq = getBitboard(sideToMove, Piece::Bishop) | getBitboard(sideToMove, Piece::Queen);
        Bitboards::clearBit(occupied, from); 
        Bitboards::clearBit(occupied, to ^ 8);
        Bitboards::setBit(occupied, to);
        if ((rq & Bitboards::rookAttacks(kingSquare, occupied)) || (bq & Bitboards::bishopAttacks(kingSquare, occupied)))
            return 2; // Discovered check through the square we left from or the captured pawn.
    }
    else if (promotion != Piece::Empty)
    {
        Bitboards::clearBit(occupied, from);
        if (Bitboards::testBit(Bitboards::pieceAttacks(sideToMove, promotion, to, occupied), kingSquare))
            return 1; // Direct check by the promoted piece.
    }

    return 0;
}

int16_t Position::SEE(const Move& move) const
{
    // Approximate piece values, SEE doesn't need to be as accurate as the main evaluation function.
    // Score for kings is not mateScore due to some annoying wrap-around problems. Doesn't really matter though.
    static const std::array<int16_t, 13> pieceValues = {
        100, 300, 300, 500, 900, 10000, 100, 300, 300, 500, 900, 10000, 0
    };

    static std::array<int16_t, 32> materialGains;
    auto occupied = getOccupiedSquares();
    const auto from = move.getFrom();
    const auto to = move.getTo();
    const auto promotion = move.getPromotion();
    const auto toAtPromoRank = (to <= 7 || to >= 56);
    auto stm = sideToMove;
    int16_t lastAttackerValue;
    int next;

    if (promotion == Piece::King)
    {
        return 0;
    }
    else if (promotion == Piece::Pawn)
    {
        materialGains[0] = pieceValues[Piece::Pawn];
        lastAttackerValue = pieceValues[Piece::Pawn];
        Bitboards::clearBit(occupied, enPassant ^ 8);
    }
    else
    {
        materialGains[0] = pieceValues[board[to]];
        lastAttackerValue = pieceValues[board[from]];
        if (promotion != Piece::Empty)
        {
            materialGains[0] += pieceValues[promotion] - pieceValues[Piece::Pawn];
            lastAttackerValue += pieceValues[promotion] - pieceValues[Piece::Pawn];
        }
    }

    Bitboards::clearBit(occupied, from);
    auto attackers = (Bitboards::rookAttacks(to, occupied) & (bitboards[Piece::WhiteQueen]
                                                            | bitboards[Piece::BlackQueen]
                                                            | bitboards[Piece::WhiteRook]
                                                            | bitboards[Piece::BlackRook]))
                   | (Bitboards::bishopAttacks(to, occupied) & (bitboards[Piece::WhiteQueen]
                                                              | bitboards[Piece::BlackQueen]
                                                              | bitboards[Piece::WhiteBishop]
                                                              | bitboards[Piece::BlackBishop]))
                   | (Bitboards::knightAttacks(to) & (bitboards[Piece::WhiteKnight] | bitboards[Piece::BlackKnight]))
                   | (Bitboards::kingAttacks(to) & (bitboards[Piece::WhiteKing] | bitboards[Piece::BlackKing]))
                   | (Bitboards::pawnAttacks(Color::Black, to) & (bitboards[Piece::WhitePawn]))
                   | (Bitboards::pawnAttacks(Color::White, to) & (bitboards[Piece::BlackPawn]));
    attackers &= occupied;
    stm = !stm;
    auto numberOfCaptures = 1;

    while (attackers & bitboards[12 + stm])
    {
        if (bitboards[Piece::Pawn + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Pawn + stm * 6] & attackers);
        }
        else if (bitboards[Piece::Knight + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Knight + stm * 6] & attackers);
        }
        else if (bitboards[Piece::Bishop + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Bishop + stm * 6] & attackers);
        }
        else if (bitboards[Piece::Rook + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Rook + stm * 6] & attackers);
        }
        else if (bitboards[Piece::Queen + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Queen + stm * 6] & attackers);
        }
        else
        {
            next = Bitboards::lsb(bitboards[Piece::King + stm * 6]);
        }

        // Update the materialgains array.
        materialGains[numberOfCaptures] = -materialGains[numberOfCaptures - 1] + lastAttackerValue;
        // Remember the value of the capturing piece because it is going to be captured next.
        lastAttackerValue = pieceValues[board[next]];
        // If we are going to do a promotion we need to correct the values a bit.
        if (toAtPromoRank && lastAttackerValue == pieceValues[Piece::Pawn])
        {
            materialGains[numberOfCaptures] += pieceValues[Piece::Queen] - pieceValues[Piece::Pawn];
            lastAttackerValue += pieceValues[Piece::Queen] - pieceValues[Piece::Pawn];
        }

        Bitboards::clearBit(occupied, next);
        attackers |= (Bitboards::rookAttacks(to, occupied) & (bitboards[Piece::WhiteQueen]
                                                            | bitboards[Piece::BlackQueen]
                                                            | bitboards[Piece::WhiteRook]
                                                            | bitboards[Piece::BlackRook]))
                   | (Bitboards::bishopAttacks(to, occupied) & (bitboards[Piece::WhiteQueen]
                                                              | bitboards[Piece::BlackQueen]
                                                              | bitboards[Piece::WhiteBishop]
                                                              | bitboards[Piece::BlackBishop]));
        attackers &= occupied;

        stm = !stm;
        if (getPieceType(board[next]) == Piece::King && (attackers & bitboards[12 + stm]))
        {
            break;
        }
        numberOfCaptures++;
    }

    while (--numberOfCaptures)
    {
        materialGains[numberOfCaptures - 1] = std::min(static_cast<int16_t>(-materialGains[numberOfCaptures]), materialGains[numberOfCaptures - 1]);
    }

    return materialGains[0];
}

bool Position::captureOrPromotion(const Move& move) const
{
    return getBoard(move.getTo()) != Piece::Empty || (move.getPromotion() != Piece::Empty && move.getPromotion() != Piece::King);
}

bool Position::verifyPsts() const
{
    auto correctPstScoreOp = 0, correctPstScoreEd = 0;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (board[sq] != Piece::Empty)
        {
            correctPstScoreOp += Evaluation::pieceSquareTableOpening[board[sq]][sq];
            correctPstScoreEd += Evaluation::pieceSquareTableEnding[board[sq]][sq];
        }
    }

    return ((pstScoreOp == correctPstScoreOp) && (pstScoreEd == correctPstScoreEd));
}

bool Position::verifyHashKeysAndPhase() const
{
    if (hashKey != calculateHash() || pawnHashKey != calculatePawnHash() && materialHashKey != calculateMaterialHash())
    {
        return false;
    }

    auto correctPhase = totalPhase;
    for (Piece p = Piece::Knight; p < Piece::King; ++p)
    {
        correctPhase -= (pieceCount[Color::White + p] + pieceCount[Color::Black * 6 + p]) * piecePhase[p];
    }

    return correctPhase == gamePhase;
}

bool Position::verifyPieceCounts() const
{
    std::array<int, 12> correctPieceCounts;
    correctPieceCounts.fill(0);
    auto correctTotalPieceCount = 0;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (board[sq] != Piece::Empty)
        {
            ++correctPieceCounts[board[sq]];
            ++correctTotalPieceCount;
        }
    }

    if (correctTotalPieceCount != totalPieceCount) 
        return false;

    for (Piece p = Piece::WhitePawn; p <= Piece::BlackKing; ++p)
    {
        if (pieceCount[p] != correctPieceCounts[p])
            return false;
    }

    return true;
}

bool Position::verifyBoardAndBitboards() const
{
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (board[sq] == Piece::Empty)
        {
            // Test that none of the bitboards has a bit set at the specified location.
            for (auto i = 0; i < 14; ++i)
            {
                if (Bitboards::testBit(bitboards[i], sq))
                    return false;
            }
        }
        else
        {
            // Check that the bitboard for that piece has the correct bit set.
            if (!Bitboards::testBit(bitboards[board[sq]], sq))
                return false;

            // Check that the bitboard for the side of the piece on the square has the correct piece set.
            if (!Bitboards::testBit(bitboards[12 + (board[sq] >= 6 ? 1 : 0)], sq))
                return false;

            // Check that the bitboard for the opposite side doesn't have a bit set.
            if (Bitboards::testBit(bitboards[12 + (board[sq] >= 6 ? 0 : 1)], sq))
                return false;

            // Check that all other bitboards don't have a bit set.
            for (auto i = 0; i < 12; ++i)
            {
                if (i == board[sq]) continue;
                if (Bitboards::testBit(bitboards[i], sq))
                    return false;
            }
        }
    }

    return true;
}

int16_t Position::mvvLva(const Move& move) const
{
    static const std::array<int16_t, 12> attackers = {
        5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0
    };

    static const std::array<int16_t, 13> victims = {
        1, 2, 2, 3, 4, 0, 1, 2, 2, 3, 4, 0, 0
    };

    const auto attackerValue = attackers[getBoard(move.getFrom())];
    const auto victimValue = static_cast<int16_t>(victims[getBoard(move.getTo())]
        + (move.getPromotion() != Piece::Empty ? victims[move.getPromotion()] -
        (move.getPromotion() != Piece::Pawn ? victims[Piece::Pawn] : 0)
        : 0));

    return victimValue * 8 + attackerValue;
}