#ifndef SEE_CPP
#define SEE_CPP

#include "see.h"
#include "bitboard.h"
#include "magic.h"
#include "position.h"
#include "eval.h"

uint64_t Position::attacksTo(int to)
{
	uint64_t attackers, attacks;

	attacks = rookAttacks(to, bitboards[14]);
	attackers = (attacks & (bitboards[WhiteQueen] | bitboards[BlackQueen] | bitboards[WhiteRook] | bitboards[BlackRook]));

	attacks = bishopAttacks(to, bitboards[14]);
	attackers |= (attacks & (bitboards[WhiteQueen] | bitboards[BlackQueen] | bitboards[WhiteBishop] | bitboards[BlackBishop]));

	attacks = knightAttacks[to];
	attackers |= (attacks & (bitboards[WhiteKnight] | bitboards[BlackKnight]));

	attacks = pawnAttacks[Black][to];
	attackers |= (attacks & (bitboards[WhitePawn]));

	attacks = pawnAttacks[White][to];
	attackers |= (attacks & (bitboards[BlackPawn]));

	attacks = kingAttacks[to];
	attackers |= (attacks & (bitboards[WhiteKing] | bitboards[BlackKing]));

	return attackers;
}

uint64_t Position::attacksTo(int to, bool side)
{
	uint64_t attackers, attacks;

	attacks = rookAttacks(to, bitboards[14]);
	attackers = (attacks & (bitboards[WhiteQueen + side * 6] | bitboards[WhiteRook + side * 6]));

	attacks = bishopAttacks(to, bitboards[14]);
	attackers |= (attacks & (bitboards[WhiteQueen + side * 6] | bitboards[WhiteBishop + side * 6]));

	attacks = knightAttacks[to];
	attackers |= (attacks & (bitboards[WhiteKnight + side * 6]));

	attacks = pawnAttacks[!side][to];
	attackers |= (attacks & (bitboards[WhitePawn + side * 6]));

	attacks = kingAttacks[to];
	attackers |= (attacks & (bitboards[WhiteKing + side * 6]));

	return attackers;
}

uint64_t Position::revealNextAttacker(uint64_t attackers, uint64_t nonremoved, int to, int from)
{
	int direction = heading[to][from];
	if (direction == -1)
	{
		return attackers;
	}

	// we update the attackers bitmap to include any attackers which have been revealed by moving the previous attacker
	uint64_t targetbitmap;
	uint64_t occupied = bitboards[14] & nonremoved;

	if (direction == SW || direction == SE || direction == NW || direction == NE)
	{
		targetbitmap = rays[direction][to] & ((bitboards[WhiteQueen] | bitboards[BlackQueen] | bitboards[WhiteBishop] | bitboards[BlackBishop]) & nonremoved);
		if (targetbitmap)
		{
			targetbitmap = bishopAttacks(to, occupied);
			targetbitmap &= ((bitboards[WhiteQueen] | bitboards[BlackQueen] | bitboards[WhiteBishop] | bitboards[BlackBishop]) & nonremoved);
			return (attackers | targetbitmap);
		}
	}
	else
	{
		targetbitmap = rays[direction][to] & ((bitboards[WhiteQueen] | bitboards[BlackQueen] | bitboards[WhiteRook] | bitboards[BlackRook]) & nonremoved);
		if (targetbitmap)
		{
			targetbitmap = rookAttacks(to, occupied);
			targetbitmap &= ((bitboards[WhiteQueen] | bitboards[BlackQueen] | bitboards[WhiteBishop] | bitboards[BlackBishop]) & nonremoved);
			return (attackers | targetbitmap);
		}
	}
	return attackers;
}

int Position::SEE(Move m)
{
	uint64_t attackers, nonremoved = (uint64_t)~0;
	int alpha = -infinity, beta, next, value, attackerValue = 0;
	int from = m.getFrom();
	int to = m.getTo();
	int promotion = m.getPromotion(); 

	if (promotion == King)
	{
		return 0;
	}
	else if (promotion == Pawn)
	{
		value = averagePieceValues[Pawn];
	}
	else 
	{
		value = averagePieceValues[getPiece(to)];
		if (promotion != Empty)
		{
			value += averagePieceValues[promotion] - averagePieceValues[Pawn];
			attackerValue += averagePieceValues[promotion] - averagePieceValues[Pawn];
		}
	}
	beta = value;

	// Generate a list of attackers.
	attackers = attacksTo(to);
	// Remove the piece moving.
	attackers ^= bit[from];
	nonremoved ^= bit[from];

	attackers = revealNextAttacker(attackers, nonremoved, to, from);

	// If there are no defenders just return beta.
	if (!(attackers & bitboards[12 + !sideToMove]))
	{
		return beta;
	}
	// Illegal king capture, return alpha.
	if (getPiece(from) == King)
	{
		return alpha;
	}

	attackerValue += averagePieceValues[getPiece(from)];
}

#endif