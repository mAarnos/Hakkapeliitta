#ifndef MOVE_H
#define MOVE_H

#include "defs.h"

class Move
{
	public:
		void clear();

		void setMove(int move);
		void setFrom(int from);  
		void setTo(int to);  
		void setPiece(int piece);  
		void setCapture(int capture);  
		void setPromotion(int promotion);

		int getMove();
		int getFrom();  
		int getTo();  
		int getPiece();  
		int getCapture();
		int getPromotion();   
	private:
		int moveInt;
		int score;
};



#endif
