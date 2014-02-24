#ifndef MOVEGEN_CPP
#define MOVEGEN_CPP

#include "movegen.h"
#include "defs.h"
#include "magic.h"
#include "board.h"
#include "bitboard.h"
#include "move.h"
#include "hash.h"
#include "eval.h"
#include <iomanip>
#include "see.h"

U64 attack(int sq, bool s);
void makeWhitePromotion(int prom, int &to);
void unmakeWhitePromotion(int prom, int &to);
void makeBlackPromotion(int prom, int &to);
void unmakeBlackPromotion(int prom, int &to);
void makeCapture(int &captured, int &to);
void unmakeCapture(int &captured, int &to);

Move moveStack[10000];
int moveList[600];

history historyStack[600];

int whiteShortCastle, whiteLongCastle, blackShortCastle, blackLongCastle;

void initializeCastlingMoveInts()
{
	Move m;

	m.clear();
	m.setCapture(Empty);
	m.setPiece(WhiteKing);
	m.setPromotion(WhiteKing);
	m.setFrom(E1);
	m.setTo(G1);
	whiteShortCastle = m.moveInt;
	m.setTo(C1);
	whiteLongCastle = m.moveInt; 
	
	m.clear();
	m.setCapture(Empty);
	m.setPiece(BlackKing);
	m.setPromotion(BlackKing);
	m.setFrom(E8);
	m.setTo(G8);
	blackShortCastle = m.moveInt;
	m.setTo(C8);
	blackLongCastle = m.moveInt; 
}

// returns true if side s is in check
U64 inCheck(bool s)
{
	int i; 

	if (s == White)
	{
		i = bitScanForward(whiteKing);
		return attack(i, s ^ 1);
	}
	else
	{
		i = bitScanForward(blackKing); 
		return attack(i, s ^ 1);
	}
}

U64 attack(int sq, bool s)
{
	U64 blockers;
	U64 bishopAttacks, rookAttacks;
	U64 occupied = occupiedSquares;
	int databaseIndex;

	U64 opPawns, opKnights, opRQ, opBQ;
	if (s == White) {
		opPawns     = whitePawns;
		opKnights   = whiteKnights;
		opRQ = opBQ = whiteQueens;
		opRQ       |= whiteRooks;
		opBQ       |= whiteBishops;
		
		blockers = occupied & occupancyMaskRook[sq];
		databaseIndex = (int)((blockers * magicNumberRook[sq]) >> rookMagicShifts[sq]);
		rookAttacks = magicMovesRook[sq][databaseIndex] & ~blackPieces;
		blockers = occupied & occupancyMaskBishop[sq];
		databaseIndex = (int)((blockers * magicNumberBishop[sq]) >> bishopMagicShifts[sq]);
		bishopAttacks = magicMovesBishop[sq][databaseIndex] & ~blackPieces;

		U64 returning = blackPawnAttacks[sq] & opPawns;
		returning |= knightAttacks[sq] & opKnights;
		returning |= bishopAttacks & opBQ;
		returning |= rookAttacks & opRQ;
		returning |= kingAttacks[sq] & whiteKing;

		return (returning);
	}
	else {
		opPawns     = blackPawns;
		opKnights   = blackKnights;
		opRQ = opBQ = blackQueens;
		opRQ       |= blackRooks;
		opBQ       |= blackBishops;
		
		blockers = occupied & occupancyMaskRook[sq];
		databaseIndex = (int)((blockers * magicNumberRook[sq]) >> rookMagicShifts[sq]);
		rookAttacks = magicMovesRook[sq][databaseIndex] & ~whitePieces;
		blockers = occupied & occupancyMaskBishop[sq];
		databaseIndex = (int)((blockers * magicNumberBishop[sq]) >> bishopMagicShifts[sq]);
		bishopAttacks = magicMovesBishop[sq][databaseIndex] & ~whitePieces;

		U64 returning = whitePawnAttacks[sq] & opPawns;
		returning |= knightAttacks[sq] & opKnights;
		returning |= bishopAttacks & opBQ;
		returning |= rookAttacks & opRQ;
		returning |= kingAttacks[sq] & blackKing;

		return (returning); 
	}
}	

