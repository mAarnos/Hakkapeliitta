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
	uint64_t tempPiece, tempMove, tempCapture;

	Move m;
	m.clear(); 
	m.setPromotion(Empty);
	m.setScore(0);

	uint64_t freeSquares = pos.getFreeSquares();
	uint64_t enemyPieces = pos.getPieces(!side);
	uint64_t occupiedSquares = pos.getOccupiedSquares();

	tempPiece = pos.getBitboard(side, Pawn);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		m.setFrom(from);
		tempPiece ^= bit[from];

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

		tempCapture = pawnAttacks[side][from] & enemyPieces;
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			tempCapture ^= bit[to];
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
		tempPiece ^= bit[from];
		m.setFrom(from);
		
		tempMove = knightAttacks[from] & freeSquares;
		tempCapture = knightAttacks[from] & enemyPieces;

		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove ^= bit[to];
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			tempCapture ^= bit[to];
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	tempPiece = pos.getBitboard(side, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece ^= bit[from];
		m.setFrom(from);

		uint64_t allMoves = bishopAttacks(from, occupiedSquares);
		tempMove = allMoves & freeSquares;
		tempCapture = allMoves & enemyPieces;

		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove ^= bit[to];
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			tempCapture ^= bit[to];
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	tempPiece = pos.getBitboard(side, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece ^= bit[from];
		m.setFrom(from);

		uint64_t allMoves = rookAttacks(from, occupiedSquares);
		tempMove = allMoves & freeSquares;
		tempCapture = allMoves & enemyPieces;

		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove ^= bit[to];
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			tempCapture ^= bit[to];
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	tempPiece = pos.getBitboard(side, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece ^= bit[from];
		m.setFrom(from);

		uint64_t allMoves = queenAttacks(from, occupiedSquares);
		tempMove = allMoves & freeSquares;
		tempCapture = allMoves & enemyPieces;

		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove ^= bit[to];
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			tempCapture ^= bit[to];
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
	}

	from = bitScanForward(pos.getBitboard(side, King));
	m.setFrom(from);
	tempMove = kingAttacks[from] & freeSquares;
	tempCapture = kingAttacks[from] & enemyPieces;
	while (tempMove)
	{
		to = bitScanForward(tempMove);
		tempMove ^= bit[to];
		m.setTo(to);
		mlist[generatedMoves++] = m;
	}
	while (tempCapture)
	{
		to = bitScanForward(tempCapture);
		tempCapture ^= bit[to];
		m.setTo(to);
		mlist[generatedMoves++] = m;
	}

	if (side == White)
	{
		if (pos.getCastlingRights() & 1)
		{
			if (!(occupiedSquares & (uint64_t)0x0000000000000060))
			{
				m.setFrom(E1);
				m.setTo(G1);
				m.setPromotion(King);
				mlist[generatedMoves++] = m;
			}
		}
		if (pos.getCastlingRights() & 2)
		{
			if (!(occupiedSquares & (uint64_t)0x000000000000000E))
			{
				m.setFrom(E1);
				m.setTo(C1);
				m.setPromotion(King);
				mlist[generatedMoves++] = m;
			}
		}
	}
	else
	{
		if (pos.getCastlingRights() & 4)
		{
			if (!(occupiedSquares & (uint64_t)0x6000000000000000))
			{
				m.setFrom(E8);
				m.setTo(G8);
				m.setPromotion(King);
				mlist[generatedMoves++] = m;
			}
		}
		if (pos.getCastlingRights() & 8)
		{
			if (!(occupiedSquares & (uint64_t)0x0E00000000000000))
			{
				m.setFrom(E8);
				m.setTo(C8);
				m.setPromotion(King);
				mlist[generatedMoves++] = m;
			}
		}
	}
	return generatedMoves;
}
	
#endif