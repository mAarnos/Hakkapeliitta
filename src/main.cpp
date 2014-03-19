#ifndef MAIN_CPP
#define MAIN_CPP

#include <iostream>
#include <time.h>
#include "search.h"
#include "eval.h"
#include "hash.h"
#include "uci.h"

int main()
{
	Init = false;
	Searching = false;
	cout << "Hakkapeliitta dev 67, (C) 2013-2014 Mikko Aarnos" << endl;
	initInput();

	while(true)
	{ 
		listenForInput();
	}
	return 0; 
}

#endif