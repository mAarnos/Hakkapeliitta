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
#include <sstream>
#include "bitboards.hpp"
#include "zobrist.hpp"
#include "color.hpp"
#include "square.hpp"
#include "constants.hpp"
#include "evaluation.hpp"

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

Position::Position(const std::string& fen)
{
    // Split the FEN into parts.
    std::vector<std::string> strList;
    std::stringstream ss(fen);
    std::string item;
    while (getline(ss, item, ' ')) 
    {
        strList.push_back(item);
    }

    mBoard.fill(Piece::Empty);
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
        case 'p': mBoard[sq] = Piece::BlackPawn; break;
        case 'r': mBoard[sq] = Piece::BlackRook; break;
        case 'n': mBoard[sq] = Piece::BlackKnight; break;
        case 'b': mBoard[sq] = Piece::BlackBishop; break;
        case 'q': mBoard[sq] = Piece::BlackQueen; break;
        case 'k': mBoard[sq] = Piece::BlackKing; break;
        case 'P': mBoard[sq] = Piece::WhitePawn; break;
        case 'R': mBoard[sq] = Piece::WhiteRook; break;
        case 'N': mBoard[sq] = Piece::WhiteKnight; break;
        case 'B': mBoard[sq] = Piece::WhiteBishop; break;
        case 'Q': mBoard[sq] = Piece::WhiteQueen; break;
        case 'K': mBoard[sq] = Piece::WhiteKing; break;
        case '/': --j; break;
        case '1': break;
        case '2': ++j; break;
        case '3': j += 2; break;
        case '4': j += 3; break;
        case '5': j += 4; break;
        case '6': j += 5; break;
        case '7': j += 6; break;
        case '8': j += 7; break;
        default: return;
        }
        ++j;
    }

    // Set the turn.
    mSideToMove = Color::White;
    if (strList.size() >= 2)
    {
        if (strList[1] == "w")
        {
            mSideToMove = Color::White;
        }
        else if (strList[1] == "b")
        {
            mSideToMove = Color::Black;
        }
        else
        {
            return;
        }
    }

    // Set castling rights.
    mCastlingRights = 0;
    if (strList.size() >= 3)
    {
        if (strList[2].find('K') != std::string::npos)
        {
            mCastlingRights += CastlingRights::WhiteOO;
        }
        if (strList[2].find('Q') != std::string::npos)
        {
            mCastlingRights += CastlingRights::WhiteOOO;
        }
        if (strList[2].find('k') != std::string::npos)
        {
            mCastlingRights += CastlingRights::BlackOO;
        }
        if (strList[2].find('q') != std::string::npos)
        {
            mCastlingRights += CastlingRights::BlackOOO;
        }
    }

    // Set the en passant square, if any.
    mEnPassant = Square::NoSquare;
    if ((strList.size() >= 4) && (strList[3].length() >= 2))
    {
        if ((strList[3].at(0) >= 'a') && (strList[3].at(0) <= 'h') && ((strList[3].at(1) == '3') || (strList[3].at(1) == '6')))
        {
            const auto f = strList[3][0] - 96; // ASCII 'a' == 97 
            const auto r = strList[3][1] - 48; // ASCII '1' == 49 
            mEnPassant = ((r - 1) * 8 + f - 1);
        }
        else
        {
            return;
        }
    }

    // Set the fifty move distance.
    mFiftyMoveDistance = 0;
    if (strList.size() >= 5)
    {
        mFiftyMoveDistance = static_cast<int8_t>(std::stoi(strList[4]));
    }

    // Set the ply of the game.
    mGamePly = 0;
    if (strList.size() >= 6)
    {
        mGamePly = static_cast<short>(std::max(2 * stoi(strList[5]) - 1, 0));
        if (mSideToMove == Color::Black)
        {
            ++mGamePly;
        }
    }

    // Populate the bitboards.
    mBitboards.fill(0);
    mPieceCounts.fill(0);
    mPstScoreOp = mPstScoreEd = 0;
    mNonPawnPieceCounts.fill(0);
    for (Square sq = Square::A1; sq <= Square::H8; ++sq) 
    {
        if (mBoard[sq] != Piece::Empty)
        {
            Bitboards::setBit(mBitboards[mBoard[sq]], sq);
            mPstScoreOp += Evaluation::getPieceSquareTableOp(mBoard[sq], sq);
            mPstScoreEd += Evaluation::getPieceSquareTableEd(mBoard[sq], sq);
            ++mPieceCounts[mBoard[sq]];
            if (mBoard[sq].getPieceType() != Piece::Pawn && mBoard[sq].getPieceType() != Piece::King)
            {
                ++mNonPawnPieceCounts[mBoard[sq] >= Piece::BlackPawn];
            }
        }
    }

    mBitboards[12] = mBitboards[Piece::WhiteKing] | mBitboards[Piece::WhiteQueen] | mBitboards[Piece::WhiteRook] 
                   | mBitboards[Piece::WhiteBishop] | mBitboards[Piece::WhiteKnight] | mBitboards[Piece::WhitePawn];
    mBitboards[13] = mBitboards[Piece::BlackKing] | mBitboards[Piece::BlackQueen] | mBitboards[Piece::BlackRook] 
                  | mBitboards[Piece::BlackBishop] | mBitboards[Piece::BlackKnight] | mBitboards[Piece::BlackPawn];

    // Calculate all the different hash keys for the position.
    mHashKey = calculateHash();
    mPawnHashKey = calculatePawnHash();
    mMaterialHashKey = calculateMaterialHash();

    // Calculate pinned pieces and discovered check candidates.
    mPinned = pinnedPieces(mSideToMove);
    mDcCandidates = discoveredCheckCandidates();

    // Calculate the phase of the game.
    mGamePhase = totalPhase;
    for (Piece p = Piece::Knight; p < Piece::King; ++p)
    {
        mGamePhase -= (mPieceCounts[Color::White + p] + mPieceCounts[Color::Black * 6 + p]) * piecePhase[p];
    }
}

