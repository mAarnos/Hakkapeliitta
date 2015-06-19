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

void addPieceMovesFromMask(MoveList& moveList, Bitboard mask, Square from)
{
    while (mask)
    {
        const auto to = Bitboards::popLsb(mask);
        moveList.emplace_back(from, to, Piece::Empty);
    }
}

void addPawnSingleMovesFromMask(MoveList& moveList, Bitboard mask, bool underPromotions, Color side)
{
    while (mask)
    {
        const auto to = Bitboards::popLsb(mask);
        const auto from = to - 8 + side * 16;
        if (to >= Square::A8 || to <= Square::H1)
        {
            moveList.emplace_back(from, to, Piece::Queen);
            if (underPromotions)
            {
                moveList.emplace_back(from, to, Piece::Rook);
                moveList.emplace_back(from, to, Piece::Bishop);
                moveList.emplace_back(from, to, Piece::Knight);
            }
        }
        else
        {
            moveList.emplace_back(from, to, Piece::Empty);
        }
    }
}

void addPawnDoubleMovesFromMask(MoveList& moveList, Bitboard mask, Color side)
{
    while (mask)
    {
        const auto to = Bitboards::popLsb(mask);
        const auto from = to - 16 + side * 32;
        moveList.emplace_back(from, to, Piece::Empty);
    }
}

template <bool rightCaptures>
void addPawnCapturesFromMask(MoveList& moveList, Bitboard mask, Square ep, bool underPromotions, Color side)
{
    while (mask)
    {
        const auto to = Bitboards::popLsb(mask);
        const auto from = to - 7 - (2 * rightCaptures) + side * 16;
        if (to == ep)
        {
            moveList.emplace_back(from, to, Piece::Pawn);
        }
        else if (to >= Square::A8 || to <= Square::H1)
        {
            moveList.emplace_back(from, to, Piece::Queen);
            if (underPromotions)
            {
                moveList.emplace_back(from, to, Piece::Rook);
                moveList.emplace_back(from, to, Piece::Bishop);
                moveList.emplace_back(from, to, Piece::Knight);
            }
        }
        else
        {
            moveList.emplace_back(from, to, Piece::Empty);
        }
    }
}

void MoveGen::generatePseudoLegalMoves(const Position& pos, MoveList& moveList)
{
    const auto side = pos.getSideToMove();
    const auto occupiedSquares = pos.getOccupiedSquares();
    const auto freeSquares = ~occupiedSquares;
    const auto enemyPieces = pos.getPieces(!side);
    const auto targetBB = freeSquares | enemyPieces;
    const auto ep = (pos.getEnPassantSquare() != Square::NoSquare ? Bitboards::bit(pos.getEnPassantSquare()) : 0);

    // Pawn moves.
    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    auto tempMove = (side ? tempPiece >> 8 : tempPiece << 8) & freeSquares;
    addPawnSingleMovesFromMask(moveList, tempMove, true, side);

    tempMove = (side ? (tempMove & Bitboards::ranks[5]) >> 8 : (tempMove & Bitboards::ranks[2]) << 8) & freeSquares;
    addPawnDoubleMovesFromMask(moveList, tempMove, side);

    tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (enemyPieces | ep);
    addPawnCapturesFromMask<false>(moveList, tempMove, pos.getEnPassantSquare(), true, side);

    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (enemyPieces | ep);
    addPawnCapturesFromMask<true>(moveList, tempMove, pos.getEnPassantSquare(), true, side);

    // King moves (without castling which is handled later).
    auto from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks(from) & targetBB;
    addPieceMovesFromMask(moveList, tempMove, from);

    // Knight moves.
    // Since pinned knights do not have legal moves we can remove them.
    tempPiece = pos.getBitboard(side, Piece::Knight); // & ~pos.getPinnedPieces();
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & targetBB;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Bishop moves.
    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupiedSquares) & targetBB;
        addPieceMovesFromMask(moveList, tempMove, from);
    }
    
    // Rook moves.
    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupiedSquares) & targetBB;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Queen moves.
    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupiedSquares) & targetBB;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Castling. We generate legal castling moves only so that making moves is easier.
    const auto shortCastlingPossible = pos.getCastlingRights() & (1 << (2 * side)) && !(occupiedSquares & (0x0000000000000060ULL << (56 * side)));
    const auto longCastlingPossible = pos.getCastlingRights() & (2 << (2 * side)) && !(occupiedSquares & (0x000000000000000EULL << (56 * side)));
    if (shortCastlingPossible || longCastlingPossible)
    {
        if (!pos.isAttacked(Square::E1 + 56 * side, !side))
        {
            if (shortCastlingPossible) 
            {
                if (!(pos.isAttacked(Square::F1 + 56 * side, !side)) && !(pos.isAttacked(Square::G1 + 56 * side, !side)))
                {
                    moveList.emplace_back(Square::E1 + 56 * side, Square::G1 + 56 * side, Piece::King);
                }
            }
            if (longCastlingPossible)
            {
                if (!(pos.isAttacked(Square::D1 + 56 * side, !side)) && !(pos.isAttacked(Square::C1 + 56 * side, !side)))
                {
                    moveList.emplace_back(Square::E1 + 56 * side, Square::C1 + 56 * side, Piece::King);
                }
            }
        }
    }
}

