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

#include "movegen.hpp"
#include <iostream>

void MoveGen::generatePseudoLegalMoves(const Position& pos, MoveList& moves)
{
    pos.getSideToMove() ? generatePseudoLegalMoves<true>(pos, moves) 
                        : generatePseudoLegalMoves<false>(pos, moves);
}

void MoveGen::generatePseudoLegalCaptures(const Position& pos, MoveList& moves, bool underPromotions)
{
    pos.getSideToMove() ? generatePseudoLegalCaptures<true>(pos, moves, underPromotions)
                        : generatePseudoLegalCaptures<false>(pos, moves, underPromotions);
}

void MoveGen::generateLegalEvasions(const Position& pos, MoveList& moves)
{
    pos.getSideToMove() ? generateLegalEvasions<true>(pos, moves)
                        : generateLegalEvasions<false>(pos, moves);
}

void MoveGen::generatePseudoLegalCapturesAndQuietChecks(const Position& pos, MoveList& moves)
{
    pos.getSideToMove() ? generatePseudoLegalCapturesAndQuietChecks<true>(pos, moves)
                        : generatePseudoLegalCapturesAndQuietChecks<false>(pos, moves);
}

void MoveGen::generatePseudoLegalQuietMoves(const Position& pos, MoveList& moves)
{
    pos.getSideToMove() ? generatePseudoLegalQuietMoves<true>(pos, moves)
                        : generatePseudoLegalQuietMoves<false>(pos, moves);
}

void addPieceMovesFromMask(MoveList& moveList, Bitboard mask, const Square from)
{
    while (mask)
    {
        const auto to = Bitboards::popLsb(mask);
        moveList.emplace_back(from, to, Piece::Empty, 0);
    }
}

template <bool side>
void addPawnSingleMovesFromMask(MoveList& moveList, Bitboard mask, bool underPromotions)
{
    while (mask)
    {
        const auto to = Bitboards::popLsb(mask);
        const auto from = to - 8 + side * 16;
        if (to >= Square::A8 || to <= Square::H1)
        {
            moveList.emplace_back(from, to, Piece::Queen, 0);
            if (underPromotions)
            {
                moveList.emplace_back(from, to, Piece::Rook, 0);
                moveList.emplace_back(from, to, Piece::Bishop, 0);
                moveList.emplace_back(from, to, Piece::Knight, 0);
            }
        }
        else
        {
            moveList.emplace_back(from, to, Piece::Empty, 0);
        }
    }
}

template <bool side>
void addPawnDoubleMovesFromMask(MoveList& moveList, Bitboard mask)
{
    while (mask)
    {
        const auto to = Bitboards::popLsb(mask);
        const auto from = to - 16 + side * 32;
        moveList.emplace_back(from, to, Piece::Empty, 0);
    }
}

template <bool side, bool rightCaptures>
void addPawnCapturesFromMask(MoveList& moveList, Bitboard mask, const Square ep, bool underPromotions)
{
    while (mask)
    {
        const auto to = Bitboards::popLsb(mask);
        const auto from = to - 7 - (2 * rightCaptures) + side * 16;
        if (to == ep)
        {
            moveList.emplace_back(from, to, Piece::Pawn, 0);
        }
        else if (to >= Square::A8 || to <= Square::H1)
        {
            moveList.emplace_back(from, to, Piece::Queen, 0);
            if (underPromotions)
            {
                moveList.emplace_back(from, to, Piece::Rook, 0);
                moveList.emplace_back(from, to, Piece::Bishop, 0);
                moveList.emplace_back(from, to, Piece::Knight, 0);
            }
        }
        else
        {
            moveList.emplace_back(from, to, Piece::Empty, 0);
        }
    }
}

