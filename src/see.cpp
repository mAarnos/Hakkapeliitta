#ifndef SEE_CPP
#define SEE_CPP

#include "see.h"
#include "bitboard.h"
#include "magic.h"
#include "position.h"

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

#endif