bool Position::isAttacked(Square sq, Color side) const
{
    return (side ? isAttacked<true>(sq, getOccupiedSquares()) : isAttacked<false>(sq, getOccupiedSquares()));
}

bool Position::isAttacked(Square sq, Color side, Bitboard occupied) const
{
    return (side ? isAttacked<true>(sq, occupied) : isAttacked<false>(sq, occupied));
}

void Position::makeMove(const Move& m)
{
    assert(pseudoLegal(m, inCheck()) && legal(m, inCheck()));

    const auto side = mSideToMove;
    const auto from = m.getFrom();
    const auto to = m.getTo();
    const auto flags = m.getFlags();
    const auto piece = mBoard[from];
    const auto captured = mBoard[to];
    const auto fromToBB = Bitboards::bit(from) | Bitboards::bit(to);

    // Update the PST score for the piece moving.
    mPstScoreOp += Evaluation::getPieceSquareTableOp(piece, to) 
                 - Evaluation::getPieceSquareTableOp(piece, from);
    mPstScoreEd += Evaluation::getPieceSquareTableEd(piece, to) 
                 - Evaluation::getPieceSquareTableEd(piece, from);

    // If there was an en passant move get rid of it and its hash key.
    if (mEnPassant != Square::NoSquare)
    {
        mHashKey ^= Zobrist::enPassantHashKey(mEnPassant);
        mEnPassant = Square::NoSquare;
    }

    mBoard[to] = mBoard[from];
    mBoard[from] = Piece::Empty;
    mBitboards[piece] ^= fromToBB;
    mBitboards[12 + side] ^= fromToBB;
    ++mFiftyMoveDistance;
    mHashKey ^= Zobrist::pieceHashKey(piece, from) ^ Zobrist::pieceHashKey(piece, to);

    if (captured != Piece::Empty)
    {
        Bitboards::clearBit(mBitboards[captured], to);
        Bitboards::clearBit(mBitboards[12 + !side], to);
        mFiftyMoveDistance = 0;
        mPstScoreOp -= Evaluation::getPieceSquareTableOp(captured, to);
        mPstScoreEd -= Evaluation::getPieceSquareTableEd(captured, to);
        mHashKey ^= Zobrist::pieceHashKey(captured, to);
        mMaterialHashKey ^= Zobrist::materialHashKey(captured, --mPieceCounts[captured]);

        const auto pieceType = captured.getPieceType();
        mGamePhase += piecePhase[pieceType];
        if (pieceType == Piece::Pawn)
        {
            mPawnHashKey ^= Zobrist::pieceHashKey(captured, to);
        }
        else
        {
            --mNonPawnPieceCounts[!side];
        }
    }

    if (piece.getPieceType() == Piece::Pawn)
    {
        mFiftyMoveDistance = 0;
        mPawnHashKey ^= Zobrist::pieceHashKey(piece, from) ^ Zobrist::pieceHashKey(piece, to);

        if ((to ^ from) == 16) // Double pawn move
        {
            mEnPassant = from ^ 24;
            mHashKey ^= Zobrist::enPassantHashKey(mEnPassant);
        }
        else if (flags == Piece::Pawn) // En passant
        {
            const auto enPassantSquare = to ^ 8;
            Bitboards::clearBit(mBitboards[Piece::Pawn + !side * 6], enPassantSquare);
            Bitboards::clearBit(mBitboards[12 + !side], enPassantSquare);
            mBoard[enPassantSquare] = Piece::Empty;
            mHashKey ^= Zobrist::pieceHashKey(Piece::Pawn + !side * 6, enPassantSquare);
            mPawnHashKey ^= Zobrist::pieceHashKey(Piece::Pawn + !side * 6, enPassantSquare);
            mMaterialHashKey ^= Zobrist::materialHashKey(Piece::Pawn + !side * 6, --mPieceCounts[Piece::Pawn + !side * 6]);
            mPstScoreOp -= Evaluation::getPieceSquareTableOp(Piece::Pawn + !side * 6, enPassantSquare);
            mPstScoreEd -= Evaluation::getPieceSquareTableEd(Piece::Pawn + !side * 6, enPassantSquare);
        }
        else if (flags != Piece::Empty) // Promotion
        {
            // This needs to be above the rest due to reasons. Try to fix that.
            mMaterialHashKey ^= Zobrist::materialHashKey(flags + side * 6, mPieceCounts[flags + side * 6]++);
            Bitboards::clearBit(mBitboards[Piece::Pawn + side * 6], to);
            Bitboards::setBit(mBitboards[flags + side * 6], to);
            mBoard[to] = flags + side * 6;
            mHashKey ^= Zobrist::pieceHashKey(Piece::Pawn + side * 6, to) ^ Zobrist::pieceHashKey(flags + side * 6, to);
            mPawnHashKey ^= Zobrist::pieceHashKey(Piece::Pawn + side * 6, to);
            mMaterialHashKey ^= Zobrist::materialHashKey(Piece::Pawn + side * 6, --mPieceCounts[Piece::Pawn + side * 6]);
            mGamePhase -= piecePhase[flags];
            ++mNonPawnPieceCounts[side];
            mPstScoreOp += Evaluation::getPieceSquareTableOp(flags + side * 6, to) 
                         - Evaluation::getPieceSquareTableOp(Piece::Pawn + side * 6, to);
            mPstScoreEd += Evaluation::getPieceSquareTableEd(flags + side * 6, to) 
                         - Evaluation::getPieceSquareTableEd(Piece::Pawn + side * 6, to);
        }
    }
    else if (flags == Piece::King)
    {
        const auto fromRook = (from > to ? (to - 2) : (to + 1));
        const auto toRook = (from + to) / 2;
        const auto fromToBBCastling = Bitboards::bit(fromRook) | Bitboards::bit(toRook);

        mBitboards[Piece::Rook + side * 6] ^= fromToBBCastling;
        mBitboards[12 + side] ^= fromToBBCastling;
        mBoard[toRook] = mBoard[fromRook];
        mBoard[fromRook] = Piece::Empty;
        mHashKey ^= Zobrist::pieceHashKey(Piece::Rook + side * 6, fromRook) 
                  ^ Zobrist::pieceHashKey(Piece::Rook + side * 6, toRook);
        mPstScoreOp += Evaluation::getPieceSquareTableOp(Piece::Rook + side * 6, toRook) 
                     - Evaluation::getPieceSquareTableOp(Piece::Rook + side * 6, fromRook);
        mPstScoreEd += Evaluation::getPieceSquareTableEd(Piece::Rook + side * 6, toRook) 
                     - Evaluation::getPieceSquareTableEd(Piece::Rook + side * 6, fromRook);
    }

    mPinned = pinnedPieces(!side);
    mSideToMove = !side;
    mDcCandidates = discoveredCheckCandidates();
    mHashKey ^= Zobrist::turnHashKey();
    ++mGamePly;

    // Update castling rights if needed
    if (mCastlingRights && (castlingMask[from] | castlingMask[to]))
    {
        const auto cf = castlingMask[from] | castlingMask[to];
        mHashKey ^= Zobrist::castlingRightsHashKey(mCastlingRights & cf);
        mCastlingRights &= ~cf;
    }

    assert(verifyPsts());
    assert(verifyHashKeysAndPhase());
    assert(verifyPieceCounts());
    assert(verifyBoardAndBitboards());
}

