#ifndef SEE_CPP
#define SEE_CPP

#include "see.h"
#include "defs.h"
#include "move.h"
#include "eval.h"
#include "board.h"
#include "bitboard.h"
#include "magic.h"

int SEE2(Move &move);

U64 attacksTo(int &target)
{
	U64 attackers, blockers, attackbitmap;
	int databaseIndex;

	// rook-type attacks
	blockers = occupiedSquares & occupancyMaskRook[target];
	databaseIndex = (int)((blockers * magicNumberRook[target]) >> rookMagicShifts[target]);
	attackbitmap = magicMovesRook[target][databaseIndex];
	attackers = (attackbitmap & (blackQueens | whiteQueens | blackRooks | whiteRooks));

	// bishop-type attacks
	blockers = occupiedSquares & occupancyMaskBishop[target];
	databaseIndex = (int)((blockers * magicNumberBishop[target]) >> bishopMagicShifts[target]);
	attackbitmap = magicMovesBishop[target][databaseIndex];
	attackers |= (attackbitmap & (blackQueens | whiteQueens | blackBishops | whiteBishops));

	// knight attacks
	attackbitmap = knightAttacks[target];
	attackers |= (attackbitmap & (blackKnights | whiteKnights));

	// white pawn attacks (no en passant)
	attackbitmap = blackPawnAttacks[target];
	attackers |= (attackbitmap & (whitePawns));

	// black pawn attacks (no en passant)
	attackbitmap = whitePawnAttacks[target];
	attackers |= (attackbitmap & (blackPawns));

	// king attacks
	attackbitmap = kingAttacks[target];
	attackers |= (attackbitmap & (whiteKing | blackKing));

	return attackers;

}

U64 revealNextAttacker(U64 &attackers, U64 &nonremoved, int &target, int &heading)
{
	// we update the attackers bitmap to include any attackers which have been revealed by moving the previous attacker
	U64 blockers, targetbitmap;
	U64 occupied = occupiedSquares & nonremoved;
	int databaseIndex;

	if (heading == -9)
	{
		targetbitmap = RaySW[target] & ((whiteBishops | whiteQueens | blackBishops | blackQueens) & nonremoved);
		if (targetbitmap)
		{
			blockers = occupied & occupancyMaskBishop[target];
			databaseIndex = (int)((blockers * magicNumberBishop[target]) >> bishopMagicShifts[target]);
			targetbitmap = magicMovesBishop[target][databaseIndex];
			targetbitmap &= ((blackQueens | whiteQueens | blackBishops | whiteBishops) & nonremoved);
			return (attackers | targetbitmap);
		}
		else return attackers;
	}
	else if (heading == -8)
	{
		targetbitmap = RayS[target] & ((whiteRooks | whiteQueens | blackRooks | blackQueens) & nonremoved);
		if (targetbitmap)
		{
			blockers = occupied & occupancyMaskRook[target];
			databaseIndex = (int)((blockers * magicNumberRook[target]) >> rookMagicShifts[target]);
			targetbitmap = magicMovesRook[target][databaseIndex];
			targetbitmap &= ((blackQueens | whiteQueens | blackRooks | whiteRooks) & nonremoved);
			return (attackers | targetbitmap);
		}
		else return attackers;
	}
	else if (heading == -7)
	{
		targetbitmap = RaySE[target] & ((whiteBishops | whiteQueens | blackBishops | blackQueens) & nonremoved);
		if (targetbitmap)
		{
			blockers = occupied & occupancyMaskBishop[target];
			databaseIndex = (int)((blockers * magicNumberBishop[target]) >> bishopMagicShifts[target]);
			targetbitmap = magicMovesBishop[target][databaseIndex];
			targetbitmap &= ((blackQueens | whiteQueens | blackBishops | whiteBishops) & nonremoved);
			return (attackers | targetbitmap);
		}
		else return attackers;
	}
	else if (heading == -1)
	{
		targetbitmap = RayW[target] & ((whiteRooks | whiteQueens | blackRooks | blackQueens) & nonremoved);
		if (targetbitmap)
		{
			blockers = occupied & occupancyMaskRook[target];
			databaseIndex = (int)((blockers * magicNumberRook[target]) >> rookMagicShifts[target]);
			targetbitmap = magicMovesRook[target][databaseIndex];
			targetbitmap &= ((blackQueens | whiteQueens | blackRooks | whiteRooks) & nonremoved);
			return (attackers | targetbitmap);
		}
		else return attackers;
	}
	else if (heading == 1)
	{
		targetbitmap = RayE[target] & ((whiteRooks | whiteQueens | blackRooks | blackQueens) & nonremoved);
		if (targetbitmap)
		{
			blockers = occupied & occupancyMaskRook[target];
			databaseIndex = (int)((blockers * magicNumberRook[target]) >> rookMagicShifts[target]);
			targetbitmap = magicMovesRook[target][databaseIndex];
			targetbitmap &= ((blackQueens | whiteQueens | blackRooks | whiteRooks) & nonremoved);
			return (attackers | targetbitmap);
		}
		else return attackers;
	}
	else if (heading == 7)
	{
		targetbitmap = RayNW[target] & ((whiteBishops | whiteQueens | blackBishops | blackQueens) & nonremoved);
		if (targetbitmap)
		{		
			blockers = occupied & occupancyMaskBishop[target];
			databaseIndex = (int)((blockers * magicNumberBishop[target]) >> bishopMagicShifts[target]);
			targetbitmap = magicMovesBishop[target][databaseIndex];
			targetbitmap &= ((blackQueens | whiteQueens | blackBishops | whiteBishops) & nonremoved);
			return (attackers | targetbitmap);
		}
		else return attackers;
	}
	else if (heading == 8)
	{
		targetbitmap = RayN[target] & ((whiteRooks | whiteQueens | blackRooks | blackQueens) & nonremoved);
		if (targetbitmap)
		{
			blockers = occupied & occupancyMaskRook[target];
			databaseIndex = (int)((blockers * magicNumberRook[target]) >> rookMagicShifts[target]);
			targetbitmap = magicMovesRook[target][databaseIndex];
			targetbitmap &= ((blackQueens | whiteQueens | blackRooks | whiteRooks) & nonremoved);
			return (attackers | targetbitmap);
		}
		else return attackers;
	}
	else if (heading == 9)
	{
		targetbitmap = RayNE[target] & ((whiteBishops | whiteQueens | blackBishops | blackQueens) & nonremoved);
		if (targetbitmap)
		{
			blockers = occupied & occupancyMaskBishop[target];
			databaseIndex = (int)((blockers * magicNumberBishop[target]) >> bishopMagicShifts[target]);
			targetbitmap = magicMovesBishop[target][databaseIndex];
			targetbitmap &= ((blackQueens | whiteQueens | blackBishops | whiteBishops) & nonremoved);
			return (attackers | targetbitmap);

		}
		else return attackers;
	}
	return attackers;
}

