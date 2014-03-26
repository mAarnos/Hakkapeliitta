#ifndef MOVE_CPP
#define MOVE_CPP

#include "move.h"

void Move::clear()
{
	moveInt = 0;
}    

void Move::setFrom(int from)  
{  
	moveInt &= 0xffffffc0; 
	moveInt |= from;
}
 
void Move::setTo(int to)  
{  
	moveInt &= 0xfffff03f; 
	moveInt |= to << 6;
}
 
void Move::setPromotion(int promotion)
{  
	moveInt &= 0xffff0fff;
	moveInt |= promotion << 12;
}

void Move::setScore(int score)
{
	moveInt &= 0x0000ffff;
	moveInt |= score << 16;
}

int Move::getMoveInt()
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
 
int Move::getPromotion()  
{ 
	return (moveInt >> 12) & 0x0000000f; 
}   

int Move::getScore()
{
	return (moveInt >> 16);
}
 
#endif