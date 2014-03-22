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

		int getFrom();  
		int getTo();  
		int getPromotion();   
	private:
		int moveInt;
		int score;
};



#endif
