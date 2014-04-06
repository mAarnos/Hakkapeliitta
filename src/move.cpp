#ifndef MOVE_CPP
#define MOVE_CPP

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

void Move::setCastling(bool castling)
{
	move &= 0x00000000FFFEFFFF;
	move |= (int)castling << 16;
}

void Move::setEnPassant(bool ep)
{
	move &= 0x00000000FFFDFFFF;
	move |= (int)ep << 17;
}

void Move::setScore(int s)
{
	score = s;
}

void Move::setMove(int m)
{
	move = m;
}

int Move::getFrom()  
{ 
	return (move & 0x0000003f);
}  
 
int Move::getTo()  
{  
	return (move >>  6) & 0x0000003f; 
}   
 
int Move::getPromotion()  
{ 
	return (move >> 12) & 0x0000000f; 
}  

bool Move::getCastling()
{
	return (move >> 16) & 0x00000001;
}

bool Move::getEnPassant()
{
	return (move >> 17) & 0x00000001;
}

int Move::getScore()
{
	return score;
}

int Move::getMove()
{
	return move;
}
 
#endif