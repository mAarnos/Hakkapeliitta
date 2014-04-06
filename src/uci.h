#ifndef UCI_H
#define UCI_H

#include "defs.h"

typedef int(*uciFunctionPointer)(string s);

class uciCommand
{
	public:
		string name;
		uciFunctionPointer function;
};

void uciMainLoop();
int uciProcessInput();

const int uciQuit = 0;
const int uciOk = 1;

extern bool searching;

#endif