void MoveGen::generateLegalEvasions(const Position& pos, MoveList& moveList)
{
    assert(pos.inCheck());

    int from;
    const auto side = pos.getSideToMove();
    const auto occupied = pos.getOccupiedSquares();
    const auto freeSquares = pos.getFreeSquares();
    const auto targetBitboard = ~pos.getPieces(side);
    const auto kingLocation = Bitboards::lsb(pos.getBitboard(side, Piece::King));

    // King moves.
    auto tempMove = Bitboards::kingAttacks(kingLocation) & targetBitboard;
    while (tempMove)
    {
        const auto to = Bitboards::popLsb(tempMove);
        if (!pos.isAttacked(to, !side, occupied ^ Bitboards::bit(kingLocation)))
        {
            moveList.emplace_back(kingLocation, to, Piece::Empty);
        }
    }

    const auto checkers = (Bitboards::rookAttacks(kingLocation, occupied) & pos.getRooksAndQueens(!side))
                        | (Bitboards::bishopAttacks(kingLocation, occupied) & pos.getBishopsAndQueens(!side))
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
    const auto ep = (pos.getEnPassantSquare() != Square::NoSquare 
                 && checkerLocation == (pos.getEnPassantSquare() - 8u + side * 16u)) ? Bitboards::bit(pos.getEnPassantSquare()) : 0;

    // Pawn moves.
    auto tempPiece = pos.getBitboard(side, Piece::Pawn) & ~pinned;
    tempMove = (side ? tempPiece >> 8 : tempPiece << 8) & freeSquares;
    addPawnSingleMovesFromMask(moveList, tempMove & interpose, true, side);

    tempMove = (side ? (tempMove & Bitboards::ranks[5]) >> 8 : (tempMove & Bitboards::ranks[2]) << 8) & freeSquares & interpose;
    addPawnDoubleMovesFromMask(moveList, tempMove, side);

    tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (checkers | ep);
    addPawnCapturesFromMask<false>(moveList, tempMove, pos.getEnPassantSquare(), true, side);

    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (checkers | ep);
    addPawnCapturesFromMask<true>(moveList, tempMove, pos.getEnPassantSquare(), true, side);

    // Knight moves.
    tempPiece = pos.getBitboard(side, Piece::Knight) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & interpose;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Bishop moves.
    tempPiece = pos.getBitboard(side, Piece::Bishop) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupied) & interpose;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Rook moves.
    tempPiece = pos.getBitboard(side, Piece::Rook) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupied) & interpose;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Queen moves.
    tempPiece = pos.getBitboard(side, Piece::Queen) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupied) & interpose;
        addPieceMovesFromMask(moveList, tempMove, from);
    }
}

void MoveGen::generatePseudoLegalQuietMoves(const Position& pos, MoveList& moveList)
{
    const auto side = pos.getSideToMove();
    const auto freeSquares = pos.getFreeSquares();
    const auto occupiedSquares = pos.getOccupiedSquares();

    // Pawn moves.
    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    auto tempMove = (side ? tempPiece >> 8 : tempPiece << 8) & freeSquares & 0x00FFFFFFFFFFFF00;
    addPawnSingleMovesFromMask(moveList, tempMove, true, side);

    tempMove = (side ? (tempMove & Bitboards::ranks[5]) >> 8 : (tempMove & Bitboards::ranks[2]) << 8) & freeSquares;
    addPawnDoubleMovesFromMask(moveList, tempMove, side);

    // King moves. Castling is handled later for no reason.
    auto from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks(from) & freeSquares;
    addPieceMovesFromMask(moveList, tempMove, from);

    // Knight moves.
    // Since pinned knights do not have legal moves we can remove them.
    tempPiece = pos.getBitboard(side, Piece::Knight) & ~pos.getPinnedPieces();
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & freeSquares;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Bishop moves.
    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupiedSquares) & freeSquares;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Rook moves.
    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupiedSquares) & freeSquares;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Queen moves.
    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupiedSquares) & freeSquares;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Castling. We must check for full legality.
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
                    moveList.emplace_back(Square::E1 + 56 * side, Square::G1 + 56 * side, Piece::King);
                }
            }
            if (longCastlingPossible)
            {
                if (!(pos.isAttacked(Square::D1 + 56 * side, !side)) && !(pos.isAttacked(Square::C1 + 56 * side, !side)))
                {
                    moveList.emplace_back(Square::E1 + 56 * side, Square::C1 + 56 * side, Piece::King);
                }
            }
        }
    }
}