void Position::makeNullMove()
{
    mSideToMove = !mSideToMove;
    mHashKey ^= Zobrist::turnHashKey();
    if (mEnPassant != Square::NoSquare)
    {
        mHashKey ^= Zobrist::enPassantHashKey(mEnPassant);
        mEnPassant = Square::NoSquare;
    }
    ++mFiftyMoveDistance;
    mPinned = pinnedPieces(mSideToMove);
    mDcCandidates = discoveredCheckCandidates();
}

template <bool side>
bool Position::isAttacked(Square sq, Bitboard occupied) const
{
    return (Bitboards::knightAttacks(sq) & getBitboard(side, Piece::Knight)
        || Bitboards::pawnAttacks(!side, sq) & getBitboard(side, Piece::Pawn)
        || Bitboards::bishopAttacks(sq, occupied) & (getBitboard(side, Piece::Bishop) | getBitboard(side, Piece::Queen))
        || Bitboards::rookAttacks(sq, occupied) & (getBitboard(side, Piece::Rook) | getBitboard(side, Piece::Queen))
        || Bitboards::kingAttacks(sq) & getBitboard(side, Piece::King));
}

Bitboard Position::checkBlockers(Color c, Color kingColor) const
{
    assert(c.isOk() && kingColor.isOk());

    auto result = 0ULL;
    const auto kingSquare = Bitboards::lsb(getBitboard(kingColor, Piece::King));
    const auto rq = getRooksAndQueens(!kingColor);
    const auto bq = getBishopsAndQueens(!kingColor);
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
    const auto flags = move.getFlags();

    // Check that the piece to be moved is the same color as the side to move.
    if (!Bitboards::testBit(getPieces(mSideToMove), from))
    {
        return false;
    }

    // Destination square must not be occupied by a friendly piece.
    // Note that this also catches null moves as they have the same from and to squares.
    if (Bitboards::testBit(getPieces(mSideToMove), to))
    {
        return false;
    }

    const auto pieceType = mBoard[from].getPieceType();
    // Make sure that only pawn moves can have flags which are only for pawn moves.
    if (pieceType != Piece::Pawn && ((flags >= Piece::Pawn && flags <= Piece::Queen)))
    {
        return false;
    }

    // Make sure that only king moves can have flags which are only for king moves.
    if (pieceType != Piece::King && flags == Piece::King)
    {
        return false;
    }

    // Make sure that the promotion flags are within the specified limits.
    if (!(flags >= Piece::Pawn && flags <= Piece::King) && flags != Piece::Empty)
    {
        return false;
    }

    // Check if the moving piece can actually move to the given location. Pawns need special handling.
    if (pieceType == Piece::Pawn)
    {
        // Check that the possible en passant flags are set correctly.
        if ((to == mEnPassant) == (flags != Piece::Pawn))
        {
            return false;
        }
        // Check that the possible promotion flags are set correctly.
        else if ((mSideToMove ? to <= Square::H1 : to >= Square::A8) == (flags == Piece::Pawn || flags == Piece::Empty))
        {
            return false;
        }

        const auto pawnMoveUp = (mSideToMove ? -8 : 8);
        if (!(Bitboards::pawnAttacks(mSideToMove, from) & Bitboards::bit(to) & (getPieces(!mSideToMove) | (mEnPassant != Square::NoSquare ? Bitboards::bit(mEnPassant) : 0))) // Not a capture, ep or non-ep
            && !(mBoard[to] == Piece::Empty && from + pawnMoveUp == to) // Not a single pawn move
            && !(from + 2 * pawnMoveUp == to && mBoard[to] == Piece::Empty && mBoard[to - pawnMoveUp] == Piece::Empty && (mSideToMove ? 7 - rank(from) : rank(from)) == 1)) // Not a double pawn move.
        {
            return false;
        }
    }
    else
    {
        if (pieceType == Piece::King && flags == Piece::King)
        {
            // If we are in check castling is not legal.
            if (inCheck)
            {
                return false;
            }

            if (to == (Square::C1 + 56 * mSideToMove))
            {
                // Make sure that long castling is legal, disregarding checks.
                if (!(getCastlingRights() & (2 << (2 * mSideToMove)) && !(getOccupiedSquares() & (0x000000000000000Eull << (56 * mSideToMove)))))
                {
                    return false;
                }

                // Make sure we don't move through a check or into check.
                if (isAttacked(Square::D1 + 56 * mSideToMove, !mSideToMove) || isAttacked(Square::C1 + 56 * mSideToMove, !mSideToMove))
                {
                    return false;
                }
            }
            else if (to == (Square::G1 + 56 * mSideToMove))
            {
                // Make sure that short castling is legal, disregarding checks.
                if (!(getCastlingRights() & (1 << (2 * mSideToMove)) && !(getOccupiedSquares() & (0x0000000000000060ull << (56 * mSideToMove)))))
                {
                    return false;
                }

                // Make sure we don't move through a check or into check.
                if (isAttacked(Square::F1 + 56 * mSideToMove, !mSideToMove) || isAttacked(Square::G1 + 56 * mSideToMove, !mSideToMove))
                {
                    return false;
                }
            }
            else // The move is not long or short castling -> it can't be legal
            {
                return false;
            }
        }
        else if (!Bitboards::testBit(Bitboards::pieceAttacks(mSideToMove, pieceType, from, getOccupiedSquares()), to))
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
            if (isAttacked(to, !mSideToMove, getOccupiedSquares() ^ Bitboards::bit(from)))
            {
                return false;
            }
        }
        else
        {
            const auto toFrom = Bitboards::bit(from) | Bitboards::bit(to);
            const auto newOccupied = getOccupiedSquares() ^ toFrom ^ (mBoard[to] != Piece::Empty ? Bitboards::bit(to) : 0);
            const auto kingSquare = Bitboards::lsb(getBitboard(mSideToMove, Piece::King));
            const auto exclusion = ~(Bitboards::bit(to) | (flags == Piece::Pawn ? Bitboards::bit(to + (mSideToMove ? 8 : -8)) : 0));
            // Check if the move stops our king from being in check.
            if (((Bitboards::knightAttacks(kingSquare) & getBitboard(!mSideToMove, Piece::Knight) & exclusion
                || Bitboards::pawnAttacks(mSideToMove, kingSquare) & getBitboard(!mSideToMove, Piece::Pawn) & exclusion
                || Bitboards::bishopAttacks(kingSquare, newOccupied) & getBishopsAndQueens(!mSideToMove) & exclusion
                || Bitboards::rookAttacks(kingSquare, newOccupied) & getRooksAndQueens(!mSideToMove) & exclusion
                || Bitboards::kingAttacks(kingSquare) & getBitboard(!mSideToMove, Piece::King) & exclusion)))
            {
                return false;
            }
        }
    }

    return true;
}


