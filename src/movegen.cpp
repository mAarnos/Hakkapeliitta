#include "movegen.hpp"
#include <iostream>

void MoveGen::generatePseudoLegalMoves(Position & pos, MoveList & moves)
{
    pos.getSideToMove() ? generatePseudoLegalMoves<true>(pos, moves) : generatePseudoLegalMoves<false>(pos, moves);
}

void MoveGen::generatePseudoLegalCaptureMoves(Position & pos, MoveList & moves)
{
    pos.getSideToMove() ? generatePseudoLegalCaptureMoves<true>(pos, moves) : generatePseudoLegalCaptureMoves<false>(pos, moves);
}

template <bool side> 
void MoveGen::generatePseudoLegalMoves(Position & pos, MoveList & moves)
{
    assert(moves.empty());

    int from, to;
    Bitboard tempMove;
    auto freeSquares = pos.getFreeSquares();
    auto enemyPieces = pos.getPieces(!side);
    auto targetBB = freeSquares | enemyPieces;
    auto occupiedSquares = pos.getOccupiedSquares();

    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);

        if (freeSquares & Bitboards::pawnSingleMoves[side][from])
        {
            to = from + 8 - side * 16;
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

            if (freeSquares & Bitboards::pawnDoubleMoves[side][from])
            {
                to = from ^ 16;
                moves.push_back(Move(from, to, Piece::Empty, 0));
            }
        }

        auto tempCapture = Bitboards::pawnAttacks[side][from] & enemyPieces;
        while (tempCapture)
        {
            to = Bitboards::popLsb(tempCapture);

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

        if (pos.getEnPassantSquare() != Square::NoSquare)
        {
            if (Bitboards::pawnAttacks[side][from] & Bitboards::bit[pos.getEnPassantSquare()])
            {
                moves.push_back(Move(from, pos.getEnPassantSquare(), Piece::Pawn, 0));
            }
        }
    }

    tempPiece = pos.getBitboard(side, Piece::Knight);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks[from] & targetBB;

        while (tempMove)
        {
            to = Bitboards::popLsb(tempMove);
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupiedSquares) & targetBB;

        while (tempMove)
        {
            to = Bitboards::popLsb(tempMove);
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupiedSquares) & targetBB;

        while (tempMove)
        {
            to = Bitboards::popLsb(tempMove);
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupiedSquares) & targetBB;

        while (tempMove)
        {
            to = Bitboards::popLsb(tempMove);
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks[from] & targetBB;

    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        moves.push_back(Move(from, to, Piece::Empty, 0));
    }

    if (!pos.isAttacked(Square::E1 + 56 * side, !side))
    {
        if (pos.getCastlingRights() & (1 << (2 * side))) // Short castling.
        {
            if (!(occupiedSquares & (0x0000000000000060ull << (56 * side))))
            {
                if (!(pos.isAttacked(Square::F1 + 56 * side, !side)))
                {
                    moves.push_back(Move(Square::E1 + 56 * side, Square::G1 + 56 * side, Piece::King, 0));
                }
            }
        }
        if (pos.getCastlingRights() & (2 << (2 * side))) // Long castling.
        {
            if (!(occupiedSquares & (0x000000000000000Eull << (56 * side))))
            {
                if (!(pos.isAttacked(Square::D1 + 56 * side, !side)))
                {
                    moves.push_back(Move(Square::E1 + 56 * side, Square::C1 + 56 * side, Piece::King, 0));
                }
            }
        }
    }
}

template <bool side>
void MoveGen::generatePseudoLegalCaptureMoves(Position & pos, MoveList & moves)
{
    assert(moves.empty());

    int from, to;
    Bitboard tempMove;
    auto enemyPieces = pos.getPieces(!side);
    auto occupiedSquares = pos.getOccupiedSquares();

    auto tempPiece = pos.getBitboard(side, Piece::Pawn);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);

        tempMove = Bitboards::pawnAttacks[side][from] & enemyPieces;
        while (tempMove)
        {
            to = Bitboards::popLsb(tempMove);

            if (to >= Square::A8 || to <= Square::H1)
            {
                moves.push_back(Move(from, to, Piece::Queen, 0));
            }
            else
            {
                moves.push_back(Move(from, to, Piece::Empty, 0));
            }
        }

        if (pos.getEnPassantSquare() != Square::NoSquare)
        {
            if (Bitboards::pawnAttacks[side][from] & Bitboards::bit[pos.getEnPassantSquare()])
            {
                moves.push_back(Move(from, pos.getEnPassantSquare(), Piece::Pawn, 0));
            }
        }
    }

    tempPiece = pos.getBitboard(side, Piece::Knight);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::knightAttacks[from] & enemyPieces;

        while (tempMove)
        {
            to = Bitboards::popLsb(tempMove);
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    tempPiece = pos.getBitboard(side, Piece::Bishop);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::bishopAttacks(from, occupiedSquares) & enemyPieces;

        while (tempMove)
        {
            to = Bitboards::popLsb(tempMove);
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    tempPiece = pos.getBitboard(side, Piece::Rook);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::rookAttacks(from, occupiedSquares) & enemyPieces;

        while (tempMove)
        {
            to = Bitboards::popLsb(tempMove);
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    tempPiece = pos.getBitboard(side, Piece::Queen);
    while (tempPiece)
    {
        from = Bitboards::popLsb(tempPiece);
        tempMove = Bitboards::queenAttacks(from, occupiedSquares) & enemyPieces;

        while (tempMove)
        {
            to = Bitboards::popLsb(tempMove);
            moves.push_back(Move(from, to, Piece::Empty, 0));
        }
    }

    from = Bitboards::lsb(pos.getBitboard(side, Piece::King));
    tempMove = Bitboards::kingAttacks[from] & enemyPieces;
    while (tempMove)
    {
        to = Bitboards::popLsb(tempMove);
        moves.push_back(Move(from, to, Piece::Empty, 0));
    }
}


