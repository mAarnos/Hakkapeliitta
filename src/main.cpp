#ifndef MAIN_CPP
#define MAIN_CPP

#include "defs.h"
#include "bitboard.h"
#include "magic.h"
#include "hash.h"
#include "ttable.h"
#include "position.h"

int main()
{
	cout << "Hakkapeliitta v2, (C) 2013-2014 Mikko Aarnos" << endl;

	initializeBitboards();
	initializeMagics();
	initializeHash();

	Position pos;
	pos.initializeBoardFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

	cout << "done" << endl;

	return 0; 
}

#endif