bool Position::legal(const Move& move, bool inCheck) const
{
    if (inCheck) return true; // As said before, we assume that when in check we generate legal evasions so legality checking is useless.

    const auto from = move.getFrom();
    const auto to = move.getTo();

    if (move.getFlags() == Piece::Pawn)
    {
        const auto kingSquare = Bitboards::lsb(getBitboard(mSideToMove, Piece::King));
        const auto captureSquare = to ^ 8;
        const auto occupied = (getOccupiedSquares() ^ Bitboards::bit(from) ^ Bitboards::bit(captureSquare)) | Bitboards::bit(to);
        const auto rq = getRooksAndQueens(!mSideToMove);
        const auto bq = getBishopsAndQueens(!mSideToMove);
        return !(Bitboards::bishopAttacks(kingSquare, occupied) & bq) && !(Bitboards::rookAttacks(kingSquare, occupied) & rq);
    }

    if (mBoard[from].getPieceType() == Piece::King)
    {
        // Castling is checked for legality in move generation.
        // Otherwise a king move is legal if the target square is not attacked.
        return move.getFlags() == Piece::King || !isAttacked(to, !mSideToMove);
    }

    // Otherwise a move is legal if it is not pinned or it is moving along the ray towards or away from the king.
    return !Bitboards::testBit(mPinned, from) 
         || Bitboards::testBit(Bitboards::lineFormedBySquares(from, to), Bitboards::lsb(getBitboard(mSideToMove, Piece::King)));
}