template <bool side> 
void MoveGen::generatePseudoLegalMoves(const Position& pos, MoveList& moves)
{
    int from;
    const auto freeSquares = pos.getFreeSquares();
    const auto enemyPieces = pos.getPieces(!side);
    const auto targetBB = freeSquares | enemyPieces;
    const auto occupiedSquares = pos.getOccupiedSquares();

    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    auto m = (side ? tempPiece >> 8 : tempPiece << 8) & freeSquares;
    addPawnSingleMovesFromMask<side>(moves, m, true);

    m = (side ? (m & Bitboards::ranks[5]) >> 8 : (m & Bitboards::ranks[2]) << 8) & freeSquares;
    addPawnDoubleMovesFromMask<side>(moves, m);

    const auto ep = (pos.getEnPassantSquare() != Square::NoSquare ? Bitboards::bit(pos.getEnPassantSquare()) : 0);
    auto tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (enemyPieces | ep);
    addPawnCapturesFromMask<side, false>(moves, tempMove, pos.getEnPassantSquare(), true);

    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (enemyPieces | ep);
    addPawnCapturesFromMask<side, true>(moves, tempMove, pos.getEnPassantSquare(), true);

    // Since pinned knights do not have legal moves we can remove them.
    tempPiece = pos.getBitboard(side, Piece::Knight); // & ~pos.getPinnedPieces();
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & targetBB;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupiedSquares) & targetBB;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupiedSquares) & targetBB;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupiedSquares) & targetBB;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks(from) & targetBB;
    addPieceMovesFromMask(moves, tempMove, from);

    auto shortCastlingPossible = pos.getCastlingRights() & (1 << (2 * side)) && !(occupiedSquares & (0x0000000000000060ull << (56 * side)));
    auto longCastlingPossible = pos.getCastlingRights() & (2 << (2 * side)) && !(occupiedSquares & (0x000000000000000Eull << (56 * side)));
    if (shortCastlingPossible || longCastlingPossible)
    {
        if (!pos.isAttacked(Square::E1 + 56 * side, !side))
        {
            if (shortCastlingPossible) 
            {
                if (!(pos.isAttacked(Square::F1 + 56 * side, !side)) && !(pos.isAttacked(Square::G1 + 56 * side, !side)))
                {
                    moves.emplace_back(Square::E1 + 56 * side, Square::G1 + 56 * side, Piece::King, 0);
                }
            }
            if (longCastlingPossible)
            {
                if (!(pos.isAttacked(Square::D1 + 56 * side, !side)) && !(pos.isAttacked(Square::C1 + 56 * side, !side)))
                {
                    moves.emplace_back(Square::E1 + 56 * side, Square::C1 + 56 * side, Piece::King, 0);
                }
            }
        }
    }
}

template <bool side>
void MoveGen::generatePseudoLegalCaptures(const Position& pos, MoveList& moves, bool underPromotions)
{
    int from;
    const auto enemyPieces = pos.getPieces(!side);
    const auto occupiedSquares = pos.getOccupiedSquares();

    const auto ep = (pos.getEnPassantSquare() != Square::NoSquare ? Bitboards::bit(pos.getEnPassantSquare()) : 0);
    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    addPawnSingleMovesFromMask<side>(moves, (side ? (tempPiece & Bitboards::ranks[1]) >> 8 : (tempPiece & Bitboards::ranks[6]) << 8) & pos.getFreeSquares(), underPromotions);
    auto tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (enemyPieces | ep);
    addPawnCapturesFromMask<side, false>(moves, tempMove, pos.getEnPassantSquare(), underPromotions);

    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (enemyPieces | ep);
    addPawnCapturesFromMask<side, true>(moves, tempMove, pos.getEnPassantSquare(), underPromotions);

    tempPiece = pos.getBitboard(side, Piece::Knight);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & enemyPieces;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupiedSquares) & enemyPieces;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupiedSquares) & enemyPieces;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupiedSquares) & enemyPieces;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks(from) & enemyPieces;
    addPieceMovesFromMask(moves, tempMove, from);
}

