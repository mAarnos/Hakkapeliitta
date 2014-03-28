#ifndef POSITION_CPP
#define POSITION_CPP

#include "position.h"
#include "bitboard.h"
#include "hash.h"
#include "magic.h"

void Position::displayBoard()
{
	string pieceToMark = "PNBRQKpnbrqk.";
	cout << "  +-----------------------+" << endl;
	for (int i = 7; i >= 0; i--)
	{
		cout << i + 1 << " ";
		for (int j = 0; j < 8; j++)
		{
			cout << "|" << pieceToMark[board[i * 8 + j]] << " ";
		}
		cout << "|" << endl << "  +--+--+--+--+--+--+--+--+" << endl;
	}
	cout << "   A  B  C  D  E  F  G  H" << endl;
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
		// Avoid possible underflow.
		if (hply < 0)
		{
			hply = 0; 
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
		if (board[i] != Empty)
		{
			bitboards[board[i]] |= bit[i];
		}
	}
	bitboards[12] = bitboards[WhiteKing] | bitboards[WhiteQueen] | bitboards[WhiteRook] | bitboards[WhiteBishop] | bitboards[WhiteKnight] | bitboards[WhitePawn];
	bitboards[13] = bitboards[BlackKing] | bitboards[BlackQueen] | bitboards[BlackRook] | bitboards[BlackBishop] | bitboards[BlackKnight] | bitboards[BlackPawn];
	bitboards[14] = bitboards[12] | bitboards[13];
	bitboards[15] = ~(bitboards[14]);

	hash = calculateHash();
	pawnHash = calculatePawnHash();
	matHash = calculateMaterialHash();

	return;
}

bool Position::makeMove(Move m)
{
	uint64_t fromBB, fromToBB;
	int fromRook, toRook;
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
	historyStack[hply].mHash = matHash;
	hply++;
	
	fromToBB = fromBB = bit[from];
	fromToBB |= bit[to];

	board[to] = board[from];
	board[from] = Empty;

	if (enPassantSquare != 64)
	{
		hash ^= enPassantHash[enPassantSquare];
		enPassantSquare = 64;
	}
	fiftyMoveDistance++;

	bitboards[piece] ^= fromToBB;
	bitboards[12 + sideToMove] ^= fromToBB; 

	hash ^= (pieceHash[piece][from] ^ pieceHash[piece][to]);

	if (captured != Empty)
	{
		makeCapture(captured, to);
		bitboards[14] ^= bit[from];
	}
	else
	{
		bitboards[14] ^= fromToBB;
	}

	int pieceType = piece % Pieces;
	if (pieceType == Pawn)
	{
		fiftyMoveDistance = 0;
		pawnHash ^= (pieceHash[piece][from] ^ pieceHash[piece][to]);

		// Check if the move is a double pawn move. If it is update the en passant square.
		if (abs(to - from) == 16)
		{
			enPassantSquare = from + 8 - 16 * sideToMove;
			hash ^= enPassantHash[enPassantSquare];
		}

		// Handle en passant and promotions.
		if (promotion == Pawn)
		{
			// Move these to a doEnPassant function or something.
			int enPassantPawn = to - 8 + 16 * sideToMove;
			// Since a pawn is either 0(white) or 6(black) if we reduce from 6 we get the opposite coloured pawn.
			bitboards[6 - piece] ^= bit[enPassantPawn];
			bitboards[12 + !sideToMove] ^= bit[enPassantPawn];
			bitboards[14] ^= bit[enPassantPawn];
			board[enPassantPawn] = Empty;
			hash ^= pieceHash[6 - piece][enPassantPawn];
			pawnHash ^= pieceHash[6 - piece][enPassantPawn];
			matHash ^= materialHash[6 - piece][popcnt(bitboards[6 - piece])];
		}
		else if (promotion != Empty)
		{
			makePromotion(promotion, to);
		}
	}
	else if (pieceType == Rook)
	{
		// Update the castling rights after a rook move if necessary.
		// Try to make this colourblind.
		if (from == H1 && (castlingRights & WhiteOO))
		{
			castlingRights ^= WhiteOO;
			hash ^= castlingRightsHash[WhiteOO];
		}
		else if (from == A1 && (castlingRights & WhiteOOO))
		{
			castlingRights ^= WhiteOOO;
			hash ^= castlingRightsHash[WhiteOOO];
		}
		else if (from == H8 && (castlingRights & BlackOO))
		{
			castlingRights ^= BlackOO;
			hash ^= castlingRightsHash[BlackOO];
		}
		else if (from == A8 && (castlingRights & BlackOOO))
		{
			castlingRights ^= BlackOOO;
			hash ^= castlingRightsHash[BlackOOO];
		}
	}
	else if (pieceType == King)
	{
		// Updates the castling rights after a king move.
		if (castlingRights & (WhiteOO << (sideToMove * 2)))
		{
			castlingRights ^= WhiteOO << (sideToMove * 2);
			hash ^= castlingRightsHash[WhiteOO << (sideToMove * 2)];
		}
		if (castlingRights & (WhiteOOO << (sideToMove * 2)))
		{
			castlingRights ^= WhiteOOO << (sideToMove * 2);
			hash ^= castlingRightsHash[WhiteOOO << (sideToMove * 2)];
		}

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
			else
			{
				fromRook = A8;
				toRook = D8;
			}

			bitboards[Rook + sideToMove * 6] ^= bit[fromRook] | bit[toRook];
			bitboards[12 + sideToMove] ^= bit[fromRook] | bit[toRook];
			bitboards[14] ^= bit[fromRook] | bit[toRook];
			board[toRook] = board[fromRook];
			board[fromRook] = Empty;
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

	return true;
}

void Position::unmakeMove(Move m)
{
	uint64_t fromBB, fromToBB;
	int from = m.getFrom();
	int to = m.getTo();
	int promotion = m.getPromotion();
	int piece = board[to];

	hply--;
	castlingRights = historyStack[hply].castle;
	enPassantSquare = historyStack[hply].ep;
	fiftyMoveDistance = historyStack[hply].fifty;
	int captured = historyStack[hply].captured;
	hash = historyStack[hply].hash;
	pawnHash = historyStack[hply].pHash;
	matHash = historyStack[hply].mHash;

	sideToMove = !sideToMove;

	// How to get rid of this?
	if (promotion != Empty && promotion != King)
	{
		piece = Pawn + sideToMove * 6;
	}

	fromToBB = fromBB = bit[from];
	fromToBB |= bit[to];

	bitboards[piece] ^= fromToBB;
	bitboards[12 + sideToMove] ^= fromToBB;

	board[from] = piece;
	board[to] = captured;

	if (captured != Empty)
	{
		unmakeCapture(captured, to);
		bitboards[14] |= bit[from];
	}
	else
	{
		bitboards[14] ^= fromToBB;
	}

	int pieceType = piece % Pieces;
	if (pieceType == Pawn)
	{
		if (promotion == Pawn)
		{
			int enPassantPawn = to - 8 + 16 * sideToMove;
			bitboards[Pawn + !sideToMove * 6] |= bit[enPassantPawn];
			bitboards[12 + !sideToMove] |= bit[enPassantPawn];
			bitboards[14] |= bit[enPassantPawn];
			board[enPassantPawn] = Pawn + !sideToMove * 6;
		}
		else if (promotion != Empty)
		{
			unmakePromotion(promotion, to);
		}
	}
	else if (pieceType == King)
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
			else
			{
				fromRook = A8;
				toRook = D8;
			}

			bitboards[Rook + sideToMove * 6] ^= bit[fromRook] | bit[toRook];
			bitboards[12 + sideToMove] ^= bit[fromRook] | bit[toRook];
			bitboards[14] ^= bit[fromRook] | bit[toRook];
			board[fromRook] = board[toRook];
			board[toRook] = Empty;
		}
	}

	bitboards[15] = ~bitboards[14];

}