void generateMoves()
{
	int from, to;
	U64 blockers;
	U64 occupied = occupiedSquares;
	int databaseIndex;
	U64 tempPiece, tempMove, tempCapture;
	U64 targetBitboard, freeSquares;
	Move m;
	
	m.clear();
	freeSquares = ~occupiedSquares;
	
	moveList[ply + 1] = moveList[ply];
	if (sideToMove == White)
	{
		targetBitboard = ~whitePieces;
		
		// White pawns
		m.setPiece(WhitePawn);
		tempPiece = whitePawns;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			int first = bitScanForward(whitePawnMoves[from] & occupiedSquares);
			tempMove = whitePawnMoves[from] & below[first];
			tempCapture = whitePawnAttacks[from] & blackPieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				if (to >= A8)
				{
					m.setPromotion(WhiteQueen); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteRook); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteBishop); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteKnight); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(Empty); // this is necessary
				}
				else 
				{
					moveStack[moveList[ply + 1]].score = 0;
					moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				}
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				if (to >= A8)
				{
					m.setPromotion(WhiteQueen); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteRook); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteBishop); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteKnight); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(Empty); // this is necessary
				}
				else 
				{
					moveStack[moveList[ply + 1]].score = 0;
					moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				}
				tempCapture ^= setMask[to];
			}
			if (enPassant > 23)
			{
				if (whitePawnAttacks[from] & setMask[enPassant])
				{
					m.setPromotion(WhitePawn); // this is necessary, see Move::cpp and Move::isEnpassant
					m.setCapture(BlackPawn);
					m.setTo(enPassant);
					moveStack[moveList[ply + 1]].score = 0;
					moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				}
			}
			tempPiece ^= setMask[from];
			m.setPromotion(Empty); 
		}
		
		// white knights
		m.setPiece(WhiteKnight);
		tempPiece = whiteKnights;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			tempMove = knightAttacks[from] & freeSquares;
			tempCapture = knightAttacks[from] & blackPieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// white bishops
		m.setPiece(WhiteBishop);
		tempPiece = whiteBishops;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempMove = magicMovesBishop[from][databaseIndex] & freeSquares;
			tempCapture = magicMovesBishop[from][databaseIndex] & blackPieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt =m.moveInt;
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt =m.moveInt;
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}

		// white rooks
		m.setPiece(WhiteRook);
		tempPiece = whiteRooks;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempMove = magicMovesRook[from][databaseIndex] & freeSquares;
			tempCapture = magicMovesRook[from][databaseIndex] & blackPieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// white queens
		m.setPiece(WhiteQueen);
		tempPiece = whiteQueens;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempMove = magicMovesRook[from][databaseIndex] & freeSquares;
			tempCapture = magicMovesRook[from][databaseIndex] & blackPieces;
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempMove |= magicMovesBishop[from][databaseIndex] & freeSquares;
			tempCapture |= magicMovesBishop[from][databaseIndex] & blackPieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// white king
		m.setPiece(WhiteKing);
		from = bitScanForward(whiteKing);
		m.setFrom(from);
		tempMove = kingAttacks[from] & freeSquares;
		tempCapture = kingAttacks[from] & blackPieces;
		m.setCapture(Empty);
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			m.setTo(to);
			moveStack[moveList[ply + 1]].score = 0;
			moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
			tempMove ^= setMask[to];
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			m.setTo(to);
			m.setCapture(getPiece(to));
			moveStack[moveList[ply + 1]].score = 0;
			moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
			tempCapture ^= setMask[to];
		}
		
		// castling moves
		if (castling & 1)
		{
			if (!((U64)0x0000000000000060 & occupied))
			{
				if (!(attack(E1, Black)))
				{
					if (!(attack(F1, Black))) 
					{
						if (!(attack(G1, Black)))
						{
							moveStack[moveList[ply + 1]].score = 0;
							moveStack[moveList[ply + 1]++].moveInt = whiteShortCastle;
						}
					}
				}
			}
		}
		if (castling & 2)
		{
			if (!((U64)0x000000000000000E & occupied))
			{
				if (!(attack(E1, Black)))
				{
					if (!(attack(D1, Black))) 
					{
						if (!(attack(C1, Black)))
						{
							moveStack[moveList[ply + 1]].score = 0;
							moveStack[moveList[ply + 1]++].moveInt = whiteLongCastle;
						}
					}
				}
			}
		}
		m.setPromotion(Empty);
	}
	else 
	{
		targetBitboard = ~blackPieces;
		
		// black pawns
		m.setPiece(BlackPawn);
		tempPiece = blackPawns;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			int first = bitScanReverse(blackPawnMoves[from] & occupiedSquares);
			tempMove = blackPawnMoves[from] & above[first];
			tempCapture = blackPawnAttacks[from] & whitePieces; 
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				if (to <= H1)
				{
					m.setPromotion(BlackQueen); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(BlackRook); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(BlackBishop); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(BlackKnight); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(Empty); // this is necessary
				}
				else 
				{
					moveStack[moveList[ply + 1]].score = 0;
					moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				}
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				if (to <= H1)
				{
					m.setPromotion(BlackQueen); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(BlackRook); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(BlackBishop); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(BlackKnight); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(Empty); // this is necessary
				}
				else 
				{
					moveStack[moveList[ply + 1]].score = 0;
					moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				}
				tempCapture ^= setMask[to];
			}
			if (enPassant < 40)
			{
				if (blackPawnAttacks[from] & setMask[enPassant])
				{
					m.setPromotion(BlackPawn); 
					m.setCapture(WhitePawn);
					m.setTo(enPassant);
					moveStack[moveList[ply + 1]].score = 0;
					moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				}
			}
			tempPiece ^= setMask[from];
			m.setPromotion(Empty); 
		}
		
		// black knights
		m.setPiece(BlackKnight);
		tempPiece = blackKnights;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			tempMove = knightAttacks[from] & freeSquares;
			tempCapture = knightAttacks[from] & whitePieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// black bishops
		m.setPiece(BlackBishop);
		tempPiece = blackBishops;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempMove = magicMovesBishop[from][databaseIndex] & freeSquares;
			tempCapture = magicMovesBishop[from][databaseIndex] & whitePieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}

		// black rooks
		m.setPiece(BlackRook);
		tempPiece = blackRooks;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempMove = magicMovesRook[from][databaseIndex] & freeSquares;
			tempCapture = magicMovesRook[from][databaseIndex] & whitePieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// black queens
		m.setPiece(BlackQueen);
		tempPiece = blackQueens;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempMove = magicMovesRook[from][databaseIndex] & freeSquares;
			tempCapture = magicMovesRook[from][databaseIndex] & whitePieces;
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempMove |= magicMovesBishop[from][databaseIndex] & freeSquares;
			tempCapture |= magicMovesBishop[from][databaseIndex] & whitePieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].score = 0;
				moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// black king
		m.setPiece(BlackKing);
		from = bitScanForward(blackKing);
		m.setFrom(from);
		tempMove = kingAttacks[from] & freeSquares;
		tempCapture = kingAttacks[from] & whitePieces;
		m.setCapture(Empty);
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			m.setTo(to);
			moveStack[moveList[ply + 1]].score = 0;
			moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
			tempMove ^= setMask[to];
		}
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			m.setTo(to);
			m.setCapture(getPiece(to));
			moveStack[moveList[ply + 1]].score = 0;
			moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
			tempCapture ^= setMask[to];
		}
		
		// castling moves
		if (castling & 4)
		{
			if (!((U64)0x6000000000000000 & occupied))
			{
				if (!(attack(E8, White) || attack(F8, White) || attack(G8, White)))
				{
					moveStack[moveList[ply + 1]].score = 0;
					moveStack[moveList[ply + 1]++].moveInt = blackShortCastle;
				}
			}
		}
		if (castling & 8)
		{
			if (!((U64)0x0E00000000000000 & occupied))
			{
				if (!(attack(E8, White) || attack(D8, White) || attack(C8, White)))
				{
					moveStack[moveList[ply + 1]].score = 0;
					moveStack[moveList[ply + 1]++].moveInt = blackLongCastle;
				}
			}
		}
		m.setPromotion(Empty);
	}
}

// generates captures and promotions for the quiescence search
void generateCaptures()
{
	int from, to;
	U64 blockers;
	U64 occupied = occupiedSquares;
	int databaseIndex, seeScore;
	U64 tempPiece, tempMove, tempCapture;
	U64 targetBitboard, freeSquares;
	Move m;
	
	m.clear();
	freeSquares = ~occupiedSquares;
	
	moveList[ply + 1] = moveList[ply];
	if (sideToMove == White)
	{
		targetBitboard = ~whitePieces;
		
		// White pawns
		m.setPiece(WhitePawn);
		tempPiece = whitePawns;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			tempMove = whitePawnMoves[from] & 0xFF00000000000000 & freeSquares;
			tempCapture = whitePawnAttacks[from] & blackPieces;
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);

				m.setPromotion(WhiteQueen); 
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}

				m.setPromotion(WhiteRook);
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}

				m.setPromotion(WhiteBishop); 
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}
				m.setPromotion(WhiteKnight); 
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}

				m.setPromotion(Empty); // this is necessary
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				if (to >= A8)
				{
					m.setPromotion(WhiteQueen); 
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}

					m.setPromotion(WhiteRook); 
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}

					m.setPromotion(WhiteBishop); 
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}

					m.setPromotion(WhiteKnight); 					
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}

					m.setPromotion(Empty); // this is necessary
				}
				else 
				{
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}
				}
				tempCapture ^= setMask[to];
			}
			if (enPassant > 23)
			{
				if (whitePawnAttacks[from] & setMask[enPassant])
				{
					m.setPromotion(WhitePawn); // this is necessary, see Move::cpp and Move::isEnpassant
					m.setCapture(BlackPawn);
					m.setTo(enPassant);
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}
				}
			}
			tempPiece ^= setMask[from];
			m.setPromotion(Empty); 
		}
		
		// white knights
		m.setPiece(WhiteKnight);
		tempPiece = whiteKnights;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			tempCapture = knightAttacks[from] & blackPieces;
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// white bishops
		m.setPiece(WhiteBishop);
		tempPiece = whiteBishops;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempCapture = magicMovesBishop[from][databaseIndex] & blackPieces;
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}

		// white rooks
		m.setPiece(WhiteRook);
		tempPiece = whiteRooks;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempCapture = magicMovesRook[from][databaseIndex] & blackPieces;
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// white queens
		m.setPiece(WhiteQueen);
		tempPiece = whiteQueens;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempCapture = magicMovesRook[from][databaseIndex] & blackPieces;
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempCapture |= magicMovesBishop[from][databaseIndex] & blackPieces;
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// white king
		m.setPiece(WhiteKing);
		from = bitScanForward(whiteKing);
		m.setFrom(from);
		tempCapture = kingAttacks[from] & blackPieces;
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			m.setTo(to);
			m.setCapture(getPiece(to));
			seeScore = SEE(m); 
			if (seeScore >= 0) 
			{
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = seeScore;
			}
			tempCapture ^= setMask[to];
		}
		m.setPromotion(Empty);
	}
	else 
	{
		targetBitboard = ~blackPieces;
		
		// black pawns
		m.setPiece(BlackPawn);
		tempPiece = blackPawns;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			tempMove = blackPawnMoves[from] & 0x00000000000000FF & freeSquares;
			tempCapture = blackPawnAttacks[from] & whitePieces; 
			m.setCapture(Empty);
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);

				m.setPromotion(BlackQueen); 
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}

				m.setPromotion(BlackRook); 
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}

				m.setPromotion(BlackBishop); 
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}

				m.setPromotion(BlackKnight); 					
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}

				m.setPromotion(Empty); // this is necessary
				tempMove ^= setMask[to];
			}
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				if (to <= H1)
				{
					m.setPromotion(BlackQueen); 
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}

					m.setPromotion(BlackRook); 
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}

					m.setPromotion(BlackBishop); 
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}

					m.setPromotion(BlackKnight); 					
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}
					m.setPromotion(Empty); // this is necessary
				}
				else 
				{
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}
				}
				tempCapture ^= setMask[to];
			}
			if (enPassant < 40)
			{
				if (blackPawnAttacks[from] & setMask[enPassant])
				{
					m.setPromotion(BlackPawn); 
					m.setCapture(WhitePawn);
					m.setTo(enPassant);
					seeScore = SEE(m); 
					if (seeScore >= 0) 
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = seeScore;
					}
				}
			}
			tempPiece ^= setMask[from];
			m.setPromotion(Empty); 
		}
		
		// black knights
		m.setPiece(BlackKnight);
		tempPiece = blackKnights;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			tempCapture = knightAttacks[from] & whitePieces;
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// black bishops
		m.setPiece(BlackBishop);
		tempPiece = blackBishops;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempCapture = magicMovesBishop[from][databaseIndex] & whitePieces;
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}

		// black rooks
		m.setPiece(BlackRook);
		tempPiece = blackRooks;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempCapture = magicMovesRook[from][databaseIndex] & whitePieces;
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// black queens
		m.setPiece(BlackQueen);
		tempPiece = blackQueens;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempCapture = magicMovesRook[from][databaseIndex] & whitePieces;
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempCapture |= magicMovesBishop[from][databaseIndex] & whitePieces;
			while (tempCapture)
			{
				to = bitScanForward(tempCapture);
				m.setTo(to);
				m.setCapture(getPiece(to));
				seeScore = SEE(m); 
				if (seeScore >= 0) 
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = seeScore;
				}
				tempCapture ^= setMask[to];
			}
			tempPiece ^= setMask[from];
		}
		
		// black king
		m.setPiece(BlackKing);
		from = bitScanForward(blackKing);
		m.setFrom(from);
		tempCapture = kingAttacks[from] & whitePieces;
		while (tempCapture)
		{
			to = bitScanForward(tempCapture);
			m.setTo(to);
			m.setCapture(getPiece(to));
			seeScore = SEE(m); 
			if (seeScore >= 0) 
			{
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = seeScore;
			}
			tempCapture ^= setMask[to];
		}

		m.setPromotion(Empty);
	}
}

