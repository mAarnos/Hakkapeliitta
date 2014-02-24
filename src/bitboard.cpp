#ifndef BITBOARD_CPP
#define BITBOARD_CPP

#include "bitboard.h"

U64 setMask[Squares+1];
U64 above[Squares+1];
U64 below[Squares+1];
U64 kingAttacks[Squares];
U64 knightAttacks[Squares];
U64 whitePawnAttacks[Squares];
U64 whitePawnMoves[Squares];
U64 blackPawnAttacks[Squares];
U64 blackPawnMoves[Squares];

U64 RaySW[Squares];
U64 RayS[Squares];
U64 RaySE[Squares];
U64 RayW[Squares];
U64 RayE[Squares];
U64 RayNW[Squares];
U64 RayN[Squares];
U64 RayNE[Squares];

U64 passedWhite[Squares];
U64 backwardWhite[Squares];
U64 passedBlack[Squares];
U64 backwardBlack[Squares];
U64 isolated[Squares];

int Headings[Squares][Squares];
U64 between[Squares][Squares];

void initializeBitMasks();
void initializeKingAttacks();
void initializeKnightAttacks();
void initializePawnAttacks();
void initializePawnMoves();
void initializeRays();
void initializePawnEvaluation();
void initializeBetween();

void initializeBitboards() 
{	
   initializeBitMasks();
   initializeKingAttacks();
   initializeKnightAttacks();
   initializePawnAttacks();
   initializePawnMoves();
   initializeRays();
   initializePawnEvaluation();
   initializeBetween();
}

void initializeBitMasks() 
{
	int sq, i;
	for (sq = A1; sq <= H8; sq++) 
	{
		setMask[sq] = (U64)1 << sq;
	}
	setMask[64] = 0;
	above[64] = 0xffffffffffffffff;
	below[64] = 0xffffffffffffffff;
	for (sq = A1; sq <= H8; sq++) 
	{
		above[sq] = 0;
		for (i = sq + 1; i <= 63; i++) {
			above[sq] |= setMask[i];
		}

		below[sq] = 0;
		for (i = 0; i < sq; i++) 
		{
			below[sq] |= setMask[i];
		}
	}
}

void initializeKingAttacks()
{
	int sq;
	U64 notAFile = 0xFEFEFEFEFEFEFEFE;
	U64 notHFile = 0x7F7F7F7F7F7F7F7F;
	U64 kingSet;
	for (sq = A1; sq <= H8; sq++)
	{
		kingSet = setMask[sq];
		kingSet |= (setMask[sq] << 1) & notAFile;
		kingSet |= (setMask[sq] >> 1) & notHFile;
		kingAttacks[sq] = (kingSet ^ setMask[sq]);
		kingAttacks[sq] |= (kingSet << 8) | (kingSet >> 8);
	}
}

void initializeKnightAttacks()
{
	int sq;
	U64 notAFile = 0xFEFEFEFEFEFEFEFE;
	U64 notHFile = 0x7F7F7F7F7F7F7F7F;
	U64 notABFile = 0xFCFCFCFCFCFCFCFC;
	U64 notGHFile = 0x3F3F3F3F3F3F3F3F;
	for (sq = A1; sq <= H8; sq++)
	{
		knightAttacks[sq] = ((setMask[sq] << 17) & notAFile) | ((setMask[sq] << 10) & notABFile) | ((setMask[sq] << 15) & notHFile) | ((setMask[sq] << 6) & notGHFile);
		knightAttacks[sq] |= ((setMask[sq] >> 17) & notHFile) | ((setMask[sq] >> 10) & notGHFile) | ((setMask[sq] >> 15) & notAFile) | ((setMask[sq] >> 6) & notABFile);
	}
}
	
void initializePawnAttacks()
{
	int sq;
	U64 notAFile = 0xFEFEFEFEFEFEFEFE;
	U64 notHFile = 0x7F7F7F7F7F7F7F7F;
	for (sq = A1; sq <= H8; sq++)
	{
		whitePawnAttacks[sq] = ((setMask[sq] << 9) & notAFile) | ((setMask[sq] << 7) & notHFile);
		blackPawnAttacks[sq] = ((setMask[sq] >> 9) & notHFile) | ((setMask[sq] >> 7) & notAFile);
	}
}
   
