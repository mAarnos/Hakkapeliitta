#ifndef MOVE_H
#define MOVE_H

#include "defs.h"

class Move
{
	public:
		void clear();
		void setFrom(int from);  
		void setTo(int to);  
		void setPromotion(int promotion);
		void setScore(int score);
		void setMove(int m);
		int getMove();
		int getFrom();  
		int getTo();  
		int getPromotion();   
		int getScore();
	private:
		uint32_t move;
		int score;
};



#endif
