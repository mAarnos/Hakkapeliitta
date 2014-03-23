#ifndef POSITION_CPP
#define POSITION_CPP

#include "position.h"
#include "bitboard.h"
#include "hash.h"
#include "magic.h"

void Position::displayBoard()
{
	string pieceToMark = "PNBRQKpnbrqk.";
	for (int i = 7; i >= 0; i--)
	{
		for (int j = 0; j < 8; j++)
		{
			cout << pieceToMark[board[i * 8 + j]] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

void Position::initializeBoardFromFEN(string FEN)
{
	unsigned int i, j;
	int sq;
	char letter;
	int aRank, aFile;

	for (sq = A1; sq <= H8; sq++)
	{
		board[sq] = Empty;
	}

	// Split the FEN into parts.
	vector<string> strList;
	stringstream ss(FEN);
	string item;
	while (getline(ss, item, ' ')) 
	{
		strList.push_back(item);
	}

	j = 1; i = 0;
	// Translate the FEN string into piece locations on the board.
	while ((j <= 64) && (i <= strList[0].length())) 
	{
		letter = strList[0].at(i);
		i++;
		aFile = 1 + ((j - 1) % 8);
		aRank = 8 - ((j - 1) / 8);
		sq = (int)(((aRank - 1) * 8) + (aFile - 1));
		switch (letter)
		{
			case 'p': board[sq] = BlackPawn; break;
			case 'r': board[sq] = BlackRook; break;
			case 'n': board[sq] = BlackKnight; break;
			case 'b': board[sq] = BlackBishop; break;
			case 'q': board[sq] = BlackQueen; break;
			case 'k': board[sq] = BlackKing; break;
			case 'P': board[sq] = WhitePawn; break;
			case 'R': board[sq] = WhiteRook; break;
			case 'N': board[sq] = WhiteKnight; break;
			case 'B': board[sq] = WhiteBishop; break;
			case 'Q': board[sq] = WhiteQueen; break;
			case 'K': board[sq] = WhiteKing; break;
			case '/': j--; break;
			case '1': break;
			case '2': j++; break;
			case '3': j += 2; break;
			case '4': j += 3; break;
			case '5': j += 4; break;
			case '6': j += 5; break;
			case '7': j += 6; break;
			case '8': j += 7; break;
			default: return;
		}
		j++;
	}

	// set the turn; default = White 
	sideToMove = White;
	if (strList.size() >= 2)
	{
		if (strList[1] == "w")
		{
			sideToMove = White;
		}
		else if (strList[1] == "b")
		{
			sideToMove = Black;
		}
		else return;
	}

	// set castling to default 0 
	castlingRights = 0;
	// Initialize all castle possibilities 
	if (strList.size() >= 3)
	{
		if (strList[2].find('K') != string::npos)
		{
			castlingRights += WhiteOO;
		}
		if (strList[2].find('Q') != string::npos)
		{
			castlingRights += WhiteOOO;
		}
		if (strList[2].find('k') != string::npos)
		{
			castlingRights += BlackOO;
		}
		if (strList[2].find('q') != string::npos)
		{
			castlingRights += BlackOOO;
		}
	}

	// Set the en passant square, if any. Default is 64 which means no en passant
	enPassantSquare = 64;
	if ((strList.size() >= 4) && (strList[3].length() >= 2))
	{
		if ((strList[3].at(0) >= 'a') && (strList[3].at(0) <= 'h') && ((strList[3].at(1) == '3') || (strList[3].at(1) == '6')))
		{
			aFile = strList[3].at(0) - 96; // ASCII 'a' = 97 
			aRank = strList[3].at(1) - 48; // ASCII '1' = 49 
			enPassantSquare = (int)((aRank - 1) * 8 + aFile - 1);
		}
		else return;
	}

	// Fifty move distance, we start at 0 by default.
	fiftyMoveDistance = 0;
	if (strList.size() >= 5)
	{
		fiftyMoveDistance = stoi(strList[4]);
	}

	hply = 0;
	if (strList.size() >= 6)
	{
		hply = 2 * stoi(strList[5]) - 1;
		if (hply < 0)
		{
			hply = 0; // avoid possible underflow 
		}
		if (sideToMove == Black)
		{
			hply++;
		}
	}

	bitboards.fill(0);

	// populate the bitboards
	for (i = A1; i <= H8; i++) 
	{
		if (board[i] == WhitePawn)
		{
			setBit(bitboards[WhitePawn], i);
		}
		else if (board[i] == WhiteKnight)
		{
			setBit(bitboards[WhiteKnight], i);
		}
		else if (board[i] == WhiteBishop)
		{
			setBit(bitboards[WhiteBishop], i);
		}
		else if (board[i] == WhiteRook)
		{
			setBit(bitboards[WhiteRook], i);
		}
		else if (board[i] == WhiteQueen)
		{
			setBit(bitboards[WhiteQueen], i);
		}
		else if (board[i] == WhiteKing)
		{
			setBit(bitboards[WhiteKing], i);
		}
		else if (board[i] == BlackPawn)
		{
			setBit(bitboards[BlackPawn], i);
		}
		else if (board[i] == BlackKnight)
		{
			setBit(bitboards[BlackKnight], i);
		}
		else if (board[i] == BlackBishop)
		{
			setBit(bitboards[BlackBishop], i);
		}
		else if (board[i] == BlackRook)
		{
			setBit(bitboards[BlackRook], i);
		}
		else if (board[i] == BlackQueen)
		{
			setBit(bitboards[BlackQueen], i);
		}
		else if (board[i] == BlackKing)
		{
			setBit(bitboards[BlackKing], i);
		}
	}
	bitboards[12] = bitboards[WhiteKing] | bitboards[WhiteQueen] | bitboards[WhiteRook] | bitboards[WhiteBishop] | bitboards[WhiteKnight] | bitboards[WhitePawn];
	bitboards[13] = bitboards[BlackKing] | bitboards[BlackQueen] | bitboards[BlackRook] | bitboards[BlackBishop] | bitboards[BlackKnight] | bitboards[BlackPawn];
	bitboards[14] = bitboards[12] | bitboards[13];
	bitboards[15] = ~(bitboards[14]);

	hash = 0;
	pawnHash = 0;

	return;
}

bool Position::makeMove(Move m)
{
	uint64_t fromBB = 0, fromToBB = 0;
	int fromRook = -1, toRook = -1;
	int from = m.getFrom();
	int to = m.getTo();
	int promotion = m.getPromotion();
	int piece = board[from];
	int captured = board[to];

	historyStack[hply].castle = castlingRights;
	historyStack[hply].ep = enPassantSquare;
	historyStack[hply].fifty = fiftyMoveDistance;
	historyStack[hply].captured = captured;
	historyStack[hply].hash = hash;
	historyStack[hply].pHash = pawnHash;
	hply++;
	
	setBit(fromBB, from);
	setBit(fromToBB, to);
	fromToBB |= fromBB;

	board[to] = board[from];
	board[from] = Empty;

	hash ^= (pieceHash[piece][from] ^ pieceHash[piece][to]);
	if (enPassantSquare != 64)
	{
		hash ^= enPassantHash[enPassantSquare];
		enPassantSquare = 64;
	}

	bitboards[piece] ^= fromToBB;
	bitboards[12 + sideToMove] ^= fromToBB; 

	fiftyMoveDistance++;
	if (captured != Empty)
	{
		makeCapture(captured, to);
		clearBit(bitboards[14], from);
	}
	else
	{
		bitboards[14] ^= fromToBB;
	}

	if ((piece - sideToMove * 6) == Pawn)
	{
		fiftyMoveDistance = 0;
		pawnHash ^= (pieceHash[Pawn + sideToMove * 6][from] ^ pieceHash[Pawn + sideToMove * 6][to]);
		
		if (abs(to - from) == 16)
		{
			enPassantSquare = from + 8 - 16 * sideToMove;
		}

		if (promotion == Pawn)
		{
			clearBit(bitboards[Pawn + !sideToMove * 6], to - 8 + 16 * sideToMove);
			clearBit(bitboards[12 + !sideToMove], to - 8 + 16 * sideToMove);
			clearBit(bitboards[14], to - 8 + 16 * sideToMove);
			board[to - 8 + 16 * sideToMove] = Empty;
			hash ^= pieceHash[Pawn + !sideToMove * 6][to - 8 + 16 * sideToMove];
			pawnHash ^= pieceHash[Pawn + !sideToMove * 6][to - 8 + 16 * sideToMove];
		}
		else if (promotion != Empty)
		{
			makePromotion(promotion, to);
		}
	}
	else if ((piece - sideToMove * 6) == Knight)
	{
	}
	else if ((piece - sideToMove * 6) == Bishop)
	{
	}
	else if ((piece - sideToMove * 6) == Rook)
	{
		// Try to make this part colourblind.
		if (sideToMove == White)
		{
			if (from == H1)
			{
				if (castlingRights & WhiteOO)
				{
					hash ^= castlingRightsHash[WhiteOO];
					castlingRights ^= WhiteOO;
				}
			}
			else if (from == A1)
			{
				if (castlingRights & WhiteOOO)
				{
					hash ^= castlingRightsHash[WhiteOOO];
					castlingRights ^= WhiteOOO;
				}
			}
		}
		else
		{
			if (from == H8)
			{
				if (castlingRights & BlackOO)
				{
					hash ^= castlingRightsHash[BlackOO];
					castlingRights ^= BlackOO;
				}
			}
			else if (from == A8)
			{
				if (castlingRights & BlackOOO)
				{
					hash ^= castlingRightsHash[BlackOOO];
					castlingRights ^= BlackOOO;
				}
			}
		}
	}
	else if ((piece - sideToMove * 6) == Queen)
	{
	}
	else if ((piece - sideToMove * 6) == King)
	{
		if (castlingRights & (WhiteOO << (sideToMove * 2)))
		{
			hash ^= castlingRightsHash[WhiteOO << (sideToMove * 2)];
		}
		if (castlingRights & (WhiteOOO << (sideToMove * 2)))
		{
			hash ^= castlingRightsHash[WhiteOOO << (sideToMove * 2)];
		}

		// Make this part check the legality of the castling move sometime.
		if (promotion == King)
		{
			if (to == G1)
			{
				fromRook = H1;
				toRook = F1;
			}
			else if (to == C1)
			{
				fromRook = A1;
				toRook = D1;
			}
			else if (to == G8)
			{
				fromRook = H8;
				toRook = F8;
			}
			else if (to == C8)
			{
				fromRook = A8;
				toRook = D8;
			}

			clearBit(bitboards[Rook + sideToMove * 6], fromRook);
			clearBit(bitboards[12 + sideToMove], fromRook);
			setBit(bitboards[Rook + sideToMove * 6], toRook);
			setBit(bitboards[12 + sideToMove], toRook);
			hash ^= (pieceHash[Rook + sideToMove * 6][fromRook] ^ pieceHash[Rook + sideToMove * 6][toRook]);
		}
	}

	sideToMove = !sideToMove;
	hash ^= turnHash;
	bitboards[15] = ~bitboards[14];

	if (inCheck(!sideToMove))
	{
		unmakeMove(m);
		return false;
	}

	if (promotion == King)
	{
		if (attack(from, sideToMove) || attack(toRook, sideToMove))
		{
			unmakeMove(m);
			return false;
		}
	}
	
	return true;
}

void Position::unmakeMove(Move m)
{
	uint64_t fromBB = 0, fromToBB = 0;
	int from = m.getFrom();
	int to = m.getTo();
	int promotion = m.getPromotion();
	int piece = board[to];
	int captured;

	hply--;
	enPassantSquare = historyStack[hply].ep;
	fiftyMoveDistance = historyStack[hply].fifty;
	captured = historyStack[hply].captured;
	hash = historyStack[hply].hash;
	pawnHash = historyStack[hply].pHash;

	sideToMove = !sideToMove;
	hash ^= turnHash;

	setBit(fromBB, from);
	setBit(fromToBB, to);
	fromToBB |= fromBB;

	board[from] = board[to];
	board[to] = captured;

	hash ^= (pieceHash[piece][from] ^ pieceHash[piece][to]);
	bitboards[piece] ^= fromToBB;
	bitboards[12 + sideToMove] ^= fromToBB;

	if (captured != Empty)
	{
		unmakeCapture(captured, to);
		clearBit(bitboards[14], from);
	}
	else
	{
		bitboards[14] ^= fromToBB;
	}

	bitboards[15] = ~bitboards[14];

	if ((piece - sideToMove * 6) == Pawn)
	{
		pawnHash ^= (pieceHash[Pawn + sideToMove * 6][from] ^ pieceHash[Pawn + sideToMove * 6][to]);

		if (promotion == Pawn)
		{
			clearBit(bitboards[Pawn + !sideToMove * 6], to - 8 + 16 * sideToMove);
			clearBit(bitboards[12 + !sideToMove], to - 8 + 16 * sideToMove);
			clearBit(bitboards[14], to - 8 + 16 * sideToMove);
			board[to - 8 + 16 * sideToMove] = Pawn + !sideToMove * 6;
			hash ^= pieceHash[Pawn + !sideToMove * 6][to - 8 + 16 * sideToMove];
			pawnHash ^= pieceHash[Pawn + !sideToMove * 6][to - 8 + 16 * sideToMove];
		}
		else if (promotion != Empty)
		{
			unmakePromotion(promotion, to);
		}
	}
	else if ((piece - sideToMove * 6) == Knight)
	{
	}
	else if ((piece - sideToMove * 6) == Bishop)
	{
	}
	else if ((piece - sideToMove * 6) == Rook)
	{
	}
	else if ((piece - sideToMove * 6) == Queen)
	{
	}
	else if ((piece - sideToMove * 6) == King)
	{
		if (promotion == King)
		{
			int fromRook, toRook;
			if (to == G1)
			{
				fromRook = H1;
				toRook = F1;
			}
			else if (to == C1)
			{
				fromRook = A1;
				toRook = D1;
			}
			else if (to == G8)
			{
				fromRook = H8;
				toRook = F8;
			}
			else if (to == C8)
			{
				fromRook = A8;
				toRook = D8;
			}

			clearBit(bitboards[Rook + sideToMove * 6], toRook);
			clearBit(bitboards[12 + sideToMove], toRook);
			setBit(bitboards[Rook + sideToMove * 6], fromRook);
			setBit(bitboards[12 + sideToMove], fromRook);
			hash ^= (pieceHash[Rook + sideToMove * 6][fromRook] ^ pieceHash[Rook + sideToMove * 6][toRook]);
		}
	}
}

void Position::makeCapture(int captured, int to)
{
	hash ^= pieceHash[captured][to];
	clearBit(bitboards[captured], to);
	clearBit(bitboards[12 + !sideToMove], to);
	fiftyMoveDistance = 0;

	if (captured - !sideToMove * 6 == Pawn)
	{
		pawnHash ^= pieceHash[Pawn + !sideToMove * 6][to];
	}
	else if (captured - !sideToMove * 6 == Rook)
	{
		if (to == H1 && (castlingRights & WhiteOO))
		{
			hash ^= castlingRightsHash[WhiteOO];
			castlingRights ^= WhiteOO;
		}
		else if (to == A1 && (castlingRights & WhiteOOO))
		{
			hash ^= castlingRightsHash[WhiteOOO];
			castlingRights ^= WhiteOOO;
		}
		else if (to == H1 && (castlingRights & BlackOO))
		{
			hash ^= castlingRightsHash[BlackOO];
			castlingRights ^= BlackOO;
		}
		else if (to == A8 && (castlingRights & BlackOOO))
		{
			hash ^= castlingRightsHash[BlackOOO];
			castlingRights ^= BlackOOO;
		}
	}
}

void Position::unmakeCapture(int captured, int to)
{
	hash ^= pieceHash[captured][to];
	clearBit(bitboards[captured], to);
	clearBit(bitboards[12 + !sideToMove], to);
	fiftyMoveDistance = 0;

	if (captured - !sideToMove * 6 == Pawn)
	{
		pawnHash ^= pieceHash[Pawn + !sideToMove * 6][to];
	}
}

void Position::makePromotion(int promotion, int to)
{
	clearBit(bitboards[Pawn + sideToMove * 6], to);
	setBit(bitboards[promotion + sideToMove * 6], to);

	hash ^= (pieceHash[Pawn + sideToMove * 6][to] ^ pieceHash[promotion + sideToMove * 6][to]);
	pawnHash ^= pieceHash[Pawn + sideToMove * 6][to];
}

void Position::unmakePromotion(int promotion, int to)
{
	setBit(bitboards[Pawn + sideToMove * 6], to);
	clearBit(bitboards[promotion + sideToMove * 6], to);

	hash ^= (pieceHash[Pawn + sideToMove * 6][to] ^ pieceHash[promotion + sideToMove * 6][to]);
	pawnHash ^= pieceHash[Pawn + sideToMove * 6][to];
}

bool Position::inCheck(bool side)
{
	int i = bitScanForward(bitboards[King + side * 6]);
	return attack(i, !side);
}

bool Position::attack(int sq, bool side)
{
	if (knightAttacks[sq] & bitboards[Knight + side * 6] || pawnAttacks[!side][sq] & bitboards[Pawn + side * 6] || kingAttacks[sq] & bitboards[King + side * 6])
	{
		return true;
	}
	if (queenAttacks(sq, bitboards[14]) & (bitboards[Bishop + side * 6] | bitboards[Rook + side * 6] | bitboards[Queen + side * 6]))
	{
		return true;
	}
	return false;
}

#endif