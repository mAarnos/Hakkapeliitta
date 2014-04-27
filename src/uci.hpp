#ifndef UCI_H_
#define UCI_H_

#include "defs.hpp"

typedef int(*uciFunctionPointer)(string s);

class uciCommand
{
	public:
		string name;
		uciFunctionPointer function;
};

bool inputAvailable();

void uciMainLoop();
int uciProcessInput();
void initInput();

const int uciQuit = 0;
const int uciOk = 1;

extern bool searching;

#endif
