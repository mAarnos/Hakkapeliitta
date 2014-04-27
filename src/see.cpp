#include "see.hpp"
#include "bitboard.hpp"
#include "magic.hpp"
#include "position.hpp"
#include "eval.hpp"

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
	uint64_t attackers, nonRemoved = (uint64_t)~0;
	array<int, 32> materialGains;
	int next, attackerValue, numberOfCaptures = 0;
	int from = m.getFrom();
	int to = m.getTo();
	int promotion = m.getPromotion();
	bool toAtPromoRank = false, side = sideToMove;

	if (promotion == King)
	{
		return 0;
	}
	else if (promotion == Pawn)
	{
		materialGains[0] = pieceValues[Pawn];
		attackerValue = pieceValues[Pawn];
	}
	else
	{
		materialGains[0] = pieceValues[board[to]];
		attackerValue = pieceValues[board[from]];
		if (promotion != Empty)
		{
			materialGains[0] += pieceValues[promotion] - pieceValues[Pawn];
			attackerValue += pieceValues[promotion] - pieceValues[Pawn];
			toAtPromoRank = true;
		}
	}
	numberOfCaptures++;

	// Generate a list of attackers.
	attackers = attacksTo(to);
	// Remove the piece moving.
	// DO NOT OPTIMIZE THIS TO attackers ^= bit[from], that causes a bug when promoting a pawn.
	// Basically, attacksTo cannot detect pawns about to promote so ^= creates an attacker instead of removing one.
	attackers &= ~bit[from];
	nonRemoved &= ~bit[from];

	attackers = revealNextAttacker(attackers, nonRemoved, to, from);
	side = !side;

	while (attackers & bitboards[12 + side])
	{
		if (!toAtPromoRank && bitboards[Pawn + side * 6] & attackers)
		{
			next = bitScanForward(bitboards[Pawn + side * 6] & attackers);
		}
		else if (bitboards[Knight + side * 6] & attackers)
		{
			next = bitScanForward(bitboards[Knight + side * 6] & attackers);
		}
		else if (bitboards[Bishop + side * 6] & attackers)
		{
			next = bitScanForward(bitboards[Bishop + side * 6] & attackers);
		}
		else if (bitboards[Rook + side * 6] & attackers)
		{
			next = bitScanForward(bitboards[Rook + side * 6] & attackers);
		}
		else if (toAtPromoRank && bitboards[Pawn + side * 6] & attackers)
		{
			next = bitScanForward(bitboards[Pawn + side * 6] & attackers);
		}
		else if (bitboards[Queen + side * 6] & attackers)
		{
			next = bitScanForward(bitboards[Queen + side * 6] & attackers);
		}
		else
		{
			next = bitScanForward(bitboards[King + side * 6]);
		}

		// Update the materialgains array.
		materialGains[numberOfCaptures] = -materialGains[numberOfCaptures - 1] + attackerValue;
		// Remember the value of the capturing piece because it is going to be captured next.
		attackerValue = pieceValues[board[next]];
		// If we are going to do a promotion we need to correct the values a bit.
		if (toAtPromoRank && attackerValue == pieceValues[Pawn])
		{
			materialGains[numberOfCaptures] += pieceValues[Queen] - pieceValues[Pawn];
			attackerValue += pieceValues[Queen] - pieceValues[Pawn];
		}

		attackers &= ~bit[next];
		nonRemoved &= ~bit[next];

		attackers = revealNextAttacker(attackers, nonRemoved, to, next);
		side = !side;

		if (getPieceType(next) == King && (attackers & bitboards[12 + side]))
		{
			break;
		}
		numberOfCaptures++;
	}

	while (--numberOfCaptures)
	{
		materialGains[numberOfCaptures - 1] = min(-materialGains[numberOfCaptures], materialGains[numberOfCaptures - 1]);
	}

	return materialGains[0];
}