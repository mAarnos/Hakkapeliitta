#ifndef MOVEGEN_CPP
#define MOVEGEN_CPP

#include "movegen.h"
#include "bitboard.h"
#include "magic.h"

int generateMoves(Position & pos, Move * mlist)
{
	int from, to;
	bool side = pos.getSideToMove();
	int generatedMoves = 0;
	uint64_t tempPiece, tempMove;

	Move m;
	m.clear(); 
	m.setPromotion(Empty);

	uint64_t freeSquares = pos.getFreeSquares();
	uint64_t enemyPieces = pos.getPieces(!side);
	uint64_t targetBB = freeSquares | enemyPieces;
	uint64_t occupiedSquares = pos.getOccupiedSquares();

	tempPiece = pos.getBitboard(side, Pawn);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		m.setFrom(from);
		tempPiece &= (tempPiece - 1);

		if (freeSquares & pawnSingleMoves[side][from])
		{
			to = from + 8 - side * 16;
			m.setTo(to);
			if (to >= A8 || to <= H1)
			{
				m.setPromotion(Queen); mlist[generatedMoves++] = m;
				m.setPromotion(Rook); mlist[generatedMoves++] = m;
				m.setPromotion(Bishop); mlist[generatedMoves++] = m;
				m.setPromotion(Knight); mlist[generatedMoves++] = m;
				m.setPromotion(Empty); 
			}
			else
			{
				mlist[generatedMoves++] = m;
			}

			if (freeSquares & pawnDoubleMoves[side][from])
			{
				to = from + 16 - side * 32;
				m.setTo(to);
				mlist[generatedMoves++] = m;
			}
		}

		uint64_t tempCapture = pawnAttacks[side][from] & enemyPieces;
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			tempCapture &= (tempCapture - 1);
			m.setTo(to);
			if (to >= A8 || to <= H1)
			{
				m.setPromotion(Queen); mlist[generatedMoves++] = m;
				m.setPromotion(Rook); mlist[generatedMoves++] = m;
				m.setPromotion(Bishop); mlist[generatedMoves++] = m;
				m.setPromotion(Knight); mlist[generatedMoves++] = m;
				m.setPromotion(Empty);
			}
			else
			{
				mlist[generatedMoves++] = m;
			}
		}

		// Beware with Null move, you might be en passanting your own pawns.
		if (pos.getEnPassantSquare() != 64)
		{
			if (pawnAttacks[side][from] & bit[pos.getEnPassantSquare()])
			{
				m.setPromotion(Pawn);
				m.setTo(pos.getEnPassantSquare());
				mlist[generatedMoves++] = m;
				m.setPromotion(Empty);
			}
		}
	}
	
	tempPiece = pos.getBitboard(side, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		m.setFrom(from);
		
		tempMove = knightAttacks[from] & targetBB;

		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	tempPiece = pos.getBitboard(side, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		m.setFrom(from);

		tempMove = bishopAttacks(from, occupiedSquares) & targetBB;

		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	tempPiece = pos.getBitboard(side, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		m.setFrom(from);

		tempMove = rookAttacks(from, occupiedSquares) & targetBB;

		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	tempPiece = pos.getBitboard(side, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		m.setFrom(from);

		tempMove = queenAttacks(from, occupiedSquares) & targetBB;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	from = bitScanForward(pos.getBitboard(side, King));
	m.setFrom(from);
	tempMove = kingAttacks[from] & targetBB;
	while (tempMove)
	{
		to = bitScanForward(tempMove);
		tempMove &= (tempMove - 1);
		m.setTo(to);
		mlist[generatedMoves++] = m;
	}

	if (side == White)
	{
		if (pos.isAttacked(E1, Black))
		{
			return generatedMoves;
		}
		m.setFrom(E1);
		m.setPromotion(King);
		if (pos.getCastlingRights() & 1)
		{
			if (!(occupiedSquares & (uint64_t)0x0000000000000060))
			{
				if (!(pos.isAttacked(F1, Black)))
				{
					m.setTo(G1);
					mlist[generatedMoves++] = m;
				}
			}
		}
		if (pos.getCastlingRights() & 2)
		{
			if (!(occupiedSquares & (uint64_t)0x000000000000000E))
			{
				if (!(pos.isAttacked(D1, Black)))
				{
					m.setTo(C1);
					mlist[generatedMoves++] = m;
				}
			}
		}
	}
	else
	{
		if (pos.isAttacked(E8, White))
		{
			return generatedMoves;
		}
		m.setFrom(E8);
		m.setPromotion(King);
		if (pos.getCastlingRights() & 4)
		{
			if (!(occupiedSquares & (uint64_t)0x6000000000000000))
			{
				if (!(pos.isAttacked(F8, White)))
				{
					m.setTo(G8);
					mlist[generatedMoves++] = m;
				}
			}
		}
		if (pos.getCastlingRights() & 8)
		{
			if (!(occupiedSquares & (uint64_t)0x0E00000000000000))
			{
				if (!(pos.isAttacked(D8, White)))
				{
					m.setTo(C8);
					mlist[generatedMoves++] = m;
				}
			}
		}
	}
	return generatedMoves;
}

int generateCaptures(Position & pos, Move * mlist)
{
	int from, to;
	bool side = pos.getSideToMove();
	int generatedMoves = 0;
	uint64_t tempPiece, tempMove;

	Move m;
	m.clear();
	m.setPromotion(Empty);

	uint64_t enemyPieces = pos.getPieces(!side);
	uint64_t occupiedSquares = pos.getOccupiedSquares();

	tempPiece = pos.getBitboard(side, Pawn);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		m.setFrom(from);
		tempPiece &= (tempPiece - 1);

		tempMove = pawnAttacks[side][from] & enemyPieces;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			if (to >= A8 || to <= H1)
			{
				m.setPromotion(Queen); mlist[generatedMoves++] = m;
				m.setPromotion(Rook); mlist[generatedMoves++] = m;
				m.setPromotion(Bishop); mlist[generatedMoves++] = m;
				m.setPromotion(Knight); mlist[generatedMoves++] = m;
				m.setPromotion(Empty);
			}
			else
			{
				mlist[generatedMoves++] = m;
			}
		}

		// Beware with Null move, you might be en passanting your own pawns.
		if (pos.getEnPassantSquare() != 64)
		{
			if (pawnAttacks[side][from] & bit[pos.getEnPassantSquare()])
			{
				m.setPromotion(Pawn);
				m.setTo(pos.getEnPassantSquare());
				mlist[generatedMoves++] = m;
				m.setPromotion(Empty);
			}
		}
	}

	tempPiece = pos.getBitboard(side, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		m.setFrom(from);

		tempMove = knightAttacks[from] & enemyPieces;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	tempPiece = pos.getBitboard(side, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		m.setFrom(from);

		tempMove = bishopAttacks(from, occupiedSquares) & enemyPieces;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	tempPiece = pos.getBitboard(side, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		m.setFrom(from);

		tempMove = rookAttacks(from, occupiedSquares) & enemyPieces;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	tempPiece = pos.getBitboard(side, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		m.setFrom(from);

		tempMove = queenAttacks(from, occupiedSquares) & enemyPieces;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	from = bitScanForward(pos.getBitboard(side, King));
	m.setFrom(from);
	tempMove = kingAttacks[from] & enemyPieces;
	while (tempMove)
	{
		to = bitScanForward(tempMove);
		tempMove &= (tempMove - 1);
		m.setTo(to);
		mlist[generatedMoves++] = m;
	}

	return generatedMoves;
}
	
#endif