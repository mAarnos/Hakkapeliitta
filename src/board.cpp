#ifndef BOARD_CPP
#define BOARD_CPP

#include "board.h"
#include "eval.h"
#include "move.h"
#include "bitboard.h"
#include "Shlwapi.h"
#include "movegen.h"
#include "hash.h"
#include <vector>
#include <intrin.h>
#include <boost/algorithm/string.hpp>

using namespace std;

U64 whiteKing;
U64 whiteQueens;
U64 whiteRooks;
U64 whiteBishops;
U64 whiteKnights;
U64 whitePawns;
U64 blackKing;
U64 blackQueens; 
U64 blackRooks;
U64 blackBishops;
U64 blackKnights;
U64 blackPawns;
U64 whitePieces;
U64 blackPieces;
U64 occupiedSquares;

int wp; 
int bp;
int wn;
int bn;
int wb;
int bb;
int wr;
int br;
int wq;
int bq;

int phase;

int materialOpening = 0;
int materialEnding = 0;

int scoreOpeningPST = 0;
int scoreEndingPST = 0;

bool sideToMove;   
int castling; 
int enPassant;                  
int fiftyMoveDistance;

int ply;
int hply;

U64 Hash;
U64 pHash;

int Distance[64][64];

// starting position of a chess game

int startPosition[64] = { 
	6, 3, 5, 7, 2, 5, 3, 6,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	9, 9, 9, 9, 9, 9, 9, 9,
	14, 11, 13, 15, 10, 13, 11, 14
};

