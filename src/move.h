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
		void setMove(uint16_t m);
	
		int getFrom();  
		int getTo();  
		int getPromotion(); 

		int getScore();
		uint16_t getMove();
	private:
		uint16_t move;
		int score;
};



#endif
