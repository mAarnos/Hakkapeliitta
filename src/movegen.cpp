#include "movegen.hpp"
#include <iostream>

void MoveGen::generatePseudoLegalMoves(const Position& pos, MoveList& moves)
{
    pos.getSideToMove() ? generatePseudoLegalMoves<true>(pos, moves) 
                        : generatePseudoLegalMoves<false>(pos, moves);
}

void MoveGen::generateLegalEvasions(const Position& pos, MoveList& moves)
{
    pos.getSideToMove() ? generateLegalEvasions<true>(pos, moves)
                        : generateLegalEvasions<false>(pos, moves);
}

void addMovesFromMask(MoveList& moveList, Bitboard mask, Square from)
{
    while (mask)
    {
        auto to = Bitboards::popLsb(mask);
        moveList.push_back(Move(from, to, Piece::Empty, 0));
    }
}

template <bool side> 
void MoveGen::generatePseudoLegalMoves(const Position& pos, MoveList& moves)
{
    assert(moves.empty());

    int from, to;
    auto freeSquares = pos.getFreeSquares();
    auto enemyPieces = pos.getPieces(!side);
    auto targetBB = freeSquares | enemyPieces;
    auto occupiedSquares = pos.getOccupiedSquares();

    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    auto m = (side ? tempPiece >> 8 : tempPiece << 8) & freeSquares;
    auto tempMove = m;
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        from = to - 8 + side * 16;
        if (to >= Square::A8 || to <= Square::H1)
        {
            moves.push_back(Move(from, to, Piece::Queen, 0));
            moves.push_back(Move(from, to, Piece::Rook, 0));
            moves.push_back(Move(from, to, Piece::Bishop, 0));
            moves.push_back(Move(from, to, Piece::Knight, 0));
        }
        else
        {
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }
    tempMove = (side ? (m & Bitboards::ranks[5]) >> 8 : (m & Bitboards::ranks[2]) << 8) & freeSquares;
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        from = to - 16 + side * 32;
        moves.push_back(Move(from, to, Piece::Empty, 0));
    }

    auto ep = (pos.getEnPassantSquare() != Square::NoSquare ? Bitboards::bit(pos.getEnPassantSquare()) : 0);
    tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (enemyPieces | ep);
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        from = to - 7 + side * 16;
        if (to == pos.getEnPassantSquare())
        {
            moves.push_back(Move(from, to, Piece::Pawn, 0));
        }
        else if (to >= Square::A8 || to <= Square::H1)
        {
            moves.push_back(Move(from, to, Piece::Queen, 0));
            moves.push_back(Move(from, to, Piece::Rook, 0));
            moves.push_back(Move(from, to, Piece::Bishop, 0));
            moves.push_back(Move(from, to, Piece::Knight, 0));
        }
        else
        {
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (enemyPieces | ep);
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        from = to - 9 + side * 16;
        if (to == pos.getEnPassantSquare())
        {
            moves.push_back(Move(from, to, Piece::Pawn, 0));
        }
        else if (to >= Square::A8 || to <= Square::H1)
        {
            moves.push_back(Move(from, to, Piece::Queen, 0));
            moves.push_back(Move(from, to, Piece::Rook, 0));
            moves.push_back(Move(from, to, Piece::Bishop, 0));
            moves.push_back(Move(from, to, Piece::Knight, 0));
        }
        else
        {
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    // Since pinned pieces do not have legal moves we can remove them.
    tempPiece = pos.getBitboard(side, Piece::Knight) & ~pos.getPinnedPieces();
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & targetBB;
        addMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupiedSquares) & targetBB;
        addMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupiedSquares) & targetBB;
        addMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupiedSquares) & targetBB;
        addMovesFromMask(moves, tempMove, from);
    }

    from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks(from) & targetBB;
    addMovesFromMask(moves, tempMove, from);

    if (!pos.isAttacked(Square::E1 + 56 * side, !side))
    {
        if (pos.getCastlingRights() & (1 << (2 * side))) // Short castling.
        {
            if (!(occupiedSquares & (0x0000000000000060ull << (56 * side))))
            {
                if (!(pos.isAttacked(Square::F1 + 56 * side, !side)) && !(pos.isAttacked(Square::G1 + 56 * side, !side)))
                {
                    moves.push_back(Move(Square::E1 + 56 * side, Square::G1 + 56 * side, Piece::King, 0));
                }
            }
        }
        if (pos.getCastlingRights() & (2 << (2 * side))) // Long castling.
        {
            if (!(occupiedSquares & (0x000000000000000Eull << (56 * side))))
            {
                if (!(pos.isAttacked(Square::D1 + 56 * side, !side)) && !(pos.isAttacked(Square::C1 + 56 * side, !side)))
                {
                    moves.push_back(Move(Square::E1 + 56 * side, Square::C1 + 56 * side, Piece::King, 0));
                }
            }
        }
    }
}

