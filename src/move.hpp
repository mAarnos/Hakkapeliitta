#ifndef MOVE_H_
#define MOVE_H_

#include "defs.hpp"

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
