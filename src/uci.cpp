#ifndef UCI_CPP
#define UCI_CPP

#include <windows.h>
#include "uci.h"
#include "search.h"
#include "eval.h"
#include "hash.h"
#include "time.h"
#include "ttable.h"
#include "tbprobe.h"
#include "tbcore.h"
#include <time.h>

bool Init = false;
bool Searching = false;
bool Infinite = false;

bool ttInit = false;
int syzygyProbeLimit = 0;
bool dtzPathGiven = false;

int errors;
int successes;
	
int pipe;
HANDLE hstdin;

void listenForInput();
int Send(char * command);

void initInput()
{
	unsigned long dw;

	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	pipe = !GetConsoleMode(hstdin, &dw);
	
	if (!pipe) 
	{
        SetConsoleMode(hstdin,dw&~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(hstdin);
    } 
	else 
	{
        setvbuf(stdin,NULL,_IONBF,0);
        setvbuf(stdout,NULL,_IONBF,0);
    }
}

void init()
{
	if (!Init)
	{
		Init = true;
		initializeBitboards();
		initializeMagics();
		initializeHash();
		initializeBoard();
		initializeCastlingMoveInts();
		initializeKingTropism();
		if (!ttInit)
		{
			ttSetSize(1024 * 1024);
			pttSetSize((1024 * 1024) / 8);
		}
		init_tablebases();
	}
}

bool checkInput()
{
	unsigned long dw = 0;

	if (stdin->_cnt > 0) return true;
	if (pipe) 
	{
        if (!PeekNamedPipe(hstdin, 0, 0, 0, &dw, 0)) return true;
        return dw > 0;
    } 
	else 
	{
        GetNumberOfConsoleInputEvents(hstdin, &dw);
        return dw > 1;
    }
}

void listenForInput()
{
	char command[2048];

	gets(command);
	
    if (!strcmp(command, "uci")) 
	{
 
        cout << "id name Hakkapeliitta dev 65.2" << endl;
        cout << "id author Mikko Aarnos" << endl;
 
        // send options
		cout << "option name Hash type spin default 1 min 1 max 2048" << endl;
		cout << "option name Clear Hash type button" << endl;
		cout << "option name Drawscore type spin default 0 min -75 max 75" << endl;
		cout << "option name SyzygyProbeLimit type spin default 0 min 0 max 6" << endl;
		cout << "option name SyzygyPathWDL type string default c:\\wdl\\" << endl;
		cout << "option name SyzygyPathDTZ type string default c:\\dtz\\" << endl;

        cout << "uciok" << endl;
    }

	else if (!strncmp(command, "setoption", 9)) {
		char name[256];
        char value[256];

		sscanf(command, "setoption name %s value %s", &name, &value);
		if (!strncmp(name, "Hash", 4))
		{
			int val;
			sscanf(value, "%d", &val);
			ttSetSize(1024 * 1024 * val);
			pttSetSize(1024 * 1024 * (val / 8));
			ttInit = true;
		}
		else if (!strncmp(name, "Clear", 5)) 
		{
			memset(tt, 0, (ttSize + 1) * sizeof(ttEntry));
			memset(ptt, 0, (pttSize + 1) * sizeof(pttEntry));
		}
		else if (!strncmp(name, "Drawscore", 9))
		{
			int val;
			sscanf(value, "%d", &val);
			drawscore = val;
		}	
		else if (!strncmp(name, "SyzygyProbeLimit", 16)) 
		{
			int val;
			sscanf(value, "%d", &val);
			syzygyProbeLimit = val;
		}
		else if (!strncmp(name, "SyzygyPathWDL", 14)) 
		{
			int i;
			for (i = 0; i < 128; i++)
			{
				WDLdir[i] = value[i];
			}
		}
		else if (!strncmp(name, "SyzygyPathDTZ", 14)) 
		{
			int i;
			for (i = 0; i < 128; i++)
			{
				TBdir[i] = value[i];
			}
			dtzPathGiven = true;
		}
	}
	
	else if (!strcmp(command, "isready"))
	{
		init();
		Searching = false;
        cout << "readyok" << endl;
	}
	
	else if (!strcmp(command, "ucinewgame")) 
	{
		Searching = false;
	}
	
	else if (!strncmp(command, "position", 8)) {
        //position [fen | startpos] [moves ...]

		Searching = false;
 
        if (!strncmp(command,"position fen",12)) 
		{
            initializeBoardFromFen(command + 13);
        } 
		else 
		{
            initializeBoardFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
 
        char * moves = strstr(command, "moves");
        if (moves) 
		{ 
			algebraicMoves(moves+6);
		}
    }
	
	else if (!strncmp(command, "go", 2))
	{
        go(command);
	}

	else if (!strncmp(command, "evaluate", 8))
	{
		// not part of UCI-protocol
        cout << "score "<< eval() << endl;
	}
	
	else if (!strncmp(command, "debug", 5)) {}

	else if (!strcmp(command, "stop"))
	{
        Searching = false;
		Infinite = false;
	}
	
	else if (!strcmp(command, "quit"))
	{
        exit(0);
	}
}

#endif