int SEE(Move &move)
{
	U64 attackers, nonremoved = (U64)~0;
	int alpha, beta, next, value = 0, attackerValue = 0;
	int from = move.getFrom();
	int to = move.getTo();
	bool ispromorank = ((Rank(to) == 7) || (Rank(to) == 0));
	
	if (move.isEnpassant())
		value = pieceValues[1];
	else
		value = pieceValues[getPiece(to)];
	if (ispromorank && move.isPawnmove())
	{
		value += pieceValues[move.getPromotion()] - pieceValues[WhitePawn];
		attackerValue += pieceValues[move.getPromotion()] - pieceValues[WhitePawn];
	}
	alpha = -(mateScore - 100); // not -(mateScore + 1) as that causes a bug, likely with move sorting/selectmove
	beta = value;

	// generate the list of attackers
	attackers = attacksTo(to);
	// remove the piece which is moving
	attackers &= ~setMask[from];
	nonremoved &= ~setMask[from];
 
	// another attacker might be revealed, update attackers accordingly:
	int heading = Headings[to][from];
	if (heading)
	{
		attackers = revealNextAttacker(attackers, nonremoved, to, heading);
	}

	// if there are no defenders just return beta
	if (sideToMove)
	{
		if (!(attackers & whitePieces))
		{
			return beta;
		}
	}
	else 
	{
		if (!(attackers & blackPieces))
		{
			return beta;
		}
	}
	// illegal king capture, return alpha
	if (move.isKingmove())
		return alpha;

	// mark the value of the attacking piece
	attackerValue += pieceValues[getPiece(from)];
		
	while (true)
	{
		if (!sideToMove)
		{
			value -= attackerValue;
			attackerValue = 0;
			if (value >= beta)
				return beta;
			if (value > alpha)
				alpha = value;
			if (alpha > 0)
				return alpha;
			
			// select the next attacker
			if (Rank(to) != 0 && blackPawns & attackers)   
				next = bitScanForward(blackPawns & attackers);
			else if (blackKnights & attackers) 
				next = bitScanForward(blackKnights & attackers);
			else if (blackBishops & attackers) 
				next = bitScanForward(blackBishops & attackers);
			else if (blackRooks & attackers)  
				next = bitScanForward(blackRooks & attackers);
			else if (Rank(to) == 0 && blackPawns & attackers)
			{
				next = bitScanForward(blackPawns & attackers);
				value -= pieceValues[7] - pieceValues[1];
				attackerValue += pieceValues[7] - pieceValues[1];
			}
			else if (blackQueens & attackers)  
				next = bitScanForward(blackQueens & attackers);
			else if (blackKing & attackers)
				next = bitScanForward(blackKing);

			attackers &= ~setMask[next];
			nonremoved &= ~setMask[next];

			// another attacker might be revealed, update attackers accordingly:
			int heading = Headings[to][next];
			if (heading)
			{
				attackers = revealNextAttacker(attackers, nonremoved, to, heading);
			}
			// no defenders left
			if (!(attackers & whitePieces))
				break;
			// illegal king capture
			if (setMask[next] & blackKing)
				return beta;

			attackerValue += pieceValues[getPiece(next)];

			// iteration for White
			value += attackerValue;
			attackerValue = 0;
			if (value <= alpha)
				return alpha;
			if (value < beta)
				beta = value;
			if (beta < 0)
				return beta;

			// select the next attacker
			if ((Rank(to) != 7) && whitePawns & attackers)   
				next = bitScanForward(whitePawns & attackers);
			else if (whiteKnights & attackers) 
				next = bitScanForward(whiteKnights & attackers);
			else if (whiteBishops & attackers) 
				next = bitScanForward(whiteBishops & attackers);
			else if (whiteRooks & attackers)  
				next = bitScanForward(whiteRooks & attackers);
			else if ((Rank(to) == 7) && whitePawns & attackers) 
			{
				next = bitScanForward(whitePawns & attackers);
				value += pieceValues[7] - pieceValues[1];
				attackerValue += pieceValues[7] - pieceValues[1];
			}
			else if (whiteQueens & attackers)  
				next = bitScanForward(whiteQueens & attackers);
			else if (whiteKing & attackers)
				next = bitScanForward(whiteKing);

			attackers &= ~setMask[next];
			nonremoved &= ~setMask[next];

			// another attacker might be revealed, update attackers accordingly:
			heading = Headings[to][next];
			if (heading)
			{
				attackers = revealNextAttacker(attackers, nonremoved, to, heading);
			}
			// no defenders left
			if (!(attackers & blackPieces))
				break;
			// illegal king capture
			if (setMask[next] & whiteKing)
				return alpha;

			attackerValue += pieceValues[getPiece(next)];
        }
		else
		{
			// iteration for White
			value -= attackerValue;
			attackerValue = 0;
			if (value >= beta)
				return beta;
			if (value > alpha)
				alpha = value;
			if (alpha > 0)
				return alpha;

			// select the next attacker
			if ((Rank(to) != 7) && whitePawns & attackers)   
				next = bitScanForward(whitePawns & attackers);
			else if (whiteKnights & attackers) 
				next = bitScanForward(whiteKnights & attackers);
			else if (whiteBishops & attackers) 
				next = bitScanForward(whiteBishops & attackers);
			else if (whiteRooks & attackers)  
				next = bitScanForward(whiteRooks & attackers);
			else if ((Rank(to) == 7) && whitePawns & attackers) 
			{
				next = bitScanForward(whitePawns & attackers);
				value -= pieceValues[7] - pieceValues[1];
				attackerValue += pieceValues[7] - pieceValues[1];
			}
			else if (whiteQueens & attackers)  
				next = bitScanForward(whiteQueens & attackers);
			else if (whiteKing & attackers)
				next = bitScanForward(whiteKing);

			attackers &= ~setMask[next];
			nonremoved &= ~setMask[next];

			// another attacker might be revealed, update attackers accordingly:
			int heading = Headings[to][next];
			if (heading)
			{
				attackers = revealNextAttacker(attackers, nonremoved, to, heading);
			}
			// no defenders left
			if (!(attackers & blackPieces))
				break;
			// illegal king capture
			if (setMask[next] & whiteKing)
				return beta;

			attackerValue += pieceValues[getPiece(next)];

			// iteration for Black
			value += attackerValue;
			attackerValue = 0;
			if (value <= alpha)
				return alpha;
			if (value < beta)
				beta = value;
			if (beta < 0)
				return beta;
			
			// select the next attacker
			if (Rank(to) != 0 && blackPawns & attackers)   
				next = bitScanForward(blackPawns & attackers);
			else if (blackKnights & attackers) 
				next = bitScanForward(blackKnights & attackers);
			else if (blackBishops & attackers) 
				next = bitScanForward(blackBishops & attackers);
			else if (blackRooks & attackers)  
				next = bitScanForward(blackRooks & attackers);
			else if (Rank(to) == 0 && blackPawns & attackers)
			{
				next = bitScanForward(blackPawns & attackers);
				value += pieceValues[7] - pieceValues[1];
				attackerValue += pieceValues[7] - pieceValues[1];
			}
			else if (blackQueens & attackers)  
				next = bitScanForward(blackQueens & attackers);
			else if (blackKing & attackers)
				next = bitScanForward(blackKing);

			attackers &= ~setMask[next];
			nonremoved &= ~setMask[next];

			// another attacker might be revealed, update attackers accordingly:
			heading = Headings[to][next];
			if (heading)
			{
				attackers = revealNextAttacker(attackers, nonremoved, to, heading);
			}

			// no defenders left
			if (!(attackers & whitePieces))
				break;
			// illegal king capture
			if (setMask[next] & blackKing)
				return alpha;

			attackerValue += pieceValues[getPiece(next)];
		}
	}
 
	if (value < alpha) 
		value = alpha;
	if (value > beta)
		value = beta;
	
	return value;
}

#endif