// initializes the start position
 void initializeBoard()
{
	// initializeFromSquares(startPosition, 0, 15, 64, 0); this is the non-fen initialization
	// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 = start position FEN
	initializeBoardFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

int initializeBoardFromFen(string aFEN) 
{ 
	unsigned int i, j; 
	int sq; 
	char letter; 
	int aRank,aFile; 
	vector<string> strList; 
	boost::split( strList , aFEN, boost::is_any_of(" ")); 
	for (sq=A1;sq<=H8;sq++) 
		startPosition[sq] = Empty; // Empty the board quares  
		j = 1; i = 0; 
		while ((j<=64) && (i<=strList[0].length())) // read the board - translate each loop idx into a square 
		{ 
			letter = strList[0].at(i); 
			i++; 
			aFile = 1+((j-1) % 8); 
			aRank = 8-((j-1) / 8); 
			sq = (int) (((aRank-1)*8) + (aFile - 1)); 
			switch (letter) 
			{ 
				case 'p' : startPosition[sq] = BlackPawn; break; 
				case 'r' : startPosition[sq] = BlackRook; break; 
				case 'n' : startPosition[sq] = BlackKnight; break; 
				case 'b' : startPosition[sq] = BlackBishop; break; 
				case 'q' : startPosition[sq] = BlackQueen; break; 
				case 'k' : startPosition[sq] = BlackKing; break; 
				case 'P' : startPosition[sq] = WhitePawn; break; 
				case 'R' : startPosition[sq] = WhiteRook; break; 
				case 'N' : startPosition[sq] = WhiteKnight; break; 
				case 'B' : startPosition[sq] = WhiteBishop; break; 
				case 'Q' : startPosition[sq] = WhiteQueen; break; 
				case 'K' : startPosition[sq] = WhiteKing; break; 
				case '/' : j--; break; 
				case '1' : break; 
				case '2' : j++; break; 
				case '3' : j+=2; break; 
				case '4' : j+=3; break; 
				case '5' : j+=4; break; 
				case '6' : j+=5; break; 
				case '7' : j+=6; break; 
				case '8' : j+=7; break; 
				default: return -1; 
			} 
			j++; 
		}
	
	// set the turn; default = White 
	sideToMove = White; 
	if (strList.size()>=2) 
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
	castling = 0; 
	// Initialize all castle possibilities 
	if (strList.size()>=3) 
	{ 
		if (strList[2].find('K') != string::npos) 
		{	
			castling += 1; 
		}
		if (strList[2].find('Q') != string::npos) 
		{	
			castling += 2; 
		}
		if (strList[2].find('k') != string::npos) 
		{
			castling += 4;
		} 
		if (strList[2].find('q') != string::npos) 
		{
			castling += 8; 
		}	
	}
	
	// Set the en passant square, if any. Default is 64 which means no en passant
	enPassant = 64; 
	if ((strList.size()>=4) && (strList[3].length()>=2)) 
	{ 
		if ((strList[3].at(0)>='a') && (strList[3].at(0)<='h') 
		&& ((strList[3].at(1)=='3') || (strList[3].at(1)=='6'))) 
		{ 
			aFile = strList[3].at(0) - 96; // ASCII 'a' = 97 
			aRank = strList[3].at(1) - 48; // ASCII '1' = 49 
			enPassant = (int)((aRank-1)*8 + aFile-1); 
		} 
		else return -1; 
	}
	
	// fifty move distance, we start at 0 per default
	fiftyMoveDistance = 0; 
	if (strList.size()>=5) 
	{ 
		fiftyMoveDistance = atoi(strList[4].c_str()); 
	}

	ply = 0;
	// we start at hply 0 per default
	hply = 0; 
	if (strList.size()>=6) 
	{ 
		hply = 2 * atoi(strList[5].c_str()) -1; 
		if (hply<0) 
		{
			hply = 0; // avoid possible underflow 
		}
		if (sideToMove==Black) 
		{
			hply++;
		}
	}
	
	whiteKing    = 0;
    whiteQueens  = 0;
    whiteRooks   = 0;
    whiteBishops = 0;
    whiteKnights = 0;
    whitePawns   = 0;
    blackKing    = 0;
    blackQueens  = 0;
    blackRooks   = 0;
    blackBishops = 0;
    blackKnights = 0;
    blackPawns   = 0;
    whitePieces  = 0;
    blackPieces  = 0;
    occupiedSquares = 0; 

	wp = 0; 
	bp = 0;
	wn = 0;
	bn = 0;
	wb = 0;
	bb = 0;
	wr = 0;
	br = 0;
	wq = 0;
	bq = 0;

	scoreOpeningPST = 0;
	scoreEndingPST = 0;

	// populate the bitboards
	for (i = A1; i <= H8; i++) {
		U64 mask = (U64)1 << i;
		if (startPosition[i] == WhitePawn)
		{
			whitePawns |= mask; 
			wp += 1;
			scoreOpeningPST += pawnPSTOpening[i];
			scoreEndingPST += pawnPSTEnding[i];
		}
		else if (startPosition[i] == WhiteKnight)
		{
			whiteKnights |= mask;
			wn += 1;
			scoreOpeningPST += knightPSTOpening[i];
			scoreEndingPST += knightPSTEnding[i];
		}
		else if (startPosition[i] == WhiteBishop)
		{
			whiteBishops |= mask; 
			wb += 1;
			scoreOpeningPST += bishopPSTOpening[i];
			scoreEndingPST += bishopPSTEnding[i];
		}
		else if (startPosition[i] == WhiteRook)
		{
			whiteRooks |= mask; 
			wr += 1;
			scoreOpeningPST += rookPSTOpening[i];
			scoreEndingPST += rookPSTEnding[i];
		}
		else if (startPosition[i] == WhiteQueen)
		{
			whiteQueens |= mask;
			wq += 1;
			scoreOpeningPST += queenPSTOpening[i];
			scoreEndingPST += queenPSTEnding[i];
		}
		else if (startPosition[i] == WhiteKing)
		{
			whiteKing = mask; 
			scoreOpeningPST += kingPSTOpening[i];
			scoreEndingPST += kingPSTEnding[i];
		}
		else if (startPosition[i] == BlackPawn)
		{
			blackPawns |= mask; 
			bp += 1;
			scoreOpeningPST -= pawnPSTOpening[flip[i]];
			scoreEndingPST -= pawnPSTEnding[flip[i]];
		}
		else if (startPosition[i] == BlackKnight)
		{
			blackKnights |= mask; 
			bn += 1;
			scoreOpeningPST -= knightPSTOpening[flip[i]];
			scoreEndingPST -= knightPSTEnding[flip[i]];
		}
		else if (startPosition[i] == BlackBishop)
		{
			blackBishops |= mask; 
			bb += 1;
			scoreOpeningPST -= bishopPSTOpening[flip[i]];
			scoreEndingPST -= bishopPSTEnding[flip[i]];
		}
		else if (startPosition[i] == BlackRook)
		{
			blackRooks |= mask; 
			br += 1;
			scoreOpeningPST -= rookPSTOpening[flip[i]];
			scoreEndingPST -= rookPSTEnding[flip[i]];
		}
		else if (startPosition[i] == BlackQueen)
		{
			blackQueens |= mask; 
			bq += 1;
			scoreOpeningPST -= queenPSTOpening[flip[i]];
			scoreEndingPST -= queenPSTEnding[flip[i]];
		}
		else if (startPosition[i] == BlackKing)
		{
			blackKing = mask; 
			scoreOpeningPST -= kingPSTOpening[flip[i]];
			scoreEndingPST -= kingPSTEnding[flip[i]];
		}
	}
	whitePieces = whiteKing | whiteQueens | whiteRooks | whiteBishops | whiteKnights | whitePawns;
    blackPieces = blackKing | blackQueens | blackRooks | blackBishops | blackKnights | blackPawns;
    occupiedSquares = whitePieces | blackPieces;

	materialOpening = 0;
	materialEnding = 0;

	// Calculate the difference in material. Notice the opening/ending split for the tapered eval
	materialOpening = wp * pawnOpening +
		              wn * knightOpening +
					  wb * bishopOpening +
					  wr * rookOpening +
					  wq * queenOpening; 

	materialEnding =  wp * pawnEnding +
		              wn * knightEnding +
					  wb * bishopEnding +
					  wr * rookEnding +
					  wq * queenEnding;

	materialOpening -= bp * pawnOpening +
		               bn * knightOpening +
					   bb * bishopOpening +
					   br * rookOpening +
					   bq * queenOpening; 

	materialEnding -=  bp * pawnEnding +
		               bn * knightEnding +
					   bb * bishopEnding +
					   br * rookEnding +
					   bq * queenEnding; 

	// calculate the phase of the game for tapered eval
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

	Hash = setHash();
	pHash = setPHash();

	return 0;
}

int convertStringToBitboard(char * a)
{
	int sq = 0;
	
	if (a[0] == 'a')
		sq = 0;
	else if (a[0] == 'b')
		sq = 1;
	else if (a[0] == 'c')
		sq = 2;
	else if (a[0] == 'd')
		sq = 3;
	else if (a[0] == 'e')
		sq = 4;
	else if (a[0] == 'f')
		sq = 5;
	else if (a[0] == 'g')
		sq = 6;
	else if (a[0] == 'h')
		sq = 7;
	
	if (a[1] == '1')
		sq += 0;
	else if (a[1] == '2')
		sq += 8 * 1;
	else if (a[1] == '3')
		sq += 8 * 2;
	else if (a[1] == '4')
		sq += 8 * 3;
	else if (a[1] == '5')
		sq += 8 * 4;
	else if (a[1] == '6')
		sq += 8 * 5;
	else if (a[1] == '7')
		sq += 8 * 6;
	else if (a[1] == '8')
		sq += 8 * 7;
	
	return sq;
}

int algebraicMoves(char * a) {
 
    Move m;
	int from, to;
 
    while ((a[0] >= 'a') && (a[0] <= 'h')) 
	{
		m.clear();
		
		from = convertStringToBitboard(a);
		to = convertStringToBitboard(a+2);
			
        m.setFrom(from);
        m.setTo(to);
        m.setPiece(getPiece(from));
        m.setCapture(getPiece(to));
 
		if (m.isWhitemove())
		{
			switch (a[4]) 
			{
			case 'q': m.setPromotion(WhiteQueen); a++; break;
			case 'r': m.setPromotion(WhiteRook); a++; break;
			case 'b': m.setPromotion(WhiteBishop); a++; break;
			case 'n': m.setPromotion(WhiteKnight); a++; break;
			}
		}
		else 
		{
			switch (a[4]) 
			{
			case 'q': m.setPromotion(BlackQueen); a++; break;
			case 'r': m.setPromotion(BlackRook); a++; break;
			case 'b': m.setPromotion(BlackBishop); a++; break;
			case 'n': m.setPromotion(BlackKnight); a++; break;
			}
		}
 
        //castling
        if (m.getPiece() == WhiteKing && from == E1 && to == G1) 
			m.moveInt = whiteShortCastle;
        else if (m.getPiece() == WhiteKing && from == E1 && to == C1) 
			m.moveInt = whiteLongCastle;
		else if (m.getPiece() == BlackKing && from == E8 && to == G8) 
			m.moveInt = blackShortCastle;
        else if (m.getPiece() == BlackKing && from == E8 && to == C8) 
			m.moveInt = blackLongCastle;
 
        // ep
		if (m.getPiece() == WhitePawn && m.getCapture() == Empty && (to - from == 9 || to - from == 7))
		{
			m.setPromotion(WhitePawn);
			m.setCapture(BlackPawn);
		}
		
 		if (m.getPiece() == BlackPawn && m.getCapture() == Empty && (from - to == 9 || from - to == 7))
		{
			m.setPromotion(BlackPawn);
			m.setCapture(WhitePawn);
		}
		
        make(m.moveInt);
 
        a += 4;
 
        while (a[0]==' ') a++;
    }
    return 0;
}

#endif