#include "movegen.hpp"
#include "bitboard.hpp"
#include "magic.hpp"

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
		if (pos.getEnPassantSquare() != NoSquare)
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

	// TODO: make this part cleaner and faster.
	m.setPromotion(King);
	if (side == White)
	{
		if (pos.isAttacked(E1, Black))
		{
			return generatedMoves;
		}
		m.setFrom(E1);
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
		tempMove |= pawnSingleMoves[side][from] & ~occupiedSquares & 0xFF000000000000FF;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			if (to >= A8 || to <= H1)
			{
				m.setPromotion(Queen); mlist[generatedMoves++] = m;
				m.setPromotion(Empty);
			}
			else
			{
				mlist[generatedMoves++] = m;
			}
		}

		// Beware with Null move, you might be en passanting your own pawns.
		if (pos.getEnPassantSquare() != NoSquare)
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

// This works if and only if the side to move is in check.
int generateEvasions(Position & pos, Move * mlist)
{
	int from, to, generatedMoves = 0;
	bool side = pos.getSideToMove();
	uint64_t tempPiece, tempMove, pinned = 0;
	uint64_t occupied = pos.getOccupiedSquares();
	uint64_t targetBitboard = ~pos.getPieces(side);

	Move m;
	m.clear();
	m.setPromotion(Empty);
	
	from = bitScanForward(pos.getBitboard(side, King));
	m.setFrom(from);
	tempMove = kingAttacks[from] & targetBitboard;
	// Now we take the king out from the board and see which squares are attacked.
	pos.replaceBitboard(occupied ^ bit[from], 14);
	while (tempMove)
	{
		to = bitScanForward(tempMove);
		tempMove &= (tempMove - 1);
		if (pos.isAttacked(to, !side))
		{
			continue;
		}
		m.setTo(to);
		mlist[generatedMoves++] = m;
	}
	// And here we put the king back in. 
	pos.replaceBitboard(occupied, 14);

	// If we are checked by more than two pieces only the king can move. Therefore we can return.
	uint64_t checkers = pos.attacksTo(from, !side);
	if (popcnt(checkers) > 1)
	{
		return generatedMoves;
	}

	// find the pinned pieces
	uint64_t b = bishopAttacks(from, 0) & (pos.getBitboard(!side, Bishop) | pos.getBitboard(!side, Queen));
	while (b)
	{
		int pinner = bitScanForward(b);
		b &= (b - 1);
		uint64_t potentialPinned = (between[from][pinner] & occupied);
		if (popcnt(potentialPinned) == 1)
		{
			pinned |= potentialPinned & pos.getPieces(side);
		}
	}
	b = rookAttacks(from, 0) & (pos.getBitboard(!side, Rook) | pos.getBitboard(!side, Queen));
	while (b)
	{
		int pinner = bitScanForward(b);
		b &= (b - 1);
		uint64_t potentialPinned = (between[from][pinner] & occupied);
		if (popcnt(potentialPinned) == 1)
		{
			pinned |= potentialPinned & pos.getPieces(side);
		}
	}

	tempPiece = pos.getBitboard(side, Pawn);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		if (bit[from] & pinned)
		{
			continue;
		}
		m.setFrom(from);
		tempMove = pawnAttacks[side][from] & checkers;
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
		int enPassant = pos.getEnPassantSquare();
		if (enPassant != NoSquare)
		{
			if (pawnAttacks[side][from] & bit[enPassant])
			{
				if (bitScanForward(checkers) == (enPassant - 8 + side * 16))
				{
					m.setPromotion(Pawn);
					m.setTo(enPassant);
					mlist[generatedMoves++] = m;
					m.setPromotion(Empty);
				}
			}
		}

		uint64_t interpose = between[bitScanForward(pos.getBitboard(side, King))][bitScanForward(checkers)];
		if (interpose)
		{
			if (pawnSingleMoves[side][from] & occupied)
			{
				continue;
			}
			tempMove = pawnSingleMoves[side][from] & interpose;
			if (!(pawnDoubleMoves[side][from] & occupied))
			{
				tempMove |= pawnDoubleMoves[side][from] & interpose;
			}
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
		}
	}

	uint64_t interpose = between[bitScanForward(pos.getBitboard(side, King))][bitScanForward(checkers)] | bit[bitScanForward(checkers)];
	tempPiece = pos.getBitboard(side, Knight);
	while (tempPiece)
	{
		from = bitScanForward(tempPiece);
		tempPiece &= (tempPiece - 1);
		if (bit[from] & pinned)
		{
			continue;
		}
		m.setFrom(from);
		tempMove = knightAttacks[from] & interpose;
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
		if (bit[from] & pinned)
		{
			continue;
		}
		m.setFrom(from);
		tempMove = bishopAttacks(from, occupied) & interpose;
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
		if (bit[from] & pinned)
		{
			continue;
		}
		m.setFrom(from);
		tempMove = rookAttacks(from, occupied) & interpose;
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
		if (bit[from] & pinned)
		{
			continue;
		}
		m.setFrom(from);
		tempMove = queenAttacks(from, occupied) & interpose;
		while (tempMove)
		{
			to = bitScanForward(tempMove);
			tempMove &= (tempMove - 1);
			m.setTo(to);
			mlist[generatedMoves++] = m;	
		}
	}

	return generatedMoves;
}

