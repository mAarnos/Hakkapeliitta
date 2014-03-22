#ifndef MOVEGEN_CPP
#define MOVEGEN_CPP

#include "movegen.h"
#include "bitboard.h"
#include "magic.h"

void generateMoves(Position & pos, Move * mlist)
{
	int from, to;
	bool side = pos.getSideToMove();
	int generatedMoves = 0;
	uint64_t tempPiece, tempMove, tempCapture;

	Move m;
	m.clear(); 

	uint64_t freeSquares = pos.getFreeSquares();
	uint64_t enemyPieces = pos.getPieces(!side);
	uint64_t occupiedSquares = pos.getOccupiedSquares();

	m.setPiece(Pawn + side * 6);
	tempPiece = pos.getBitboard(side, Pawn);
	// Try to make this part not depend on the side to move.
	if (side == White)
	{
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			clearBit(tempPiece, from);

			m.setCapture(Empty);
			if (freeSquares & pawnSingleMoves[White][from])
			{
				to = from + 8;
				m.setTo(to);
				if (to >= A8)
				{
					m.setPromotion(WhiteQueen); mlist[generatedMoves++] = m;
					m.setPromotion(WhiteRook); mlist[generatedMoves++] = m;
					m.setPromotion(WhiteBishop); mlist[generatedMoves++] = m;
					m.setPromotion(WhiteKnight); mlist[generatedMoves++] = m;
					m.setPromotion(Empty); 
				}
				else
				{
					mlist[generatedMoves++] = m;
				}
			}
			if (freeSquares & pawnDoubleMoves[White][from])
			{
				to = from + 16;
				m.setTo(to);
				mlist[generatedMoves++] = m;
			}

			tempCapture = pawnAttacks[White][from] & enemyPieces;
			if (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(pos.getPiece(to));
				if (to >= A8)
				{
					m.setPromotion(WhiteQueen); mlist[generatedMoves++] = m;
					m.setPromotion(WhiteRook); mlist[generatedMoves++] = m;
					m.setPromotion(WhiteBishop); mlist[generatedMoves++] = m;
					m.setPromotion(WhiteKnight); mlist[generatedMoves++] = m;
					m.setPromotion(Empty);
				}
				else
				{
					mlist[generatedMoves++] = m;
				}
			}
			// Beware with Null move.
			if (pos.getEnPassantSquare() != 64)
			{
				if (pawnAttacks[White][from] & (uint64_t)1 << pos.getEnPassantSquare())
				{
					m.setPromotion(WhitePawn);
					m.setCapture(BlackPawn);
					m.setTo(pos.getEnPassantSquare());
					mlist[generatedMoves++] = m;
					m.setPromotion(Empty);
				}
			}
		}
	}
	else
	{
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			clearBit(tempPiece, from);

			m.setCapture(Empty);
			if (freeSquares & pawnSingleMoves[Black][from])
			{
				to = from - 8;
				m.setTo(to);
				if (to <= H2)
				{
					m.setPromotion(BlackQueen); mlist[generatedMoves++] = m;
					m.setPromotion(BlackRook); mlist[generatedMoves++] = m;
					m.setPromotion(BlackBishop); mlist[generatedMoves++] = m;
					m.setPromotion(BlackKnight); mlist[generatedMoves++] = m;
					m.setPromotion(Empty);
				}
				else
				{
					mlist[generatedMoves++] = m;
				}
			}
			if (freeSquares & pawnDoubleMoves[Black][from])
			{
				to = from - 16;
				m.setTo(to);
				mlist[generatedMoves++] = m;
			}

			tempCapture = pawnAttacks[Black][from] & enemyPieces;
			if (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(pos.getPiece(to));
				if (to <= H2)
				{
					m.setPromotion(BlackQueen); mlist[generatedMoves++] = m;
					m.setPromotion(BlackRook); mlist[generatedMoves++] = m;
					m.setPromotion(BlackBishop); mlist[generatedMoves++] = m;
					m.setPromotion(BlackKnight); mlist[generatedMoves++] = m;
					m.setPromotion(Empty);
				}
				else
				{
					mlist[generatedMoves++] = m;
				}
			}
			// Beware with Null move.
			if (pos.getEnPassantSquare() != 64)
			{
				if (pawnAttacks[Black][from] & (uint64_t)1 << pos.getEnPassantSquare())
				{
					m.setPromotion(BlackPawn);
					m.setCapture(WhitePawn);
					m.setTo(pos.getEnPassantSquare());
					mlist[generatedMoves++] = m;
					m.setPromotion(Empty);
				}
			}
		}
	}

	m.setPiece(Knight + side * 6);
	tempPiece = pos.getBitboard(side, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		clearBit(tempPiece, from);
		m.setFrom(from);
		
		tempMove = knightAttacks[from] & freeSquares;
		tempCapture = knightAttacks[from] & enemyPieces;

		m.setCapture(Empty);
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			clearBit(tempMove, to);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			clearBit(tempCapture, to);
			m.setTo(to);
			m.setCapture(pos.getPiece(to));
			mlist[generatedMoves++] = m;
		}
	}

	m.setPiece(Bishop + side * 6);
	tempPiece = pos.getBitboard(side, Bishop);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		clearBit(tempPiece, from);
		m.setFrom(from);
		uint64_t allMoves = bishopAttacks(from, occupiedSquares);
		tempMove = allMoves & freeSquares;
		tempCapture = allMoves & enemyPieces;
		m.setCapture(Empty);
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			clearBit(tempMove, to);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			clearBit(tempCapture, to);
			m.setTo(to);
			m.setCapture(pos.getPiece(to));
			mlist[generatedMoves++] = m;
		}
	}

	m.setPiece(Rook + side * 6);
	tempPiece = pos.getBitboard(side, Rook);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		clearBit(tempPiece, from);
		m.setFrom(from);
		uint64_t allMoves = rookAttacks(from, occupiedSquares);
		tempMove = allMoves & freeSquares;
		tempCapture = allMoves & enemyPieces;
		m.setCapture(Empty);
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			clearBit(tempMove, to);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			clearBit(tempCapture, to);
			m.setTo(to);
			m.setCapture(pos.getPiece(to));
			mlist[generatedMoves++] = m;
		}
	}

	m.setPiece(Queen + side * 6);
	tempPiece = pos.getBitboard(side, Queen);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		clearBit(tempPiece, from);
		m.setFrom(from);
		uint64_t allMoves = queenAttacks(from, occupiedSquares);
		tempMove = allMoves & freeSquares;
		tempCapture = allMoves & enemyPieces;
		m.setCapture(Empty);
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			clearBit(tempMove, to);
			m.setTo(to);
			mlist[generatedMoves++] = m;
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			clearBit(tempCapture, to);
			m.setTo(to);
			m.setCapture(pos.getPiece(to));
			mlist[generatedMoves++] = m;
		}
	}

	m.setPiece(King + side * 6);
	from = bitScanForward(pos.getBitboard(side, King));
	m.setFrom(from);
	tempMove = kingAttacks[from] & freeSquares;
	tempCapture = kingAttacks[from] & enemyPieces;
	m.setCapture(Empty);
	while (tempMove)
	{
		to = bitScanForward(tempMove);
		clearBit(tempMove, to);
		m.setTo(to);
		mlist[generatedMoves++] = m;
	}
	while (tempCapture)
	{
		to = bitScanForward(tempCapture);
		clearBit(tempCapture, to);
		m.setTo(to);
		m.setCapture(pos.getPiece(to));
		mlist[generatedMoves++] = m;
	}

	if (side == White)
	{
		if (pos.getCastlingRights() & 1)
		{
			if (!(occupiedSquares & (uint64_t)0x0000000000000060))
			{
				m.setCapture(Empty);
				m.setPiece(WhiteKing);
				m.setPromotion(WhiteKing);
				m.setFrom(E1);
				m.setTo(G1);
				mlist[generatedMoves++] = m;
			}
		}
		if (pos.getCastlingRights() & 2)
		{
			if (!(occupiedSquares & (uint64_t)0x000000000000000E))
			{
				m.setCapture(Empty);
				m.setPiece(WhiteKing);
				m.setPromotion(WhiteKing);
				m.setFrom(E1);
				m.setTo(C1);
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
				m.setCapture(Empty);
				m.setPiece(BlackKing);
				m.setPromotion(BlackKing);
				m.setFrom(E8);
				m.setTo(G8);
				mlist[generatedMoves++] = m;
			}
		}
		if (pos.getCastlingRights() & 8)
		{
			if (!(occupiedSquares & (uint64_t)0x0E00000000000000))
			{
				m.setCapture(Empty);
				m.setPiece(BlackKing);
				m.setPromotion(BlackKing);
				m.setFrom(E8);
				m.setTo(C8);
				mlist[generatedMoves++] = m;
			}
		}
	}

}
	
#endif