int Position::givesCheck(const Move& move) const
{
    const auto from = move.getFrom();
    const auto to = move.getTo();
    const auto flags = move.getFlags();
    const auto kingSquare = Bitboards::lsb(getBitboard(!mSideToMove, Piece::King)); 
    auto occupied = getOccupiedSquares();

    // Test for discovered check.
    if (Bitboards::testBit(mDcCandidates, from) && !Bitboards::testBit(Bitboards::lineFormedBySquares(from, kingSquare), to))
    {
        return 2; 
    }

    // Test for direct checks.
    if (flags == Piece::Empty || flags == Piece::Pawn)
    {
        const auto piece = mBoard[from].getPieceType();
        // A king cannot give check so we can forget about that case. For the same reason the case flags == Piece::King was unnecessary.
        if (piece != Piece::King)
        {
            const auto attacks = Bitboards::pieceAttacks(mSideToMove, piece, to, occupied);
            if (Bitboards::testBit(attacks, kingSquare))
                return 1;
        }
    }

    if (flags == Piece::King)
    {
        const auto rookTo = (from + to) / 2;
        const auto rq = getRooksAndQueens(mSideToMove) ^ Bitboards::bit(rookTo);
        Bitboards::clearBit(occupied, from);
        if (rq & Bitboards::rookAttacks(kingSquare, occupied))
            return 1; // Check by the castled rook
    }
    else if (flags == Piece::Pawn) 
    {
        const auto rq = getRooksAndQueens(mSideToMove);
        const auto bq = getBishopsAndQueens(mSideToMove);
        Bitboards::clearBit(occupied, from); 
        Bitboards::clearBit(occupied, to ^ 8);
        Bitboards::setBit(occupied, to);
        if ((rq & Bitboards::rookAttacks(kingSquare, occupied)) || (bq & Bitboards::bishopAttacks(kingSquare, occupied)))
            return 2; // Discovered check through the square we left from or the captured pawn.
    }
    else if (flags != Piece::Empty)
    {
        Bitboards::clearBit(occupied, from);
        if (Bitboards::testBit(Bitboards::pieceAttacks(mSideToMove, flags, to, occupied), kingSquare))
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

    std::array<int16_t, 32> materialGains;
    auto occupied = getOccupiedSquares();
    const auto from = move.getFrom();
    const auto to = move.getTo();
    const auto flags = move.getFlags();
    const auto toAtPromoRank = (to <= 7 || to >= 56);
    auto stm = mSideToMove;
    int16_t lastAttackerValue;
    int next;

    if (flags == Piece::King)
    {
        return 0;
    }
    else if (flags == Piece::Pawn)
    {
        materialGains[0] = pieceValues[Piece::Pawn];
        lastAttackerValue = pieceValues[Piece::Pawn];
        Bitboards::clearBit(occupied, mEnPassant ^ 8);
    }
    else
    {
        materialGains[0] = pieceValues[mBoard[to]];
        lastAttackerValue = pieceValues[mBoard[from]];
        if (flags != Piece::Empty)
        {
            materialGains[0] += pieceValues[flags] - pieceValues[Piece::Pawn];
            lastAttackerValue += pieceValues[flags] - pieceValues[Piece::Pawn];
        }
    }

    Bitboards::clearBit(occupied, from);
    auto attackers = (Bitboards::rookAttacks(to, occupied) & getRooksAndQueens())
                   | (Bitboards::bishopAttacks(to, occupied) & getBishopsAndQueens())
                   | (Bitboards::knightAttacks(to) & (mBitboards[Piece::WhiteKnight] | mBitboards[Piece::BlackKnight]))
                   | (Bitboards::kingAttacks(to) & (mBitboards[Piece::WhiteKing] | mBitboards[Piece::BlackKing]))
                   | (Bitboards::pawnAttacks(Color::Black, to) & (mBitboards[Piece::WhitePawn]))
                   | (Bitboards::pawnAttacks(Color::White, to) & (mBitboards[Piece::BlackPawn]));
    attackers &= occupied;
    stm = !stm;
    auto numberOfCaptures = 1;

    while (attackers & mBitboards[12 + stm])
    {
        if (getBitboard(stm, Piece::Pawn) & attackers)
        {
            next = Bitboards::lsb(getBitboard(stm, Piece::Pawn) & attackers);
        }
        else if (getBitboard(stm, Piece::Knight) & attackers)
        {
            next = Bitboards::lsb(getBitboard(stm, Piece::Knight) & attackers);
        }
        else if (getBitboard(stm, Piece::Bishop) & attackers)
        {
            next = Bitboards::lsb(mBitboards[Piece::Bishop + stm * 6] & attackers);
        }
        else if (getBitboard(stm, Piece::Rook) & attackers)
        {
            next = Bitboards::lsb(getBitboard(stm, Piece::Rook) & attackers);
        }
        else if (getBitboard(stm, Piece::Queen) & attackers)
        {
            next = Bitboards::lsb(getBitboard(stm, Piece::Queen) & attackers);
        }
        else
        {
            next = Bitboards::lsb(getBitboard(stm, Piece::King));
        }

        // Update the materialgains array.
        materialGains[numberOfCaptures] = -materialGains[numberOfCaptures - 1] + lastAttackerValue;
        // Remember the value of the capturing piece because it is going to be captured next.
        lastAttackerValue = pieceValues[mBoard[next]];
        // If we are going to do a promotion we need to correct the values a bit.
        if (toAtPromoRank && lastAttackerValue == pieceValues[Piece::Pawn])
        {
            materialGains[numberOfCaptures] += pieceValues[Piece::Queen] - pieceValues[Piece::Pawn];
            lastAttackerValue += pieceValues[Piece::Queen] - pieceValues[Piece::Pawn];
        }

        Bitboards::clearBit(occupied, next);
        attackers |= (Bitboards::rookAttacks(to, occupied) & getRooksAndQueens())
                   | (Bitboards::bishopAttacks(to, occupied) & getBishopsAndQueens());
        attackers &= occupied;

        stm = !stm;
        if (mBoard[next].getPieceType() == Piece::King && (attackers & mBitboards[12 + stm]))
        {
            break;
        }
        numberOfCaptures++;
    }

    while (--numberOfCaptures)
    {
        materialGains[numberOfCaptures - 1] = std::min(static_cast<int16_t>(-materialGains[numberOfCaptures]), 
                                                       materialGains[numberOfCaptures - 1]);
    }

    return materialGains[0];
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

    auto victimValue = victims[getBoard(move.getTo())];
    if (move.getFlags() != Piece::Empty)
    {
        victimValue += victims[move.getFlags()];
        if (move.getFlags() != Piece::Pawn)
        {
            victimValue -= victims[Piece::Pawn];
        }
    }

    return victimValue * 8 + attackerValue;
}

bool Position::captureOrPromotion(const Move& move) const
{
    return getBoard(move.getTo()) != Piece::Empty || (move.getFlags() != Piece::Empty && move.getFlags() != Piece::King);
}

HashKey Position::calculateHash() const
{
    auto h = 0ULL;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (mBoard[sq] != Piece::Empty)
        {
            h ^= Zobrist::pieceHashKey(mBoard[sq], sq);
        }
    }

    if (mEnPassant != Square::NoSquare)
    {
        h ^= Zobrist::enPassantHashKey(mEnPassant);
    }
    if (mSideToMove)
    {
        h ^= Zobrist::turnHashKey();
    }
    if (mCastlingRights)
    {
        h ^= Zobrist::castlingRightsHashKey(mCastlingRights);
    }

    return h;
}