void initializePawnMoves()
{
	int sq;
	for (sq = A2; sq <= H7; sq++) 
	{
	   whitePawnMoves[sq] = blackPawnMoves[sq] = (U64)0;
	   whitePawnMoves[sq] |= setMask[sq+8]; 
	   blackPawnMoves[sq] |= setMask[sq-8]; 
	   if (sq <= H2) 
	   {
		  whitePawnMoves[sq] |= setMask[sq+16]; 
	   }
	   if (sq >= A7)
	   {
		  blackPawnMoves[sq] |= setMask[sq-16]; 
	   }
    }	
}	

void initializeRays()
{
   int raySWdirections[8][2] = {{-1,-1},{-2,-2},{-3,-3},{-4,-4},{-5,-5},{-6,-6},{-7,-7},{-8,-8}};
   int raySdirections[8][2] = {{-1,0},{-2,0},{-3,0},{-4,0},{-5,0},{-6,0},{-7,0},{-8,0}};
   int raySEdirections[8][2] = {{-1,1},{-2,2},{-3,3},{-4,4},{-5,5},{-6,6},{-7,7},{-8,8}};
   int rayWdirections[8][2] = {{0,-1},{0,-2},{0,-3},{0,-4},{0,-5},{0,-6},{0,-7},{0,-8}};
   int rayEdirections[8][2] = {{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8}};
   int rayNWdirections[8][2] = {{1,-1},{2,-2},{3,-3},{4,-4},{5,-5},{6,-6},{7,-7},{8,-8}};
   int rayNdirections[8][2] = {{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},{7,0},{8,0}};
   int rayNEdirections[8][2] = {{1,1},{2,2},{3,3},{4,4},{5,5},{6,6},{7,7},{8,8}};

   memset(Headings, 0, sizeof(Headings));

   int sq, i;
   for (sq = A1; sq <= H8; sq++) {
      int r = Rank(sq), f = File(sq);

      for (i = 0; i <= 7; i++) {
         int dr = raySWdirections[i][0], df = raySWdirections[i][1]; 
         if (isPositionOnBoard(r+dr,f+df)) {
			 Headings[sq][Square(r+dr,f+df)] = -9;
			 RaySW[sq] |= setMask[Square(r+dr,f+df)];
		 }
		 else break;
	  }
	  
	  for (i = 0; i <= 7; i++) {
         int dr = raySdirections[i][0], df = raySdirections[i][1]; 
         if (isPositionOnBoard(r+dr,f+df)) {
			 Headings[sq][Square(r+dr,f+df)] = -8;
			 RayS[sq] |= setMask[Square(r+dr,f+df)];
		 }
		 else break;
	  }
	  
	  for (i = 0; i <= 7; i++) {
         int dr = raySEdirections[i][0], df = raySEdirections[i][1]; 
         if (isPositionOnBoard(r+dr,f+df)) {
			 Headings[sq][Square(r+dr,f+df)] = -7;
			 RaySE[sq] |= setMask[Square(r+dr,f+df)];
		 }
		 else break;
	  }
	  
	  for (i = 0; i <= 7; i++) {
         int dr = rayWdirections[i][0], df = rayWdirections[i][1]; 
         if (isPositionOnBoard(r+dr,f+df)) {
			 Headings[sq][Square(r+dr,f+df)] = -1;
			 RayW[sq] |= setMask[Square(r+dr,f+df)];
		 }
		 else break;
	  }
	  
	  for (i = 0; i <= 7; i++) {
         int dr = rayEdirections[i][0], df = rayEdirections[i][1]; 
         if (isPositionOnBoard(r+dr,f+df)) {
			 Headings[sq][Square(r+dr,f+df)] = 1;
			 RayE[sq] |= setMask[Square(r+dr,f+df)];
		 }
		 else break;
	  }
	  
	  for (i = 0; i <= 7; i++) {
         int dr = rayNWdirections[i][0], df = rayNWdirections[i][1]; 
         if (isPositionOnBoard(r+dr,f+df)) {
			 Headings[sq][Square(r+dr,f+df)] = 7;
			 RayNW[sq] |= setMask[Square(r+dr,f+df)];
		 }
		 else break;
	  }
	  
	  for (i = 0; i <= 7; i++) {
         int dr = rayNdirections[i][0], df = rayNdirections[i][1]; 
         if (isPositionOnBoard(r+dr,f+df)) {
			 Headings[sq][Square(r+dr,f+df)] = 8;
			 RayN[sq] |= setMask[Square(r+dr,f+df)];
		 }
		 else break;
	  }
	  
	  for (i = 0; i <= 7; i++) {
         int dr = rayNEdirections[i][0], df = rayNEdirections[i][1]; 
         if (isPositionOnBoard(r+dr,f+df)) {
			 Headings[sq][Square(r+dr,f+df)] = 9;
			 RayNE[sq] |= setMask[Square(r+dr,f+df)];
		 }	
		 else break;
	  }
   }
}

