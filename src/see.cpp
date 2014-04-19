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
			targetbitmap &= ((bitboards[WhiteQueen] | bitboards[BlackQueen] | bitboards[WhiteRook] | bitboards[BlackRook]) & nonremoved);
			return (attackers | targetbitmap);
		}
	}
	return attackers;
}

int Position::SEE(Move m)
{
	// Approximate piece values, SEE doesn't need to be as accurate as the main evaluation function.
	static const array<int, 13> pieceValues = {
		100, 300, 300, 500, 900, mateScore, 100, 300, 300, 500, 900, mateScore, 0
	};
	uint64_t attackers, nonremoved = (uint64_t)~0;
	int alpha = -infinity, beta, next, value, attackerValue;
	int from = m.getFrom();
	int to = m.getTo();

	if (m.getPromotion() == Pawn)
	{
		value = pieceValues[Pawn];
		attackerValue = value;
	}
	else 
	{
		value = pieceValues[board[to]];
		attackerValue = pieceValues[board[from]];
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
	if (getPieceType(from) == King)
	{
		return alpha;
	}

	// Replace this while(true) with something more correct.
	while (true)
	{
		value -= attackerValue;
		if (value >= beta)
		{
			return beta;
		}
		if (value > alpha)
		{
			alpha = value;
		}
		if (alpha > 0)
		{
			return alpha;
		}

		if (bitboards[Pawn + !sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Pawn + !sideToMove * 6] & attackers);
		}
		else if (bitboards[Knight + !sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Knight + !sideToMove * 6] & attackers);
		}
		else if (bitboards[Bishop + !sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Bishop + !sideToMove * 6] & attackers);
		}
		else if (bitboards[Rook + !sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Rook + !sideToMove * 6] & attackers);
		}
		else if (bitboards[Queen + !sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Queen + !sideToMove * 6] & attackers);
		}
		else
		{
			next = bitScanForward(bitboards[King + !sideToMove * 6]);
		}

		attackers ^= bit[next];
		nonremoved ^= bit[next];

		attackers = revealNextAttacker(attackers, nonremoved, to, next);
		// No defenders left, we are done.
		if (!(attackers & bitboards[12 + sideToMove]))
		{
			break;
		}
		// Illegal king capture, we are done.
		if (getPieceType(next) == King)
		{
			return beta;
		}

		attackerValue = pieceValues[board[next]];

		value += attackerValue;
		if (value <= alpha)
		{
			return alpha;
		}
		if (value < beta)
		{
			beta = value;
		}
		if (beta < 0)
		{
			return beta;
		}

		if (bitboards[Pawn + sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Pawn + sideToMove * 6] & attackers);
		}
		else if (bitboards[Knight + sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Knight + sideToMove * 6] & attackers);
		}
		else if (bitboards[Bishop + sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Bishop + sideToMove * 6] & attackers);
		}
		else if (bitboards[Rook + sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Rook + sideToMove * 6] & attackers);
		}
		else if (bitboards[Queen + sideToMove * 6] & attackers)
		{
			next = bitScanForward(bitboards[Queen + sideToMove * 6] & attackers);
		}
		else
		{
			next = bitScanForward(bitboards[King + !sideToMove * 6]);
		}
		
		attackers ^= bit[next];
		nonremoved ^= bit[next];

		attackers = revealNextAttacker(attackers, nonremoved, to, next);
		// No defenders left, we are done.
		if (!(attackers & bitboards[12 + !sideToMove]))
		{
			break;
		}
		// Illegal king capture, we are done.
		if (getPieceType(next) == King)
		{
			return alpha;
		}

		attackerValue = pieceValues[board[next]];
	}

	if (value < alpha)
	{
		value = alpha;
	}
	if (value > beta)
	{
		value = beta;
	}

	return value;
}

#endif