template <bool side>
void MoveGen::generateLegalEvasions(const Position& pos, MoveList& moves)
{
    int from;
    auto occupied = pos.getOccupiedSquares();
    const auto freeSquares = pos.getFreeSquares();
    const auto targetBitboard = ~pos.getPieces(side);

    const auto kingLocation = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    auto tempMove = Bitboards::kingAttacks(kingLocation) & targetBitboard;
    // Remove our king from occupied so we can find out if squares "behind" our king are attacked.
    Bitboards::clearBit(occupied, kingLocation);
    while (tempMove)
    {
        const auto to = Bitboards::popLsb(tempMove);
        if (!pos.isAttacked(to, !side, occupied))
        {
            moves.emplace_back(kingLocation, to, Piece::Empty, 0);
        }
    }
    Bitboards::setBit(occupied, kingLocation); // And now put it back.

    const auto checkers = (Bitboards::rookAttacks(kingLocation, occupied) & (pos.getBitboard(!side, Piece::Queen) | pos.getBitboard(!side, Piece::Rook)))
        | (Bitboards::bishopAttacks(kingLocation, occupied) & (pos.getBitboard(!side, Piece::Queen) | pos.getBitboard(!side, Piece::Bishop)))
        | (Bitboards::knightAttacks(kingLocation) & pos.getBitboard(!side, Piece::Knight))
        | (Bitboards::kingAttacks(kingLocation) & pos.getBitboard(!side, Piece::King))
        | (Bitboards::pawnAttacks(side, kingLocation) & pos.getBitboard(!side, Piece::Pawn));
    // If we are checked by more than two pieces only the king can move and we are done.
    if (Bitboards::moreThanOneBitSet(checkers))
    {
        return;
    }
    const auto checkerLocation = Bitboards::lsb(checkers);
    const auto interpose = Bitboards::squaresBetween(kingLocation, checkerLocation) | checkers;
    const auto pinned = pos.getPinnedPieces();

    auto tempPiece = pos.getBitboard(side, Piece::Pawn) & ~pinned;
    auto m = (side ? tempPiece >> 8 : tempPiece << 8) & freeSquares;
    addPawnSingleMovesFromMask<side>(moves, m & interpose, true);

    m = (side ? (m & Bitboards::ranks[5]) >> 8 : (m & Bitboards::ranks[2]) << 8) & freeSquares & interpose;
    addPawnDoubleMovesFromMask<side>(moves, m);

    const auto ep = (pos.getEnPassantSquare() != Square::NoSquare && checkerLocation == static_cast<uint64_t>(pos.getEnPassantSquare() - 8 + side * 16)) ? Bitboards::bit(pos.getEnPassantSquare()) : 0;
    tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (checkers | ep);
    addPawnCapturesFromMask<side, false>(moves, tempMove, pos.getEnPassantSquare(), true);

    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (checkers | ep);
    addPawnCapturesFromMask<side, true>(moves, tempMove, pos.getEnPassantSquare(), true);

    tempPiece = pos.getBitboard(side, Piece::Knight) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & interpose;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Bishop) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupied) & interpose;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Rook) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupied) & interpose;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Queen) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupied) & interpose;
        addPieceMovesFromMask(moves, tempMove, from);
    }
}