void initializePawnEvaluation()
{
	int file, sq;
	for (sq = 0; sq <= 63; sq++)
	{
		file = File(sq);

		// passedWhite
		passedWhite[sq] = RayN[sq];
		if (file == 0)
		{
			passedWhite[sq] |= RayN[sq + 1];
		}
		else if (file == 7)
		{
			passedWhite[sq] |= RayN[sq - 1];
		}
		else 
		{
			passedWhite[sq] |= RayN[sq - 1];
			passedWhite[sq] |= RayN[sq + 1];
		}

		// passedBlack
		passedBlack[sq] = RayS[sq];
		if (file == 0)
		{
			passedBlack[sq] |= RayS[sq + 1];
		}
		else if (file == 7)
		{
			passedBlack[sq] |= RayS[sq - 1];
		}
		else 
		{
			passedBlack[sq] |= RayS[sq - 1];
			passedBlack[sq] |= RayS[sq + 1];
		}

		// isolated
		if (file == 0)
		{
			isolated[sq] |= RayN[1];
			isolated[sq] |= setMask[1];
		}
		else if (file == 7)
		{
			isolated[sq] |= RayN[6];
			isolated[sq] |= setMask[6];
		}
		else 
		{
			isolated[sq] |= RayN[file - 1];
			isolated[sq] |= setMask[file - 1];
			isolated[sq] |= RayN[file + 1];
			isolated[sq] |= setMask[file + 1];
		}

		// backwardWhite
		if (file == 0)
		{
			backwardWhite[sq] = RayS[sq + 9];
		}
		else if (file == 7)
		{
			backwardWhite[sq] = RayS[sq + 7];
		}
		else 
		{
			backwardWhite[sq] = RayS[sq + 9];
			backwardWhite[sq] |= RayS[sq + 7];
		}

		// backwardBlack
		if (file == 0)
		{
			backwardBlack[sq] = RayN[sq - 7];
		}
		else if (file == 7)
		{
			backwardBlack[sq] = RayN[sq - 9];
		}
		else 
		{
			backwardBlack[sq] = RayN[sq - 9];
			backwardBlack[sq] |= RayN[sq - 7];
		}
	}
}

void initializeBetween()
{
	int i, j, heading;

	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 64; j++)
		{
			heading = Headings[i][j];
			if (heading == 9)
			{
				between[i][j] = RayNE[i] & RaySW[j];
			}
			else if (heading == 8)
			{
				between[i][j] = RayN[i] & RayS[j];
			}
			else if (heading == 7)
			{
				between[i][j] = RayNW[i] & RaySE[j];
			}
			else if (heading == 1)
			{
				between[i][j] = RayE[i] & RayW[j];
			}
			else if (heading == -1)
			{
				between[i][j] = RayW[i] & RayE[j];
			}
			else if (heading == -7)
			{
				between[i][j] = RaySE[i] & RayNW[j];
			}
			else if (heading == -8)
			{
				between[i][j] = RayS[i] & RayN[j];
			}
			else if (heading == -9)
			{
				between[i][j] = RaySW[i] & RayNE[j];
			}
		}
	}
}


#endif