template <bool side>
void MoveGen::generateLegalEvasions(const Position& pos, MoveList& moves)
{
    int from, to;
    auto occupied = pos.getOccupiedSquares();
    auto freeSquares = pos.getFreeSquares();
    auto targetBitboard = ~pos.getPieces(side);

    auto kingLocation = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    auto tempMove = Bitboards::kingAttacks(kingLocation) & targetBitboard;
    // Remove our king from occupied so we can find out if squares "behind" our king are attacked.
    Bitboards::clearBit(occupied, kingLocation);
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        if (!(Bitboards::pawnAttacks(side, to) & pos.getBitboard(!side, Piece::Pawn)
            || Bitboards::knightAttacks(to) & pos.getBitboard(!side, Piece::Knight)
            || Bitboards::kingAttacks(to) & pos.getBitboard(!side, Piece::King)
            || Bitboards::bishopAttacks(to, occupied) & (pos.getBitboard(!side, Piece::Queen) | pos.getBitboard(!side, Piece::Bishop))
            || Bitboards::rookAttacks(to, occupied) & (pos.getBitboard(!side, Piece::Queen) | pos.getBitboard(!side, Piece::Rook))))
        {
            moves.push_back(Move(kingLocation, to, Piece::Empty, 0));
        }
    }
    Bitboards::setBit(occupied, kingLocation); // And now put it back.

    auto checkers = (Bitboards::rookAttacks(kingLocation, occupied) & (pos.getBitboard(!side, Piece::Queen) | pos.getBitboard(!side, Piece::Rook)))
        | (Bitboards::bishopAttacks(kingLocation, occupied) & (pos.getBitboard(!side, Piece::Queen) | pos.getBitboard(!side, Piece::Bishop)))
        | (Bitboards::knightAttacks(kingLocation) & pos.getBitboard(!side, Piece::Knight))
        | (Bitboards::kingAttacks(kingLocation) & pos.getBitboard(!side, Piece::King))
        | (Bitboards::pawnAttacks(side, kingLocation) & pos.getBitboard(!side, Piece::Pawn));
    // If we are checked by more than two pieces only the king can move and we are done.
    if (Bitboards::moreThanOneBitSet(checkers))
    {
        return;
    }
    auto checkerLocation = Bitboards::lsb(checkers);
    auto interpose = Bitboards::squaresBetween(kingLocation, checkerLocation) | checkers;
    auto pinned = pos.getPinnedPieces();

    auto tempPiece = pos.getBitboard(side, Piece::Pawn) & ~pinned;
    auto m = (side ? tempPiece >> 8 : tempPiece << 8) & freeSquares;
    tempMove = m & interpose;
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        from = to - 8 + side * 16;
        if (to >= Square::A8 || to <= Square::H1)
        {
            moves.push_back(Move(from, to, Piece::Queen, 0));
            moves.push_back(Move(from, to, Piece::Rook, 0));
            moves.push_back(Move(from, to, Piece::Bishop, 0));
            moves.push_back(Move(from, to, Piece::Knight, 0));
        }
        else
        {
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }
    tempMove = (side ? (m & Bitboards::ranks[5]) >> 8 : (m & Bitboards::ranks[2]) << 8) & freeSquares & interpose;
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        from = to - 16 + side * 32;
        moves.push_back(Move(from, to, Piece::Empty, 0));
    }

    auto ep = (pos.getEnPassantSquare() != Square::NoSquare && checkerLocation == (pos.getEnPassantSquare() - 8 + side * 16)) ? Bitboards::bit(pos.getEnPassantSquare()) : 0;
    tempMove = (side ? tempPiece >> 9 : tempPiece << 7) & 0x7F7F7F7F7F7F7F7F & (checkers | ep);
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        from = to - 7 + side * 16;
        if (to == pos.getEnPassantSquare())
        {
            moves.push_back(Move(from, to, Piece::Pawn, 0));
        }
        else if (to >= Square::A8 || to <= Square::H1)
        {
            moves.push_back(Move(from, to, Piece::Queen, 0));
            moves.push_back(Move(from, to, Piece::Rook, 0));
            moves.push_back(Move(from, to, Piece::Bishop, 0));
            moves.push_back(Move(from, to, Piece::Knight, 0));
        }
        else
        {
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    tempMove = (side ? tempPiece >> 7 : tempPiece << 9) & 0xFEFEFEFEFEFEFEFE & (checkers | ep);
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        from = to - 9 + side * 16;
        if (to == pos.getEnPassantSquare())
        {
            moves.push_back(Move(from, to, Piece::Pawn, 0));
        }
        else if (to >= Square::A8 || to <= Square::H1)
        {
            moves.push_back(Move(from, to, Piece::Queen, 0));
            moves.push_back(Move(from, to, Piece::Rook, 0));
            moves.push_back(Move(from, to, Piece::Bishop, 0));
            moves.push_back(Move(from, to, Piece::Knight, 0));
        }
        else
        {
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    tempPiece = pos.getBitboard(side, Piece::Knight) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks(from) & interpose;
        addMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Bishop) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupied) & interpose;
        addMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Rook) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupied) & interpose;
        addMovesFromMask(moves, tempMove, from);
    }

    tempPiece = pos.getBitboard(side, Piece::Queen) & ~pinned;
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupied) & interpose;
        addMovesFromMask(moves, tempMove, from);
    }
}

