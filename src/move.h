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
		void setCastling(bool castling);
		void setEnPassant(bool ep);

		void setScore(int score);
		void setMove(int m);
	
		int getFrom();  
		int getTo();  
		int getPromotion(); 
		bool getCastling();
		bool getEnPassant();

		int getScore();
		int getMove();
	private:
		uint32_t move;
		int score;
};



#endif