void generateEvasions()
{
	int from, to;
	U64 blockers;
	U64 occupied = occupiedSquares;
	int databaseIndex;
	U64 tempPiece, tempMove;
	U64 targetBitboard, freeSquares;
	U64 pinned = 0;
	Move m;

	m.clear();
	freeSquares = ~occupiedSquares;
	
	moveList[ply + 1] = moveList[ply];
	if (sideToMove == White)
	{
		targetBitboard = ~whitePieces;

		// white king
		m.setPiece(WhiteKing);
		from = bitScanForward(whiteKing);
		m.setFrom(from);
		tempMove = kingAttacks[from] & targetBitboard;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove ^= setMask[to];
			occupiedSquares ^= setMask[from] ^ setMask[to];
			if (attack(to, Black))
			{
				occupiedSquares ^= setMask[from] ^ setMask[to];
				continue;
			}
			occupiedSquares ^= setMask[from] ^ setMask[to];
			m.setTo(to);
			m.setCapture(getPiece(to));
			moveStack[moveList[ply + 1]].moveInt = m.moveInt;
			moveStack[moveList[ply + 1]++].score = 0;
		}

		// double check
		U64 checkers = inCheck(White);
		if (popcnt(checkers) > 1)
			return;

		// find the pinned pieces
		U64 b = (RayNE[from] | RaySE[from] | RayNW[from] | RaySW[from]) & (blackBishops | blackQueens);
		while (b)
		{
			int pinner = bitScanForward(b);
			b ^= setMask[pinner];
			U64 potentialPinned = (between[from][pinner] & occupiedSquares);
			if (popcnt(potentialPinned) == 1)
				pinned |= potentialPinned & whitePieces;
		}
		b = (RayN[from] | RayS[from] | RayW[from] | RayE[from]) & (blackRooks | blackQueens);
		while (b)
		{
			int pinner = bitScanForward(b);
			b ^= setMask[pinner];
			U64 potentialPinned = (between[from][pinner] & occupiedSquares);
			if (popcnt(potentialPinned) == 1)
				pinned |= potentialPinned & whitePieces;
		}
		
		m.setPiece(WhitePawn);
		tempPiece = whitePawns;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			tempMove = whitePawnAttacks[from] & checkers;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				tempMove ^= setMask[to];
				m.setTo(to);
				m.setCapture(getPiece(to));
				if (to >= A8)
				{
					m.setPromotion(WhiteQueen); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteRook); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteBishop); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteKnight); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(Empty); // this is necessary
				}
				else
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = 0;
				}
			}
			// this part might contain an error
			if (enPassant > 23)
			{
				if (whitePawnAttacks[from] & setMask[enPassant])
				{
					if (bitScanForward(checkers) == (enPassant - 8))
					{
						m.setPromotion(WhitePawn); // this is necessary, see Move::cpp and Move::isEnpassant
						m.setCapture(BlackPawn);
						m.setTo(enPassant);
						moveStack[moveList[ply + 1]].score = 0;
						moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					}
				}
			}

			U64 interpose = between[bitScanForward(whiteKing)][bitScanForward(checkers)];
			if (interpose)
			{
				int first = bitScanForward(whitePawnMoves[from] & occupiedSquares);
				tempMove = whitePawnMoves[from] & below[first] & interpose;
				while (tempMove)
				{
					to = bitScanForward(tempMove);
					tempMove ^= setMask[to];
					m.setTo(to);
					m.setCapture(Empty);
					if (to >= A8)
					{
						m.setPromotion(WhiteQueen); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
						m.setPromotion(WhiteRook); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
						m.setPromotion(WhiteBishop); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
						m.setPromotion(WhiteKnight); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
						m.setPromotion(Empty); // this is necessary
					}
					else
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = 0;
					}
				}
			}
		}

		U64 interpose = between[bitScanForward(whiteKing)][bitScanForward(checkers)] | setMask[bitScanForward(checkers)];

		// white knights
		m.setPiece(WhiteKnight);
		tempPiece = whiteKnights;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			tempMove = knightAttacks[from] & interpose;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = 0;
				tempMove ^= setMask[to];
			}
		}

		// white bishops
		m.setPiece(WhiteBishop);
		tempPiece = whiteBishops;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempMove = magicMovesBishop[from][databaseIndex] & interpose;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = 0;
				tempMove ^= setMask[to];
			}	
		}

		// white rooks
		m.setPiece(WhiteRook);
		tempPiece = whiteRooks;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempMove = magicMovesRook[from][databaseIndex] & interpose;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = 0;
				tempMove ^= setMask[to];
			}		
		}

		// white queens
		m.setPiece(WhiteQueen);
		tempPiece = whiteQueens;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempMove = magicMovesRook[from][databaseIndex] & interpose;
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempMove |= magicMovesBishop[from][databaseIndex] & interpose;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = 0;
				tempMove ^= setMask[to];
			}	
		}
	}
	else
	{
		targetBitboard = ~blackPieces;

		// black king
		m.setPiece(BlackKing);
		from = bitScanForward(blackKing);
		m.setFrom(from);
		tempMove = kingAttacks[from] & targetBitboard;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove ^= setMask[to];
			occupiedSquares ^= setMask[from] ^ setMask[to];
			if (attack(to, White))
			{
				occupiedSquares ^= setMask[from] ^ setMask[to];
				continue;
			}
			occupiedSquares ^= setMask[from] ^ setMask[to];
			m.setTo(to);
			m.setCapture(getPiece(to));
			moveStack[moveList[ply + 1]].moveInt = m.moveInt;
			moveStack[moveList[ply + 1]++].score = 0;
		}

		// double check
		U64 checkers = inCheck(Black);
		if (popcnt(checkers) > 1)
			return;

		// find the pinned pieces
		U64 b = (RayNE[from] | RaySE[from] | RayNW[from] | RaySW[from]) & (whiteBishops | whiteQueens);
		while (b)
		{
			int pinner = bitScanForward(b);
			b ^= setMask[pinner];
			U64 potentialPinned = (between[from][pinner] & occupiedSquares);
			if (popcnt(potentialPinned) == 1)
				pinned |= potentialPinned & blackPieces;
		}
		b = (RayN[from] | RayS[from] | RayW[from] | RayE[from]) & (whiteRooks | whiteQueens);
		while (b)
		{
			int pinner = bitScanForward(b);
			b ^= setMask[pinner];
			U64 potentialPinned = (between[from][pinner] & occupiedSquares);
			if (popcnt(potentialPinned) == 1)
				pinned |= potentialPinned & blackPieces;
		}
		
		m.setPiece(BlackPawn);
		tempPiece = blackPawns;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			tempMove = blackPawnAttacks[from] & checkers;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				tempMove ^= setMask[to];
				m.setTo(to);
				m.setCapture(getPiece(to));
				if (to <= H1)
				{
					m.setPromotion(WhiteQueen); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteRook); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteBishop); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(WhiteKnight); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					m.setPromotion(Empty); // this is necessary
				}
				else
				{
					moveStack[moveList[ply + 1]].moveInt = m.moveInt;
					moveStack[moveList[ply + 1]++].score = 0;
				}
			}
			// this part might contain an error
			if (enPassant < 40)
			{
				if (blackPawnAttacks[from] & setMask[enPassant])
				{
					if (bitScanForward(checkers) == (enPassant + 8))
					{
						m.setPromotion(BlackPawn); // this is necessary, see Move::cpp and Move::isEnpassant
						m.setCapture(WhitePawn);
						m.setTo(enPassant);
						moveStack[moveList[ply + 1]].score = 0;
						moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
					}
				}
			}

			U64 interpose = between[bitScanForward(blackKing)][bitScanForward(checkers)];
			if (interpose)
			{
				int first = bitScanReverse(blackPawnMoves[from] & occupiedSquares);
				tempMove = blackPawnMoves[from] & above[first] & interpose;
				while (tempMove)
				{
					to = bitScanForward(tempMove);
					tempMove ^= setMask[to];
					m.setTo(to);
					m.setCapture(Empty);
					if (to >= A8)
					{
						m.setPromotion(WhiteQueen); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
						m.setPromotion(WhiteRook); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
						m.setPromotion(WhiteBishop); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
						m.setPromotion(WhiteKnight); moveStack[moveList[ply + 1]].score = 0; moveStack[moveList[ply + 1]++].moveInt = m.moveInt;
						m.setPromotion(Empty); // this is necessary
					}
					else
					{
						moveStack[moveList[ply + 1]].moveInt = m.moveInt;
						moveStack[moveList[ply + 1]++].score = 0;
					}
				}
			}
		}

		U64 interpose = between[bitScanForward(blackKing)][bitScanForward(checkers)] | setMask[bitScanForward(checkers)];

		// black knights
		m.setPiece(BlackKnight);
		tempPiece = blackKnights;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			tempMove = knightAttacks[from] & interpose;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = 0;
				tempMove ^= setMask[to];
			}
		}

		// black bishops
		m.setPiece(BlackBishop);
		tempPiece = blackBishops;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempMove = magicMovesBishop[from][databaseIndex] & interpose;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = 0;
				tempMove ^= setMask[to];
			}	
		}

		// black rooks
		m.setPiece(BlackRook);
		tempPiece = blackRooks;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempMove = magicMovesRook[from][databaseIndex] & interpose;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = 0;
				tempMove ^= setMask[to];
			}		
		}

		// black queens
		m.setPiece(BlackQueen);
		tempPiece = blackQueens;
		while (tempPiece)
		{
			from = bitScanForward(tempPiece);
			tempPiece ^= setMask[from];
			if (setMask[from] & pinned)
				continue;
			m.setFrom(from);
			blockers = occupied & occupancyMaskRook[from];
			databaseIndex = (int)((blockers * magicNumberRook[from]) >> rookMagicShifts[from]);
			tempMove = magicMovesRook[from][databaseIndex] & interpose;
			blockers = occupied & occupancyMaskBishop[from];
			databaseIndex = (int)((blockers * magicNumberBishop[from]) >> bishopMagicShifts[from]);
			tempMove |= magicMovesBishop[from][databaseIndex] & interpose;
			while (tempMove)
			{
				to = bitScanForward(tempMove);
				m.setTo(to);
				m.setCapture(getPiece(to));
				moveStack[moveList[ply + 1]].moveInt = m.moveInt;
				moveStack[moveList[ply + 1]++].score = 0;
				tempMove ^= setMask[to];
			}	
		}
	}
}