void Position::makeCapture(int captured, int to)
{
	bitboards[captured] ^= bit[to];
	bitboards[12 + !sideToMove] ^= bit[to];
	fiftyMoveDistance = 0;

	hash ^= pieceHash[captured][to];
	matHash ^= materialHash[captured][popcnt(bitboards[captured])];

	int pieceType = (captured % Pieces);
	if (pieceType == Pawn)
	{
		pawnHash ^= pieceHash[captured][to];
	}
	else if (pieceType == Rook)
	{
		if (to == H1 && (castlingRights & WhiteOO))
		{
			castlingRights ^= WhiteOO;
			hash ^= castlingRightsHash[WhiteOO];
		}
		else if (to == A1 && (castlingRights & WhiteOOO))
		{
			castlingRights ^= WhiteOOO;
			hash ^= castlingRightsHash[WhiteOOO];
		}
		else if (to == H8 && (castlingRights & BlackOO))
		{
			castlingRights ^= BlackOO;
			hash ^= castlingRightsHash[BlackOO];
		}
		else if (to == A8 && (castlingRights & BlackOOO))
		{
			castlingRights ^= BlackOOO;
			hash ^= castlingRightsHash[BlackOOO];
		}
	}
}

void Position::unmakeCapture(int captured, int to)
{
	bitboards[captured] |= bit[to];
	bitboards[12 + !sideToMove] |= bit[to];
}

void Position::makePromotion(int promotion, int to)
{
	// This needs to be above the rest due to reasons. Try to fix that.
	matHash ^= materialHash[promotion + sideToMove * 6][popcnt(bitboards[promotion + sideToMove * 6])];

	bitboards[Pawn + sideToMove * 6] ^= bit[to];
	bitboards[promotion + sideToMove * 6] |= bit[to];
	hash ^= pieceHash[board[to]][to] ^ pieceHash[promotion + sideToMove * 6][to];
	pawnHash ^= pieceHash[board[to]][to];
	matHash ^= materialHash[Pawn + sideToMove * 6][popcnt(bitboards[Pawn + sideToMove * 6])];
	board[to] = promotion + sideToMove * 6;
}

void Position::unmakePromotion(int promotion, int to)
{
	bitboards[Pawn + sideToMove * 6] ^= bit[to];
	bitboards[promotion + sideToMove * 6] ^= bit[to];
}

bool Position::attack(int sq, bool side)
{
	if (knightAttacks[sq] & bitboards[Knight + side * 6] || pawnAttacks[!side][sq] & bitboards[Pawn + side * 6] || kingAttacks[sq] & bitboards[King + side * 6])
	{
		return true;
	}
	uint64_t BQ = bitboards[Bishop + side * 6] | bitboards[Queen + side * 6];
	if ((bishopAttacks(sq, bitboards[14]) & BQ))
	{
		return true;
	}
	uint64_t RQ = bitboards[Rook + side * 6] | bitboards[Queen + side * 6];
	if (rookAttacks(sq, bitboards[14]) & RQ)
	{
		return true;
	}
	return false;
}

#endif