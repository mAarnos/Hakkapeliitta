#ifndef POSITION_CPP
#define POSITION_CPP

#include "position.h"

int Position::initializeBoardFromFEN(string FEN)
{
	array<int, 64> startPosition;
	unsigned int i, j;
	int sq;
	char letter;
	int aRank, aFile;

	for (sq = A1; sq <= H8; sq++)
	{
		startPosition[sq] = Empty;
	}

	vector<string> strList;

	// Split the FEN into parts here.

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
			case 'p': startPosition[sq] = BlackPawn; break;
			case 'r': startPosition[sq] = BlackRook; break;
			case 'n': startPosition[sq] = BlackKnight; break;
			case 'b': startPosition[sq] = BlackBishop; break;
			case 'q': startPosition[sq] = BlackQueen; break;
			case 'k': startPosition[sq] = BlackKing; break;
			case 'P': startPosition[sq] = WhitePawn; break;
			case 'R': startPosition[sq] = WhiteRook; break;
			case 'N': startPosition[sq] = WhiteKnight; break;
			case 'B': startPosition[sq] = WhiteBishop; break;
			case 'Q': startPosition[sq] = WhiteQueen; break;
			case 'K': startPosition[sq] = WhiteKing; break;
			case '/': j--; break;
			case '1': break;
			case '2': j++; break;
			case '3': j += 2; break;
			case '4': j += 3; break;
			case '5': j += 4; break;
			case '6': j += 5; break;
			case '7': j += 6; break;
			case '8': j += 7; break;
			default: return -1;
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
		else return -1;
	}

	// set castling to default 0 
	castlingRights = 0;
	// Initialize all castle possibilities 
	if (strList.size() >= 3)
	{
		if (strList[2].find('K') != string::npos)
		{
			castlingRights += whiteOO;
		}
		if (strList[2].find('Q') != string::npos)
		{
			castlingRights += whiteOOO;
		}
		if (strList[2].find('k') != string::npos)
		{
			castlingRights += blackOO;
		}
		if (strList[2].find('q') != string::npos)
		{
			castlingRights += blackOOO;
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
		else return -1;
	}

	// Fifty move distance, we start at 0 by default.
	fiftyMoveDistance = 0;
	if (strList.size() >= 5)
	{
		fiftyMoveDistance = stoi(strList[4]);
	}

	ply = 0;
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

	return 0;
}

#endif