template <bool side>
void MoveGen::generatePseudoLegalCapturesAndQuietChecks(const Position& pos, MoveList& moves)
{
    int from;
    auto occupied = pos.getOccupiedSquares();
    auto targetBitboard = ~pos.getPieces(side);
    auto opponentPieces = pos.getPieces(!side);
    auto opponentKingSquare = Bitboards::lsb(pos.getBitboard(!side, Piece::King));
    auto bishopCheckSquares = Bitboards::bishopAttacks(opponentKingSquare, occupied);
    auto rookCheckSquares = Bitboards::rookAttacks(opponentKingSquare, occupied);
    auto dcCandidates = pos.getDiscoveredCheckCandidates();

    const auto ep = (pos.getEnPassantSquare() != Square::NoSquare ? Bitboards::bit(pos.getEnPassantSquare()) : 0);
    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    auto tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (opponentPieces | ep);
    addPawnCapturesFromMask<side, false>(moves, tempMove, pos.getEnPassantSquare(), false);
    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (opponentPieces | ep);
    addPawnCapturesFromMask<side, true>(moves, tempMove, pos.getEnPassantSquare(), false);

    // A pawn push discovered check can be generated by a pawn only when the pawn is not on the same file as the opposing king.
    const auto promotionDiscoveredMask = (dcCandidates & ~Bitboards::files[file(opponentKingSquare)])
                                       | (side ? Bitboards::ranks[1] : Bitboards::ranks[6]);
    tempMove = (side ? (tempPiece & promotionDiscoveredMask) >> 8 : (tempPiece & promotionDiscoveredMask) << 8) & ~occupied;
    addPawnSingleMovesFromMask<side>(moves, tempMove, false);
    tempMove = (side ? (tempMove & Bitboards::ranks[5]) >> 8 : (tempMove & Bitboards::ranks[2]) << 8) & ~occupied;
    addPawnDoubleMovesFromMask<side>(moves, tempMove);

    tempMove = (side ? (tempPiece & ~promotionDiscoveredMask) >> 8 : (tempPiece & ~promotionDiscoveredMask) << 8) & ~occupied;
    addPawnSingleMovesFromMask<side>(moves, tempMove & Bitboards::pawnAttacks(!side, opponentKingSquare), false);
    tempMove = (side ? (tempMove & Bitboards::ranks[5]) >> 8 : (tempMove & Bitboards::ranks[2]) << 8) & ~occupied;
    addPawnDoubleMovesFromMask<side>(moves, tempMove & Bitboards::pawnAttacks(!side, opponentKingSquare));

    tempPiece = pos.getBitboard(side, Piece::Knight);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & targetBitboard;
        if (!Bitboards::testBit(dcCandidates, from))
        {
            tempMove &= opponentPieces | Bitboards::knightAttacks(opponentKingSquare);
        }
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupied) & targetBitboard;
        if (!Bitboards::testBit(dcCandidates, from))
        {
            tempMove &= opponentPieces | bishopCheckSquares;
        }
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupied) & targetBitboard;
        if (!Bitboards::testBit(dcCandidates, from))
        {
            tempMove &= opponentPieces | rookCheckSquares;
        }
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupied) & targetBitboard;
        if (!Bitboards::testBit(dcCandidates, from))
        {
            tempMove &= opponentPieces | bishopCheckSquares | rookCheckSquares;
        }
        addPieceMovesFromMask(moves, tempMove, from);
    }

    // TODO: add castling here someday.
    from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks(from)
             & (!Bitboards::testBit(dcCandidates, from) ? opponentPieces
                                                        : (targetBitboard & ~Bitboards::lineFormedBySquares(from, opponentKingSquare)));
    addPieceMovesFromMask(moves, tempMove, from);
}

template <bool side>
void MoveGen::generatePseudoLegalQuietMoves(const Position& pos, MoveList& moves)
{
    int from;
    Bitboard tempMove;
    const auto freeSquares = pos.getFreeSquares();
    const auto occupiedSquares = pos.getOccupiedSquares();

    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    auto m = (side ? tempPiece >> 8 : tempPiece << 8) & freeSquares & 0x00FFFFFFFFFFFF00;
    addPawnSingleMovesFromMask<side>(moves, m, true);

    m = (side ? (m & Bitboards::ranks[5]) >> 8 : (m & Bitboards::ranks[2]) << 8) & freeSquares;
    addPawnDoubleMovesFromMask<side>(moves, m);

    // Since pinned knights do not have legal moves we can remove them.
    tempPiece = pos.getBitboard(side, Piece::Knight); // & ~pos.getPinnedPieces();
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & freeSquares;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupiedSquares) & freeSquares;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupiedSquares) & freeSquares;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupiedSquares) & freeSquares;
        addPieceMovesFromMask(moves, tempMove, from);
    }

    from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks(from) & freeSquares;
    addPieceMovesFromMask(moves, tempMove, from);

    const auto shortCastlingPossible = pos.getCastlingRights() & (1 << (2 * side)) && !(occupiedSquares & (0x0000000000000060ull << (56 * side)));
    const auto longCastlingPossible = pos.getCastlingRights() & (2 << (2 * side)) && !(occupiedSquares & (0x000000000000000Eull << (56 * side)));
    if (shortCastlingPossible || longCastlingPossible)
    {
        if (!pos.isAttacked(Square::E1 + 56 * side, !side))
        {
            if (shortCastlingPossible)
            {
                if (!(pos.isAttacked(Square::F1 + 56 * side, !side)) && !(pos.isAttacked(Square::G1 + 56 * side, !side)))
                {
                    moves.emplace_back(Square::E1 + 56 * side, Square::G1 + 56 * side, Piece::King, 0);
                }
            }
            if (longCastlingPossible)
            {
                if (!(pos.isAttacked(Square::D1 + 56 * side, !side)) && !(pos.isAttacked(Square::C1 + 56 * side, !side)))
                {
                    moves.emplace_back(Square::E1 + 56 * side, Square::C1 + 56 * side, Piece::King, 0);
                }
            }
        }
    }
}