HashKey Position::calculatePawnHash() const
{
    auto p = 0ULL;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (mBoard[sq] == Piece::WhitePawn || mBoard[sq] == Piece::BlackPawn)
        {
            p ^= Zobrist::pieceHashKey(mBoard[sq], sq);
        }
    }

    return p;
}

HashKey Position::calculateMaterialHash() const
{
    auto m = 0ULL;

    for (Piece p = Piece::WhitePawn; p <= Piece::BlackKing; ++p)
    {
        for (auto i = 0; i < mPieceCounts[p]; ++i)
        {
            m ^= Zobrist::materialHashKey(p, i);
        }
    }

    return m;
}

bool Position::verifyPsts() const
{
    auto correctPstScoreOp = 0, correctPstScoreEd = 0;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (mBoard[sq] != Piece::Empty)
        {
            correctPstScoreOp += Evaluation::getPieceSquareTableOp(mBoard[sq], sq);
            correctPstScoreEd += Evaluation::getPieceSquareTableEd(mBoard[sq], sq);
        }
    }

    return ((mPstScoreOp == correctPstScoreOp) && (mPstScoreEd == correctPstScoreEd));
}

bool Position::verifyHashKeysAndPhase() const
{
    if (mHashKey != calculateHash() || mPawnHashKey != calculatePawnHash() || mMaterialHashKey != calculateMaterialHash())
    {
        return false;
    }

    auto correctPhase = totalPhase;
    for (Piece p = Piece::Knight; p < Piece::King; ++p)
    {
        correctPhase -= (mPieceCounts[Color::White + p] + mPieceCounts[Color::Black * 6 + p]) * piecePhase[p];
    }

    return correctPhase == mGamePhase;
}