int generateQuietChecks(Position & pos, Move * mlist)
{
    int from, to;
    auto side = pos.getSideToMove();
    auto occupied = pos.getOccupiedSquares();
    auto freeSquares = pos.getFreeSquares();
    auto generatedMoves = 0;
    uint64_t pinned = 0, tempMove;
    Move m;
    m.clear();
    m.setPromotion(Empty);

    // Calculate the squares which give check.
    auto opponentKingSquare = bitScanForward(pos.getBitboard(!side, King));
    auto bishopCheckSquares = freeSquares & bishopAttacks(opponentKingSquare, occupied);
    auto rookCheckSquares = freeSquares & rookAttacks(opponentKingSquare, occupied);
    auto queenCheckSquares = bishopCheckSquares | rookCheckSquares;

    auto tempPiece = pos.getBitboard(side, Pawn);
    while (tempPiece)
    {
        from = bitScanForward(tempPiece);
        tempPiece &= (tempPiece - 1);
        m.setFrom(from);
        auto singleMove = freeSquares & pawnSingleMoves[side][from];
        if (singleMove)
        {
            if (singleMove & pawnAttacks[!side][opponentKingSquare])
            {
                to = from + 8 - side * 16;
                m.setTo(to);
                mlist[generatedMoves++] = m;
            }
            else
            {
                if (freeSquares & pawnDoubleMoves[side][from] & pawnAttacks[!side][opponentKingSquare])
                {
                    to = from + 16 - side * 32;
                    m.setTo(to);
                    mlist[generatedMoves++] = m;
                }
            }
        }
    }

    tempPiece = pos.getBitboard(side, Knight);
    while (tempPiece)
    {
        from = bitScanForward(tempPiece);
        tempPiece &= (tempPiece - 1);
        m.setFrom(from);

        tempMove = knightAttacks[from] & knightAttacks[opponentKingSquare] & freeSquares;

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

        tempMove = bishopAttacks(from, occupied) & bishopCheckSquares;

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

        tempMove = rookAttacks(from, occupied) & rookCheckSquares;

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

        tempMove = queenAttacks(from, occupied) & queenCheckSquares;
        while (tempMove)
        {
            to = bitScanForward(tempMove);
            tempMove &= (tempMove - 1);
            m.setTo(to);
            mlist[generatedMoves++] = m;
        }
    }

    // Kings cannot give check unless it is a discovered check.
    // Discovered checks go here, someday.

    return generatedMoves;
}
