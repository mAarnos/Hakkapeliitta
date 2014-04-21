#include "move.h"

void Move::clear()
{
	move = 0;
}    

void Move::setFrom(int from)  
{  
	move &= 0xffffffc0; 
	move |= from;
}
 
void Move::setTo(int to)  
{  
	move &= 0xfffff03f; 
	move |= to << 6;
}
 
void Move::setPromotion(int promotion)
{  
	move &= 0xffff0fff;
	move |= promotion << 12;
}

void Move::setScore(int s)
{
	score = s;
}

void Move::setMove(uint16_t m)
{
	move = m;
}

int Move::getFrom()  
{ 
	return (move & 0x3f);
}  
 
int Move::getTo()  
{  
	return (move >>  6) & 0x3f; 
}   
 
int Move::getPromotion()  
{ 
	return (move >> 12); 
}  

int Move::getScore()
{
	return score;
}

uint16_t Move::getMove()
{
	return move;
}