bool Position::verifyPieceCounts() const
{
    std::array<int, 12> correctPieceCounts;
    correctPieceCounts.fill(0);
    auto correctTotalPieceCount = 0;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (mBoard[sq] != Piece::Empty)
        {
            ++correctPieceCounts[mBoard[sq]];
            ++correctTotalPieceCount;
        }
    }

    for (Piece p = Piece::WhitePawn; p <= Piece::BlackKing; ++p)
    {
        if (mPieceCounts[p] != correctPieceCounts[p])
            return false;
    }

    const auto correctWhiteNonPawnPieceCount = (mPieceCounts[Piece::WhiteKnight]
                                             + mPieceCounts[Piece::WhiteBishop]
                                             + mPieceCounts[Piece::WhiteRook]
                                             + mPieceCounts[Piece::WhiteQueen]);

    const auto correctBlackNonPawnPieceCount = (mPieceCounts[Piece::BlackKnight]
                                             + mPieceCounts[Piece::BlackBishop]
                                             + mPieceCounts[Piece::BlackRook]
                                             + mPieceCounts[Piece::BlackQueen]);

    if (correctWhiteNonPawnPieceCount != mNonPawnPieceCounts[Color::White] ||
        correctBlackNonPawnPieceCount != mNonPawnPieceCounts[Color::Black])
    {
        return false;
    }

    return true;
}

bool Position::verifyBoardAndBitboards() const
{
    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (mBoard[sq] == Piece::Empty)
        {
            // Test that none of the bitboards has a bit set at the specified location.
            for (auto i = 0; i < 14; ++i)
            {
                if (Bitboards::testBit(mBitboards[i], sq))
                    return false;
            }
        }
        else
        {
            // Check that the bitboard for that piece has the correct bit set.
            if (!Bitboards::testBit(mBitboards[mBoard[sq]], sq))
                return false;

            // Check that the bitboard for the side of the piece on the square has the correct piece set.
            if (!Bitboards::testBit(mBitboards[12 + (mBoard[sq] >= 6 ? 1 : 0)], sq))
                return false;

            // Check that the bitboard for the opposite side doesn't have a bit set.
            if (Bitboards::testBit(mBitboards[12 + (mBoard[sq] >= 6 ? 0 : 1)], sq))
                return false;

            // Check that all other bitboards don't have a bit set.
            for (auto i = 0; i < 12; ++i)
            {
                if (i == mBoard[sq]) continue;
                if (Bitboards::testBit(mBitboards[i], sq))
                    return false;
            }
        }
    }

    return true;
}