void MoveGen::generatePseudoLegalCapturesAndQuietChecks(const Position& pos, MoveList& moveList)
{
    const auto side = pos.getSideToMove();
    const auto occupied = pos.getOccupiedSquares();
    const auto targetBitboard = ~pos.getPieces(side);
    const auto opponentPieces = pos.getPieces(!side);
    const auto opponentKingSquare = Bitboards::lsb(pos.getBitboard(!side, Piece::King));
    const auto bishopCheckSquares = Bitboards::bishopAttacks(opponentKingSquare, occupied);
    const auto rookCheckSquares = Bitboards::rookAttacks(opponentKingSquare, occupied);
    const auto dcCandidates = pos.getDiscoveredCheckCandidates();
    const auto ep = (pos.getEnPassantSquare() != Square::NoSquare ? Bitboards::bit(pos.getEnPassantSquare()) : 0);

    // Pawn moves.
    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    auto tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (opponentPieces | ep);
    addPawnCapturesFromMask<false>(moveList, tempMove, pos.getEnPassantSquare(), false, side);
    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (opponentPieces | ep);
    addPawnCapturesFromMask<true>(moveList, tempMove, pos.getEnPassantSquare(), false, side);

    // A pawn push discovered check can be generated by a pawn only when the pawn is not on the same file as the opposing king.
    const auto promotionDiscoveredMask = (dcCandidates & ~Bitboards::files[file(opponentKingSquare)])
                                       | (side ? Bitboards::ranks[1] : Bitboards::ranks[6]);
    tempMove = (side ? (tempPiece & promotionDiscoveredMask) >> 8 : (tempPiece & promotionDiscoveredMask) << 8) & ~occupied;
    addPawnSingleMovesFromMask(moveList, tempMove, false, side);
    tempMove = (side ? (tempMove & Bitboards::ranks[5]) >> 8 : (tempMove & Bitboards::ranks[2]) << 8) & ~occupied;
    addPawnDoubleMovesFromMask(moveList, tempMove, side);

    tempMove = (side ? (tempPiece & ~promotionDiscoveredMask) >> 8 : (tempPiece & ~promotionDiscoveredMask) << 8) & ~occupied;
    addPawnSingleMovesFromMask(moveList, tempMove & Bitboards::pawnAttacks(!side, opponentKingSquare), false, side);
    tempMove = (side ? (tempMove & Bitboards::ranks[5]) >> 8 : (tempMove & Bitboards::ranks[2]) << 8) & ~occupied;
    addPawnDoubleMovesFromMask(moveList, tempMove & Bitboards::pawnAttacks(!side, opponentKingSquare), side);

    // King moves without castling.
    auto from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks(from)
             & (!Bitboards::testBit(dcCandidates, from) ? opponentPieces
                                                        : (targetBitboard & ~Bitboards::lineFormedBySquares(from, opponentKingSquare)));
    addPieceMovesFromMask(moveList, tempMove, from);

    // Knight moves.
    tempPiece = pos.getBitboard(side, Piece::Knight);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & targetBitboard;
        if (!Bitboards::testBit(dcCandidates, from))
        {
            tempMove &= opponentPieces | Bitboards::knightAttacks(opponentKingSquare);
        }
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Bishop moves.
    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupied) & targetBitboard;
        if (!Bitboards::testBit(dcCandidates, from))
        {
            tempMove &= opponentPieces | bishopCheckSquares;
        }
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Rook moves.
    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupied) & targetBitboard;
        if (!Bitboards::testBit(dcCandidates, from))
        {
            tempMove &= opponentPieces | rookCheckSquares;
        }
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Queen moves.
    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupied) & targetBitboard;
        if (!Bitboards::testBit(dcCandidates, from))
        {
            tempMove &= opponentPieces | bishopCheckSquares | rookCheckSquares;
        }
        addPieceMovesFromMask(moveList, tempMove, from);
    }
}

void MoveGen::generatePseudoLegalCaptures(const Position& pos, MoveList& moveList, bool underPromotions)
{
    const auto side = pos.getSideToMove();
    const auto enemyPieces = pos.getPieces(!side);
    const auto occupiedSquares = pos.getOccupiedSquares();
    const auto ep = (pos.getEnPassantSquare() != Square::NoSquare ? Bitboards::bit(pos.getEnPassantSquare()) : 0);

    // Pawn moves.
    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    auto tempMove = (side ? (tempPiece & Bitboards::ranks[1]) >> 8 : (tempPiece & Bitboards::ranks[6]) << 8) & pos.getFreeSquares();
    addPawnSingleMovesFromMask(moveList, tempMove, underPromotions, side);

    tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (enemyPieces | ep);
    addPawnCapturesFromMask<false>(moveList, tempMove, pos.getEnPassantSquare(), underPromotions, side);

    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (enemyPieces | ep);
    addPawnCapturesFromMask<true>(moveList, tempMove, pos.getEnPassantSquare(), underPromotions, side);

    // King moves.
    auto from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks(from) & enemyPieces;
    addPieceMovesFromMask(moveList, tempMove, from);

    // Knight moves.
    tempPiece = pos.getBitboard(side, Piece::Knight);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & enemyPieces;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Bishop moves.
    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupiedSquares) & enemyPieces;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Rook moves.
    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupiedSquares) & enemyPieces;
        addPieceMovesFromMask(moveList, tempMove, from);
    }

    // Queen moves.
    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupiedSquares) & enemyPieces;
        addPieceMovesFromMask(moveList, tempMove, from);
    }
}

