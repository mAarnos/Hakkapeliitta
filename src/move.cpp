#ifndef MOVE_CPP
#define MOVE_CPP

#include "move.h"

void Move::clear()
{
	moveInt = 0;
	score = 0;
}    

void Move::setMove(int move)
{
	moveInt = move;
}

void Move::setFrom(int from)  
{  
	moveInt &= 0xffffffc0; 
	moveInt |= (from & 0x0000003f);
}
 
void Move::setTo(int to)  
{  
	moveInt &= 0xfffff03f; 
	moveInt |= to << 6;
}
 
void Move::setPiece(int piece)  
{  
	moveInt &= 0xffff0fff;
	moveInt |= piece << 12;
} 
 
void Move::setCapture(int capture)  
{  
	moveInt &= 0xfff0ffff; 
	moveInt |= capture << 16;
} 
 
void Move::setPromotion(int promotion)  
{  
	moveInt &= 0xff0fffff;
	moveInt |= promotion << 20;
}  

int Move::getMove()
{
	return moveInt;
}

int Move::getFrom()  
{ 
	return (moveInt & 0x0000003f);
}  
 
int Move::getTo()  
{  
	return (moveInt >>  6) & 0x0000003f; 
}   
 
int Move::getPiece()  
{ 
	return (moveInt >> 12) & 0x0000000f; 
}   
 
int Move::getCapture()  
{ 
	return (moveInt >> 16) & 0x0000000f; 
}   
 
int Move::getPromotion()  
{ 
	return (moveInt >> 20) & 0x0000000f; 
}    

#endif