bool make(int mov)
{
	Move m;
	m.moveInt = mov;
	int from = m.getFrom();
	int to = m.getTo();
	int piece = m.getPiece();
	int captured = m.getCapture();
	
	U64 fromBB = setMask[from];
	U64 fromToBB = fromBB | setMask[to];

	historyStack[hply].castle = castling;
	historyStack[hply].ep = enPassant;
	historyStack[hply].fifty = fiftyMoveDistance;
	historyStack[hply].Hash = Hash;
	historyStack[hply].pHash = pHash;
	++ply;
	++hply;
	
	Hash ^= (pieceHash[piece][from] ^ pieceHash[piece][to]);

	if (enPassant != 64)
		Hash ^= ep[enPassant];

	if (sideToMove == White) {
		if (piece == WhitePawn)
		{
			whitePawns ^= fromToBB;
			scoreOpeningPST -= pawnPSTOpening[from];
			scoreOpeningPST += pawnPSTOpening[to];
			scoreEndingPST -= pawnPSTEnding[from];
			scoreEndingPST += pawnPSTEnding[to];
			pHash ^= (pieceHash[WhitePawn][from] ^ pieceHash[WhitePawn][to]);
			whitePieces ^= fromToBB;
			if (from <= H2 && to >= A4)
			{
				enPassant = from + 8;
				Hash ^= ep[from + 8];
			}
			else 
			{	
				enPassant = 64;
			}
			fiftyMoveDistance = 0;

			if (captured)
			{ 
				if (m.isEnpassant())
				{
					blackPawns ^= setMask[to-8];
					blackPieces ^= setMask[to-8];
					scoreOpeningPST += pawnPSTOpening[flip[to-8]];
					scoreEndingPST += pawnPSTEnding[flip[to-8]];
					occupiedSquares ^= fromToBB | setMask[to-8];
					bp -= 1;
					materialOpening += pawnOpening;
					materialEnding += pawnEnding;
					Hash ^= pieceHash[BlackPawn][to - 8];
					pHash ^= pieceHash[BlackPawn][to - 8];
				}
				else 
				{
					makeCapture(captured, to);
					occupiedSquares ^= fromBB;
				}
			}
			else  occupiedSquares ^= fromToBB;
		
			if (m.isPromotion())
			{
				makeWhitePromotion(m.getPromotion(), to);
			}
		}
		else if (piece == WhiteKing)
		{
			whiteKing ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST -= kingPSTOpening[from];
			scoreOpeningPST += kingPSTOpening[to];
			scoreEndingPST -= kingPSTEnding[from];
			scoreEndingPST += kingPSTEnding[to];
			enPassant = 64;
			fiftyMoveDistance++;
			if (castling & 1)
				Hash ^= castle[1];
			if (castling & 2)
				Hash ^= castle[2];
			castling &= 12;
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		
			if (m.isCastle())
			{
				if (m.isCastleOO())
				{
					whiteRooks ^= setMask[H1] | setMask[F1];
					whitePieces ^= setMask[H1] | setMask[F1];
					occupiedSquares ^= setMask[H1] | setMask[F1];
					scoreOpeningPST -= rookPSTOpening[H1];
					scoreOpeningPST += rookPSTOpening[F1];
					scoreEndingPST -= rookPSTEnding[H1];
					scoreEndingPST += rookPSTEnding[F1];
					Hash ^= (pieceHash[WhiteRook][H1] ^ pieceHash[WhiteRook][F1]);
				}
				else 
				{
					whiteRooks ^= setMask[A1] | setMask[D1];
					whitePieces ^= setMask[A1] | setMask[D1];
					occupiedSquares ^= setMask[A1] | setMask[D1];
					scoreOpeningPST -= rookPSTOpening[A1];
					scoreOpeningPST += rookPSTOpening[D1];
					scoreEndingPST -= rookPSTEnding[A1];
					scoreEndingPST += rookPSTEnding[D1];
					Hash ^= (pieceHash[WhiteRook][A1] ^ pieceHash[WhiteRook][D1]);
				}
			}
		}
		else if (piece == WhiteKnight)
		{
			whiteKnights ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST -= knightPSTOpening[from];
			scoreOpeningPST += knightPSTOpening[to];
			scoreEndingPST -= knightPSTEnding[from];
			scoreEndingPST += knightPSTEnding[to];
			enPassant = 64;
			fiftyMoveDistance++;
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == WhiteBishop)
		{
			whiteBishops ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST -= bishopPSTOpening[from];
			scoreOpeningPST += bishopPSTOpening[to];
			scoreEndingPST -= bishopPSTEnding[from];
			scoreEndingPST += bishopPSTEnding[to];
			enPassant = 64;
			fiftyMoveDistance++;
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}	
		else if (piece == WhiteRook)
		{
			whiteRooks ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST -= rookPSTOpening[from];
			scoreOpeningPST += rookPSTOpening[to];
			scoreEndingPST -= rookPSTEnding[from];
			scoreEndingPST += rookPSTEnding[to];
			enPassant = 64;
			fiftyMoveDistance++;
			if (from == A1)
			{
				if (castling & 2)
				{
					Hash ^= castle[2];
					castling &= 13;
				}
			}
			if (from == H1)
			{
				if (castling & 1)
				{
					Hash ^= castle[1];
					castling &= 14;
				}
			}
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == WhiteQueen)
		{
			whiteQueens ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST -= queenPSTOpening[from];
			scoreOpeningPST += queenPSTOpening[to];
			scoreEndingPST -= queenPSTEnding[from];
			scoreEndingPST += queenPSTEnding[to];
			enPassant = 64;
			fiftyMoveDistance++;
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}	
	}
	else 
	{
		if (piece == BlackPawn)
		{
			blackPawns ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST += pawnPSTOpening[flip[from]];
			scoreOpeningPST -= pawnPSTOpening[flip[to]];
			scoreEndingPST += pawnPSTEnding[flip[from]];
			scoreEndingPST -= pawnPSTEnding[flip[to]];
			pHash ^= (pieceHash[BlackPawn][from] ^ pieceHash[BlackPawn][to]);
			enPassant = 64;
			fiftyMoveDistance = 0;
			if (from >= A7 && to <= H5)
			{
				enPassant = from - 8;
				Hash ^= ep[from - 8];
			}
			if (captured)
			{ 
				if (m.isEnpassant())
				{
					whitePawns ^= setMask[to+8];
					whitePieces ^= setMask[to+8];
					scoreOpeningPST -= pawnPSTOpening[to+8];
					scoreEndingPST -= pawnPSTEnding[to+8];
					occupiedSquares ^= fromToBB | setMask[to+8];
					wp -= 1;
					materialOpening -= pawnOpening;
					materialEnding -= pawnEnding;
					Hash ^= pieceHash[WhitePawn][to + 8];
					pHash ^= pieceHash[WhitePawn][to + 8];
					
				}
				else 
				{
					makeCapture(captured, to);
					occupiedSquares ^= fromBB;
				}
			}
			else  occupiedSquares ^= fromToBB;
		
			if (m.isPromotion())
			{
				makeBlackPromotion(m.getPromotion(), to);
			}
		}
		else if (piece == BlackKing)
		{
			blackKing ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST += kingPSTOpening[flip[from]];
			scoreOpeningPST -= kingPSTOpening[flip[to]];
			scoreEndingPST += kingPSTEnding[flip[from]];
			scoreEndingPST -= kingPSTEnding[flip[to]];
			enPassant = 64;
			fiftyMoveDistance++;
			if (castling & 4)
				Hash ^= castle[4];
			if (castling & 8)
				Hash ^= castle[8];
			castling &= 3;
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}	
			else occupiedSquares ^= fromToBB;
			
			if (m.isCastle())
			{
				if (m.isCastleOO())
				{
					blackRooks ^= setMask[H8] | setMask[F8];
					blackPieces ^= setMask[H8] | setMask[F8];
					occupiedSquares ^= setMask[H8] | setMask[F8];
					scoreOpeningPST += rookPSTOpening[flip[H8]];
					scoreOpeningPST -= rookPSTOpening[flip[F8]];
					scoreEndingPST += rookPSTEnding[flip[H8]];
					scoreEndingPST -= rookPSTEnding[flip[F8]];
					Hash ^= (pieceHash[BlackRook][H8] ^ pieceHash[BlackRook][F8]);
				}
				else 
				{
					blackRooks ^= setMask[A8] | setMask[D8];
					blackPieces ^= setMask[A8] | setMask[D8];
					occupiedSquares ^= setMask[A8] | setMask[D8];
					scoreOpeningPST += rookPSTOpening[flip[A8]];
					scoreOpeningPST -= rookPSTOpening[flip[D8]];
					scoreEndingPST += rookPSTEnding[flip[A8]];
					scoreEndingPST -= rookPSTEnding[flip[D8]];
					Hash ^= (pieceHash[BlackRook][A8] ^ pieceHash[BlackRook][D8]);
				}
			}
		}
		else if (piece == BlackKnight)
		{
			blackKnights ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST += knightPSTOpening[flip[from]];
			scoreOpeningPST -= knightPSTOpening[flip[to]];
			scoreEndingPST += knightPSTEnding[flip[from]];
			scoreEndingPST -= knightPSTEnding[flip[to]];
			enPassant = 64;
			fiftyMoveDistance++;
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == BlackBishop)
		{
			blackBishops ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST += bishopPSTOpening[flip[from]];
			scoreOpeningPST -= bishopPSTOpening[flip[to]];
			scoreEndingPST += bishopPSTEnding[flip[from]];
			scoreEndingPST -= bishopPSTEnding[flip[to]];
			enPassant = 64;
			fiftyMoveDistance++;
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == BlackRook)
		{
			blackRooks ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST += rookPSTOpening[flip[from]];
			scoreOpeningPST -= rookPSTOpening[flip[to]];
			scoreEndingPST += rookPSTEnding[flip[from]];
			scoreEndingPST -= rookPSTEnding[flip[to]];
			enPassant = 64;
			fiftyMoveDistance++;			
			if (from == A8)
			{
				if (castling & 8)
				{
					Hash ^= castle[8];
					castling &= 7;
				}
			}
			if (from == H8)
			{
				if (castling & 4)
				{
					Hash ^= castle[4];
					castling &= 11;
				}
			}
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == BlackQueen)
		{
			blackQueens ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST += queenPSTOpening[flip[from]];
			scoreOpeningPST -= queenPSTOpening[flip[to]];
			scoreEndingPST += queenPSTEnding[flip[from]];
			scoreEndingPST -= queenPSTEnding[flip[to]];
			enPassant = 64;
			fiftyMoveDistance++;
			if (captured)
			{
				makeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
	}

	sideToMove ^= 1;
	Hash ^= side;

	if (inCheck(sideToMove ^ 1)) {
		unmake(mov);
		return false;
	}
	return true;
}

void unmake(int mov)
{
	Move m;
	m.moveInt = mov;
	int piece = m.getPiece();
	int captured = m.getCapture();
	int from = m.getFrom();
	int to = m.getTo();

	
	U64 fromBB = setMask[from];
	U64 fromToBB = fromBB | setMask[to];

    --ply;
	--hply;
	castling = historyStack[hply].castle;
	enPassant = historyStack[hply].ep;
	fiftyMoveDistance = historyStack[hply].fifty; 
	Hash = historyStack[hply].Hash;
	pHash = historyStack[hply].pHash;
	sideToMove ^= 1;
	
	if (sideToMove == White) {
		if (piece == WhitePawn)
		{
			whitePawns ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST += pawnPSTOpening[from];
			scoreOpeningPST -= pawnPSTOpening[to];
			scoreEndingPST += pawnPSTEnding[from];
			scoreEndingPST -= pawnPSTEnding[to];
			if (captured)
			{ 
				if (m.isEnpassant())
				{
					blackPawns ^= setMask[to-8];
					blackPieces ^= setMask[to-8];
					scoreOpeningPST -= pawnPSTOpening[flip[to-8]];
					scoreEndingPST -= pawnPSTEnding[flip[to-8]];
					occupiedSquares ^= fromToBB | setMask[to-8];
					bp += 1;
					materialOpening -= pawnOpening;
					materialEnding -= pawnEnding;
				}
				else 
				{
					unmakeCapture(captured, to);
					occupiedSquares ^= fromBB;
				}
			}
			else  occupiedSquares ^= fromToBB;
		
			if (m.isPromotion())
			{
				unmakeWhitePromotion(m.getPromotion(), to);
			}
		}
		else if (piece == WhiteKing)
		{
			whiteKing ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST += kingPSTOpening[from];
			scoreOpeningPST -= kingPSTOpening[to];
			scoreEndingPST += kingPSTEnding[from];
			scoreEndingPST -= kingPSTEnding[to];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		
			if (m.isCastle())
			{
				if (m.isCastleOO())
				{
					whiteRooks ^= setMask[H1] | setMask[F1];
					whitePieces ^= setMask[H1] | setMask[F1];
					occupiedSquares ^= setMask[H1] | setMask[F1];
					scoreOpeningPST += rookPSTOpening[H1];
					scoreOpeningPST -= rookPSTOpening[F1];
					scoreEndingPST += rookPSTEnding[H1];
					scoreEndingPST -= rookPSTEnding[F1];
				}
				else 
				{
					whiteRooks ^= setMask[A1] | setMask[D1];
					whitePieces ^= setMask[A1] | setMask[D1];
					occupiedSquares ^= setMask[A1] | setMask[D1];
					scoreOpeningPST += rookPSTOpening[A1];
					scoreOpeningPST -= rookPSTOpening[D1];
					scoreEndingPST += rookPSTEnding[A1];
					scoreEndingPST -= rookPSTEnding[D1];
				}
			}
		}
		else if (piece == WhiteKnight)
		{
			whiteKnights ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST += knightPSTOpening[from];
			scoreOpeningPST -= knightPSTOpening[to];
			scoreEndingPST += knightPSTEnding[from];
			scoreEndingPST -= knightPSTEnding[to];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == WhiteBishop)
		{
			whiteBishops ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST += bishopPSTOpening[from];
			scoreOpeningPST -= bishopPSTOpening[to];
			scoreEndingPST += bishopPSTEnding[from];
			scoreEndingPST -= bishopPSTEnding[to];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}	
		else if (piece == WhiteRook)
		{
			whiteRooks ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST += rookPSTOpening[from];
			scoreOpeningPST -= rookPSTOpening[to];
			scoreEndingPST += rookPSTEnding[from];
			scoreEndingPST -= rookPSTEnding[to];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == WhiteQueen)
		{
			whiteQueens ^= fromToBB;
			whitePieces ^= fromToBB;
			scoreOpeningPST += queenPSTOpening[from];
			scoreOpeningPST -= queenPSTOpening[to];
			scoreEndingPST += queenPSTEnding[from];
			scoreEndingPST -= queenPSTEnding[to];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}	
	}
	else 
	{
		if (piece == BlackPawn)
		{
			blackPawns ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST -= pawnPSTOpening[flip[from]];
			scoreOpeningPST += pawnPSTOpening[flip[to]];
			scoreEndingPST -= pawnPSTEnding[flip[from]];
			scoreEndingPST += pawnPSTEnding[flip[to]];
			if (captured)
			{ 
				if (m.isEnpassant())
				{
					whitePawns ^= setMask[to+8];
					whitePieces ^= setMask[to+8];
					occupiedSquares ^= fromToBB | setMask[to+8];
					scoreOpeningPST += pawnPSTOpening[to+8];
					scoreEndingPST += pawnPSTEnding[to+8];
					wp += 1;
					materialOpening += pawnOpening;
					materialEnding += pawnEnding;
				}
				else 
				{
					unmakeCapture(captured, to);
					occupiedSquares ^= fromBB;
				}
			}
			else occupiedSquares ^= fromToBB;
		
			if (m.isPromotion())
			{
				unmakeBlackPromotion(m.getPromotion(), to);
			}
		}
		else if (piece == BlackKing)
		{
			blackKing ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST -= kingPSTOpening[flip[from]];
			scoreOpeningPST += kingPSTOpening[flip[to]];
			scoreEndingPST -= kingPSTEnding[flip[from]];
			scoreEndingPST += kingPSTEnding[flip[to]];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}	
			else occupiedSquares ^= fromToBB;
			
			if (m.isCastle())
			{
				if (m.isCastleOO())
				{
					blackRooks ^= setMask[H8] | setMask[F8];
					blackPieces ^= setMask[H8] | setMask[F8];
					occupiedSquares ^= setMask[H8] | setMask[F8];
					scoreOpeningPST -= rookPSTOpening[flip[H8]];
					scoreOpeningPST += rookPSTOpening[flip[F8]];
					scoreEndingPST -= rookPSTEnding[flip[H8]];
					scoreEndingPST += rookPSTEnding[flip[F8]];
				}
				else 
				{
					blackRooks ^= setMask[A8] | setMask[D8];
					blackPieces ^= setMask[A8] | setMask[D8];
					occupiedSquares ^= setMask[A8] | setMask[D8];
					scoreOpeningPST -= rookPSTOpening[flip[A8]];
					scoreOpeningPST += rookPSTOpening[flip[D8]];
					scoreEndingPST -= rookPSTEnding[flip[A8]];
					scoreEndingPST += rookPSTEnding[flip[D8]];
				}
			}
		}
		else if (piece == BlackKnight)
		{
			blackKnights ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST -= knightPSTOpening[flip[from]];
			scoreOpeningPST += knightPSTOpening[flip[to]];
			scoreEndingPST -= knightPSTEnding[flip[from]];
			scoreEndingPST += knightPSTEnding[flip[to]];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == BlackBishop)
		{
			blackBishops ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST -= bishopPSTOpening[flip[from]];
			scoreOpeningPST += bishopPSTOpening[flip[to]];
			scoreEndingPST -= bishopPSTEnding[flip[from]];
			scoreEndingPST += bishopPSTEnding[flip[to]];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == BlackRook)
		{
			blackRooks ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST -= rookPSTOpening[flip[from]];
			scoreOpeningPST += rookPSTOpening[flip[to]];
			scoreEndingPST -= rookPSTEnding[flip[from]];
			scoreEndingPST += rookPSTEnding[flip[to]];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
		else if (piece == BlackQueen)
		{
			blackQueens ^= fromToBB;
			blackPieces ^= fromToBB;
			scoreOpeningPST -= queenPSTOpening[flip[from]];
			scoreOpeningPST += queenPSTOpening[flip[to]];
			scoreEndingPST -= queenPSTEnding[flip[from]];
			scoreEndingPST += queenPSTEnding[flip[to]];
			if (captured)
			{
				unmakeCapture(captured, to);
				occupiedSquares ^= fromBB;
			}
			else occupiedSquares ^= fromToBB;
		}
	}
}

void makeCapture(int &captured, int &to)
{
	U64 toBB = setMask[to];
	Hash ^= pieceHash[captured][to];
		
	if (captured == WhitePawn)
	{
		whitePawns ^= toBB;
		whitePieces ^= toBB;
		scoreOpeningPST -= pawnPSTOpening[to];
		scoreEndingPST -= pawnPSTEnding[to];
		pHash ^= pieceHash[WhitePawn][to];
		wp -= 1;
		materialOpening -= pawnOpening;
		materialEnding -= pawnEnding;
	}
	else if (captured == BlackPawn)
	{
		blackPawns ^= toBB;
		blackPieces ^= toBB;
		scoreOpeningPST += pawnPSTOpening[flip[to]];
		scoreEndingPST += pawnPSTEnding[flip[to]];
		pHash ^= pieceHash[BlackPawn][to];
		bp -= 1;
	    materialOpening += pawnOpening;
		materialEnding += pawnEnding;
	}
	else if (captured == WhiteKnight)
	{
		whiteKnights ^= toBB;
		whitePieces ^= toBB;
		scoreOpeningPST -= knightPSTOpening[to];
		scoreEndingPST -= knightPSTEnding[to];
		wn -= 1;
		materialOpening -= knightOpening;
		materialEnding -= knightEnding;
	}
	else if (captured == BlackKnight)
	{
		blackKnights ^= toBB;
		blackPieces ^= toBB;
		scoreOpeningPST += knightPSTOpening[flip[to]];
		scoreEndingPST += knightPSTEnding[flip[to]];
		bn -= 1;
		materialOpening += knightOpening;
		materialEnding += knightEnding;
	}
	else if (captured == WhiteBishop)
	{
		whiteBishops ^= toBB;
		whitePieces ^= toBB;
		scoreOpeningPST -= bishopPSTOpening[to];
		scoreEndingPST -= bishopPSTEnding[to];
		wb -= 1;
		materialOpening -= bishopOpening;
		materialEnding -= bishopEnding;
	}
	else if (captured == BlackBishop)
	{
		blackBishops ^= toBB;
		blackPieces ^= toBB;
		scoreOpeningPST += bishopPSTOpening[flip[to]];
		scoreEndingPST += bishopPSTEnding[flip[to]];
		bb -= 1;
		materialOpening += bishopOpening;
		materialEnding += bishopEnding;
	}
	else if (captured == WhiteRook)
	{
		whiteRooks ^= toBB;
		whitePieces ^= toBB;
		scoreOpeningPST -= rookPSTOpening[to];
		scoreEndingPST -= rookPSTEnding[to];
		wr -= 1;
		if (to == A1)
		{
			if (castling & 2)
			{
				castling &= 13;
				Hash ^= castle[2];
			}
		}
		if (to == H1)
		{
			if (castling & 1)
			{
				castling &= 14;
				Hash ^= castle[1];
			}
		}
		materialOpening -= rookOpening;
		materialEnding -= rookEnding;
	}
	else if (captured == BlackRook)
	{
		blackRooks ^= toBB;
		blackPieces ^= toBB;
		scoreOpeningPST += rookPSTOpening[flip[to]];
		scoreEndingPST += rookPSTEnding[flip[to]];
		br -= 1;
		if (to == A8)
		{
			if (castling & 8)
			{
				castling &= 7;
				Hash ^= castle[8];
			}
		}
		if (to == H8)
		{
			if (castling & 4)
			{
				castling &= 11;
				Hash ^= castle[4];
			}
		}
		materialOpening += rookOpening;
		materialEnding += rookEnding;
	}
	else if (captured == WhiteQueen)
	{
		whiteQueens ^= toBB;
		whitePieces ^= toBB;
		scoreOpeningPST -= queenPSTOpening[to];
		scoreEndingPST -= queenPSTEnding[to];
		wq -= 1;
		materialOpening -= queenOpening;
		materialEnding -= queenEnding;
	}
	else if (captured == BlackQueen)
	{
		blackQueens ^= toBB;
		blackPieces ^= toBB;
		scoreOpeningPST += queenPSTOpening[flip[to]];
		scoreEndingPST += queenPSTEnding[flip[to]];
		bq -= 1;
		materialOpening += queenOpening;
		materialEnding += queenEnding;
	}
	fiftyMoveDistance = 0;

	// update the phase of the game
	phase = totalPhase;

//	phase -= wp * pawnPhase;
//	phase -= bp * pawnPhase;
	phase -= wn * knightPhase;
	phase -= bn * knightPhase;
	phase -= wb * bishopPhase;
	phase -= bb * bishopPhase;
	phase -= wr * rookPhase;
	phase -= br * rookPhase;
	phase -= wq * queenPhase;
	phase -= bq * queenPhase;

	phase = (phase * 256 + (totalPhase / 2)) / totalPhase;
}

void unmakeCapture(int &captured, int &to)
{
	U64 toBB = setMask[to];
	if (captured == WhitePawn)
	{
		whitePawns ^= toBB;
		whitePieces ^= toBB;
		wp += 1;
		materialOpening += pawnOpening;
		materialEnding += pawnEnding;
		scoreOpeningPST += pawnPSTOpening[to];
		scoreEndingPST += pawnPSTEnding[to];
	}
	else if (captured == BlackPawn)
	{
		blackPawns ^= toBB;
		blackPieces ^= toBB;
		bp += 1;
		materialOpening -= pawnOpening;
		materialEnding -= pawnEnding;
		scoreOpeningPST -= pawnPSTOpening[flip[to]];
		scoreEndingPST -= pawnPSTEnding[flip[to]];
	}
	else if (captured == WhiteKnight)
	{
		whiteKnights ^= toBB;
		whitePieces ^= toBB;
		wn += 1;
		materialOpening += knightOpening;
		materialEnding += knightEnding;
		scoreOpeningPST += knightPSTOpening[to];
		scoreEndingPST += knightPSTEnding[to];
	}
	else if (captured == BlackKnight)
	{
		blackKnights ^= toBB;
		blackPieces ^= toBB;
		bn += 1;
		materialOpening -= knightOpening;
		materialEnding -= knightEnding;
		scoreOpeningPST -= knightPSTOpening[flip[to]];
		scoreEndingPST -= knightPSTEnding[flip[to]];
	}
	else if (captured == WhiteBishop)
	{
		whiteBishops ^= toBB;
		whitePieces ^= toBB;
		wb += 1;
		materialOpening += bishopOpening;
		materialEnding += bishopEnding;
		scoreOpeningPST += bishopPSTOpening[to];
		scoreEndingPST += bishopPSTEnding[to];
	}
	else if (captured == BlackBishop)
	{
		blackBishops ^= toBB;
		blackPieces ^= toBB;
		bb += 1;
		materialOpening -= bishopOpening;
		materialEnding -= bishopEnding;
		scoreOpeningPST -= bishopPSTOpening[flip[to]];
		scoreEndingPST -= bishopPSTEnding[flip[to]];
	}
	else if (captured == WhiteRook)
	{
		whiteRooks ^= toBB;
		whitePieces ^= toBB;
		wr += 1;
		materialOpening += rookOpening;
		materialEnding += rookEnding;
		scoreOpeningPST += rookPSTOpening[to];
		scoreEndingPST += rookPSTEnding[to];
	}
	else if (captured == BlackRook)
	{
		blackRooks ^= toBB;
		blackPieces ^= toBB;	
		br += 1;
		materialOpening -= rookOpening;
		materialEnding -= rookEnding;
		scoreOpeningPST -= rookPSTOpening[flip[to]];
		scoreEndingPST -= rookPSTEnding[flip[to]];
	}
	else if (captured == WhiteQueen)
	{
		whiteQueens ^= toBB;
		whitePieces ^= toBB;
		wq += 1;
		materialOpening += queenOpening;
		materialEnding += queenEnding;
		scoreOpeningPST += queenPSTOpening[to];
		scoreEndingPST += queenPSTEnding[to];
	}
	else if (captured == BlackQueen)
	{
		blackQueens ^= toBB;
		blackPieces ^= toBB;
		bq += 1;
		materialOpening -= queenOpening;
		materialEnding -= queenEnding;
		scoreOpeningPST -= queenPSTOpening[flip[to]];
		scoreEndingPST -= queenPSTEnding[flip[to]];
	}

	// update the phase of the game
	phase = totalPhase;

//	phase -= wp * pawnPhase;
//	phase -= bp * pawnPhase;
	phase -= wn * knightPhase;
	phase -= bn * knightPhase;
	phase -= wb * bishopPhase;
	phase -= bb * bishopPhase;
	phase -= wr * rookPhase;
	phase -= br * rookPhase;
	phase -= wq * queenPhase;
	phase -= bq * queenPhase;

	phase = (phase * 256 + (totalPhase / 2)) / totalPhase;
}

void makeWhitePromotion(int prom, int &to)
{
	U64 toBB = setMask[to];
	whitePawns ^= toBB;
	wp -= 1;
	materialOpening -= pawnOpening;
	materialEnding -= pawnEnding;
	scoreOpeningPST -= pawnPSTOpening[to];
	scoreEndingPST -= pawnPSTEnding[to];

	Hash ^= (pieceHash[WhitePawn][to] ^ pieceHash[prom][to]);
	pHash ^= pieceHash[WhitePawn][to];
	
	if (prom == WhiteQueen)
	{
		whiteQueens ^= toBB;
		wq += 1;
		materialOpening += queenOpening;
		materialEnding += queenEnding;
		scoreOpeningPST += queenPSTOpening[to];
		scoreEndingPST += queenPSTEnding[to];
	}
	else if (prom == WhiteRook)
	{
		whiteRooks ^= toBB;
		wr += 1;
		materialOpening += rookOpening;
		materialEnding += rookEnding;
		scoreOpeningPST += rookPSTOpening[to];
		scoreEndingPST += rookPSTEnding[to];
	}
	else if (prom == WhiteBishop)
	{
		whiteBishops ^= toBB;
		wb += 1;
		materialOpening += bishopOpening;
		materialEnding += bishopEnding;
		scoreOpeningPST += bishopPSTOpening[to];
		scoreEndingPST += bishopPSTEnding[to];
	}
	else if (prom == WhiteKnight)
	{
		whiteKnights ^= toBB;
		wn += 1;
		materialOpening += knightOpening;
		materialEnding += knightEnding;
		scoreOpeningPST += knightPSTOpening[to];
		scoreEndingPST += knightPSTEnding[to];
	}
	
	// update the phase of the game
	phase = totalPhase;

//	phase -= wp * pawnPhase;
//	phase -= bp * pawnPhase;
	phase -= wn * knightPhase;
	phase -= bn * knightPhase;
	phase -= wb * bishopPhase;
	phase -= bb * bishopPhase;
	phase -= wr * rookPhase;
	phase -= br * rookPhase;
	phase -= wq * queenPhase;
	phase -= bq * queenPhase;

	phase = (phase * 256 + (totalPhase / 2)) / totalPhase;
}	

void unmakeWhitePromotion(int prom, int &to)
{
	U64 toBB = setMask[to];
	whitePawns ^= toBB;
	wp += 1;
	materialOpening += pawnOpening;
	materialEnding += pawnEnding;
	scoreOpeningPST += pawnPSTOpening[to];
	scoreEndingPST += pawnPSTEnding[to];
	
	if (prom == WhiteQueen)
	{
		whiteQueens ^= toBB;
		wq -= 1;
		materialOpening -= queenOpening;
		materialEnding -= queenEnding;
		scoreOpeningPST -= queenPSTOpening[to];
		scoreEndingPST -= queenPSTEnding[to];
	}
	else if (prom == WhiteRook)
	{
		whiteRooks ^= toBB;
		wr -= 1;
		materialOpening -= rookOpening;
		materialEnding -= rookEnding;
		scoreOpeningPST -= rookPSTOpening[to];
		scoreEndingPST -= rookPSTEnding[to];
	}
	else if (prom == WhiteBishop)
	{
		whiteBishops ^= toBB;
		wb -= 1;
		materialOpening -= bishopOpening;
		materialEnding -= bishopEnding;
		scoreOpeningPST -= bishopPSTOpening[to];
		scoreEndingPST -= bishopPSTEnding[to];
	}
	else if (prom == WhiteKnight)
	{
		whiteKnights ^= toBB;
		wn -= 1;
		materialOpening -= knightOpening;
		materialEnding -= knightEnding;
		scoreOpeningPST -= knightPSTOpening[to];
		scoreEndingPST -= knightPSTEnding[to];
	}

	// update the phase of the game
	phase = totalPhase;

//	phase -= wp * pawnPhase;
//	phase -= bp * pawnPhase;
	phase -= wn * knightPhase;
	phase -= bn * knightPhase;
	phase -= wb * bishopPhase;
	phase -= bb * bishopPhase;
	phase -= wr * rookPhase;
	phase -= br * rookPhase;
	phase -= wq * queenPhase;
	phase -= bq * queenPhase;

	phase = (phase * 256 + (totalPhase / 2)) / totalPhase;
}	

void makeBlackPromotion(int prom, int &to)
{
	U64 toBB = setMask[to];
	blackPawns ^= toBB;
	bp -= 1;
	materialOpening += pawnOpening;
	materialEnding += pawnEnding;
	scoreOpeningPST += pawnPSTOpening[flip[to]];
	scoreEndingPST += pawnPSTEnding[flip[to]];

	Hash ^= (pieceHash[BlackPawn][to] ^ pieceHash[prom][to]);
	pHash ^= pieceHash[BlackPawn][to];
	
	if (prom == BlackQueen)
	{
		blackQueens ^= toBB;
		bq += 1;
		materialOpening -= queenOpening;
		materialEnding -= queenEnding;
		scoreOpeningPST -= queenPSTOpening[flip[to]];
		scoreEndingPST -= queenPSTEnding[flip[to]];
	}
	else if (prom == BlackRook)
	{
		blackRooks ^= toBB;
		br += 1;
		materialOpening -= rookOpening;
		materialEnding -= rookEnding;
		scoreOpeningPST -= rookPSTOpening[flip[to]];
		scoreEndingPST -= rookPSTEnding[flip[to]];
	}
	else if (prom == BlackBishop)
	{
		blackBishops ^= toBB;
		bb += 1;
		materialOpening -= bishopOpening;
		materialEnding -= bishopEnding;
		scoreOpeningPST -= bishopPSTOpening[flip[to]];
		scoreEndingPST -= bishopPSTEnding[flip[to]];
	}
	else if (prom == BlackKnight)
	{
		blackKnights ^= toBB;
		bn += 1;
		materialOpening -= knightOpening;
		materialEnding -= knightEnding;
		scoreOpeningPST -= knightPSTOpening[flip[to]];
		scoreEndingPST -= knightPSTEnding[flip[to]];
	}

	// update the phase of the game
	phase = totalPhase;

//	phase -= wp * pawnPhase;
//	phase -= bp * pawnPhase;
	phase -= wn * knightPhase;
	phase -= bn * knightPhase;
	phase -= wb * bishopPhase;
	phase -= bb * bishopPhase;
	phase -= wr * rookPhase;
	phase -= br * rookPhase;
	phase -= wq * queenPhase;
	phase -= bq * queenPhase;

	phase = (phase * 256 + (totalPhase / 2)) / totalPhase;
}	

void unmakeBlackPromotion(int prom, int &to)
{
	U64 toBB = setMask[to];
	blackPawns ^= toBB;
	bp += 1;
	materialOpening -= pawnOpening;
	materialEnding -= pawnEnding;
	scoreOpeningPST -= pawnPSTOpening[flip[to]];
	scoreEndingPST -= pawnPSTEnding[flip[to]];
	
	if (prom == BlackQueen)
	{
		blackQueens ^= toBB;
		bq -= 1;
		materialOpening += queenOpening;
		materialEnding += queenEnding;
		scoreOpeningPST += queenPSTOpening[flip[to]];
		scoreEndingPST += queenPSTEnding[flip[to]];
	}
	else if (prom == BlackRook)
	{
		blackRooks ^= toBB;
		br -= 1;
		materialOpening += rookOpening;
		materialEnding += rookEnding;
		scoreOpeningPST += rookPSTOpening[flip[to]];
		scoreEndingPST += rookPSTEnding[flip[to]];
	}
	else if (prom == BlackBishop)
	{
		blackBishops ^= toBB;
		bb -= 1;
		materialOpening += bishopOpening;
		materialEnding += bishopEnding;
		scoreOpeningPST += bishopPSTOpening[flip[to]];
		scoreEndingPST += bishopPSTEnding[flip[to]];
	}
	else if (prom == BlackKnight)
	{
		blackKnights ^= toBB;
		bn -= 1;
		materialOpening += knightOpening;
		materialEnding += knightEnding;
		scoreOpeningPST += knightPSTOpening[flip[to]];
		scoreEndingPST += knightPSTEnding[flip[to]];
	}

	// update the phase of the game
	phase = totalPhase;

//	phase -= wp * pawnPhase;
//	phase -= bp * pawnPhase;
	phase -= wn * knightPhase;
	phase -= bn * knightPhase;
	phase -= wb * bishopPhase;
	phase -= bb * bishopPhase;
	phase -= wr * rookPhase;
	phase -= br * rookPhase;
	phase -= wq * queenPhase;
	phase -= bq * queenPhase;

	phase = (phase * 256 + (totalPhase / 2)) / totalPhase;
}	

	
#endif