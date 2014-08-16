#include "piece.hpp"

Piece::Piece():
piece(NoPiece)
{
}

Piece::Piece(int newPiece) :
piece(newPiece)
{
}

bool isPieceOk(Piece p)
{
	return ((p >= Piece::WhitePawn) && (p <= Piece::BlackKing));
}

