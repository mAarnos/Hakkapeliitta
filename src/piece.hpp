#ifndef PIECE_HPP_
#define PIECE_HPP_

class Piece
{
public:
    Piece() : piece(NoPiece) {};
    Piece(int newPiece) : piece(newPiece) {};

    enum {
        Pawn = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4, King = 5
    };

    enum {
        WhitePawn = 0, WhiteKnight = 1, WhiteBishop = 2, WhiteRook = 3, WhiteQueen = 4, WhiteKing = 5,
        BlackPawn = 6, BlackKnight = 7, BlackBishop = 8, BlackRook = 9, BlackQueen = 10, BlackKing = 11, 
        Empty = 12, NoPiece = 13
    };

    operator int() const { return piece; }
    operator int&() { return piece; }
private:
    int piece;
};

// Checks if the piece, i.e. >= WhitePawn and <= BlackKing. Expand to empty?
inline bool isPieceOk(Piece p)
{
    return ((p >= Piece::WhitePawn) && (p <= Piece::BlackKing));
}

#endif