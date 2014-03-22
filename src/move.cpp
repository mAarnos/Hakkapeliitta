#ifndef MOVE_CPP
#define MOVE_CPP

#include "move.h"

void Move::clear()
{
	moveInt = 0;
	score = 0;
}    

void Move::setFrom(int from)  
{  // bits  0.. 5
	moveInt &= 0xffffffc0; 
	moveInt |= (from & 0x0000003f);
}
 
void Move::setTo(int to)  
{  // bits  6..11        
	moveInt &= 0xfffff03f; 
	moveInt |= (to & 0x0000003f) << 6;
}
 
void Move::setPiece(int piece)  
{  // bits 12..15
	moveInt &= 0xffff0fff;
	moveInt |= (piece & 0x0000000f) << 12;
} 
 
void Move::setCapture(int capture)  
{  // bits 16..19
	moveInt &= 0xfff0ffff; 
	moveInt |= (capture & 0x0000000f) << 16;
} 
 
void Move::setPromotion(int promotion)  
{  // bits 20..23
	moveInt &= 0xff0fffff;
	moveInt |= (promotion & 0x0000000f) << 20;
}  

// read Move information:
// first shift right, then mask to get the info

int Move::getFrom()  
{ // 6 bits (value 0..63), position  0.. 5
	return (moveInt & 0x0000003f);
}  
 
int Move::getTo()  
{  // 6 bits (value 0..63), position  6..11
	return (moveInt >>  6) & 0x0000003f; 
}   
 
int Move::getPiece()  
{ // 4 bits (value 0..15), position 12..15
	return (moveInt >> 12) & 0x0000000f; 
}   
 
int Move::getCapture()  
{ // 4 bits (value 0..15), position 16..19
	return (moveInt >> 16) & 0x0000000f; 
}   
 
int Move::getPromotion()  
{  // 4 bits (value 0..15), position 20..23
	return (moveInt >> 20) & 0x0000000f; 
}    

// boolean checks for some types of moves.
// first mask, then compare
// Note that we are using the bit-wise properties of piece identifiers, so we cannot just change them anymore !

bool Move::isWhitemove()  
{   // piece is white: bit 15 must be 0
	return (~moveInt & 0x00008000) == 0x00008000;
} 
 
bool Move::isBlackmove()  
{   // piece is black: bit 15 must be 1
	return ( moveInt & 0x00008000) == 0x00008000;
} 
 
bool Move::isCapture()    
{  // capture is nonzero, bits 16 to 19 must be nonzero
	return ( moveInt & 0x000f0000) != 0x00000000;
} 
 
bool Move::isKingcaptured()
{  // bits 17 to 19 must be 010
	return ( moveInt & 0x00070000) == 0x00020000;
} 
 
bool Move::isRookmove()
{  // bits 13 to 15 must be 110
	return ( moveInt & 0x00007000) == 0x00006000;
} 
 
bool Move::isRookcaptured()
{  // bits 17 to 19 must be 110
	return ( moveInt & 0x00070000) == 0x00060000;
} 
 
bool Move::isKingmove()
{  // bits 13 to 15 must be 010
	return ( moveInt & 0x00007000) == 0x00002000;
} 
 
bool Move::isPawnmove()
{  // bits 13 to 15 must be 001
	return ( moveInt & 0x00007000) == 0x00001000;
} 
 
bool Move::isPawnDoublemove()
{   // bits 13 to 15 must be 001 &
    // bits 4 to 6 must be 001 (from rank 2) & bits 10 to 12 must be 011 (to rank 4)
    // OR: bits 4 to 6 must be 110 (from rank 7) & bits 10 to 12 must be 100 (to rank 5)
	return ((( moveInt & 0x00007000) == 0x00001000) && (((( moveInt & 0x00000038) == 0x00000008) && ((( moveInt & 0x00000e00) == 0x00000600))) || 
														((( moveInt & 0x00000038) == 0x00000030) && ((( moveInt & 0x00000e00) == 0x00000800)))));
} 
 
bool Move::isEnpassant()  
{  // prom is a pawn, bits 21 to 23 must be 001
	return ( moveInt & 0x00700000) == 0x00100000;
} 
 
bool Move::isPromotion()  
{  // prom (with color bit removed), .xxx > 2 (not king or pawn)
	return ( moveInt & 0x00700000) >  0x00200000;
} 
 
bool Move::isCastle()  
{  // prom is a king, bits 21 to 23 must be 010
	return ( moveInt & 0x00700000) == 0x00200000;
} 
 
bool Move::isCastleOO() 
{  // prom is a king and tosq is on the g-file
	return ( moveInt & 0x007001c0) == 0x00200180;
} 
 
bool Move::isCastleOOO()  
{  // prom is a king and tosq is on the c-file
	return ( moveInt & 0x007001c0) == 0x00200080;
}  

#endif