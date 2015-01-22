#ifndef PIECE_HPP_
#define PIECE_HPP_

#include <cassert>
#include <cstdint>

// Represents a single piece or piecetype.
class Piece
{
public:
    Piece() : piece(NoPiece) {};
    Piece(int8_t newPiece) : piece(newPiece) {};

    enum : int8_t
    {
        Pawn = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4, King = 5
    };

    enum : int8_t
    {
        WhitePawn = 0, WhiteKnight = 1, WhiteBishop = 2, WhiteRook = 3, WhiteQueen = 4, WhiteKing = 5,
        BlackPawn = 6, BlackKnight = 7, BlackBishop = 8, BlackRook = 9, BlackQueen = 10, BlackKing = 11, 
        Empty = 12, NoPiece = 13
    };

    operator int8_t() const { return piece; }
    operator int8_t&() { return piece; }
private:
    int8_t piece;
};

// Checks if the piece is in the specified limits, i.e. >= WhitePawn and <= NoPiece. 
inline bool pieceIsOkLoose(Piece p)
{
    return (p >= Piece::WhitePawn && p <= Piece::NoPiece);
}

// Checks if the piece is ok for playing, i.e. >= WhitePawn and <= Empty. 
inline bool pieceIsOkStrict(Piece p)
{
    return (p >= Piece::WhitePawn && p <= Piece::Empty);
}

// Checks if the piece type is ok. Should only be called if the given piece represents a piecetype.
inline bool pieceTypeIsOk(Piece p)
{
    return (p >= Piece::Pawn && p <= Piece::King);
}

// Gets the piece type of the given piece. We assume that p is representing a piece at the moment.
inline Piece getPieceType(Piece p)
{
    assert(pieceIsOkStrict(p) && p != Piece::Empty);
    return (p % 6);
}

#endif