#ifndef UCI_CPP
#define UCI_CPP

#include "uci.h"
#include "bitboard.h"
#include "magic.h"
#include "hash.h"
#include "eval.h"
#include "ttable.h"

int uciSendInformation(string s);
int uciSetOption(string s);
int uciIsReady(string s);
int uciNewGame(string s);
int uciPosition(string s);
int uciGo(string s);
int uciStop(string s);
int uciExit(string s);

const int amountOfCommands = 8;
uciCommand commands[amountOfCommands] =
{
	{ "uci", uciSendInformation },
	{ "setoption", uciSetOption },
	{ "isready", uciIsReady },
	{ "ucinewgame", uciNewGame },
	{ "position", uciPosition },
	{ "go", uciGo },
	{ "stop", uciStop },
	{ "quit", uciExit }
};

void initializeEngine()
{
	initializeBitboards();
	initializeMagics();
	initializeHash();
	initializeKnownEndgames();
}

void uciMainLoop()
{
	int status = uciOk;
	string line;
	string command;
	string parameters;

	while (status == uciOk)
	{
		// Read a line from stdin.
		if (!getline(cin, line))
		{
			break;
		}

		// Remove whitespace from the beginning and the end. 
		// We also remove whitespace inside the functions which take in multiple parameters.
		line = line.substr(line.find_first_not_of(' '));
		line = line.substr(0, line.find_last_not_of(' ') + 1);
		// Change all characters to lowercase. 
		transform(line.begin(), line.end(), line.begin(), tolower);

		// The easiest way to parse the string is to use a regular expression.
		// We haven't implemented the protocol properly here, if you give the command joho uci or something like that we will find nothing.
		// TODO: do that properly some day.
		regex expr("\\s*(\\w*)\\s*(.*)");
		smatch matches;
		if (regex_search(line, matches, expr))
		{
			command = matches[1];
			parameters = matches[2];
		}

		// Go through the list of commands and call the correct function if the command entered is available. 
		int i = 0;
		for (i = 0; i < amountOfCommands; i++)
		{
			if (command == commands[i].name)
			{
				status = (commands[i].function)(parameters);
				break;
			}
		}
	}
}

int uciSendInformation(string s)
{
	// Send the name of the engine and the name of it's author.
	cout << "id name Hakkapeliitta v2" << endl;
	cout << "id author Mikko Aarnos" << endl;

	// Send all possible options the engine has that can be modified.
	cout << "option name Hash type spin default 1 min 1 max 65536" << endl;
	cout << "option name Clear Hash type button" << endl;
	cout << "option name Drawscore type spin default 0 min -75 max 75" << endl;

	// Send a response telling the listener that we are ready in UCI-mode.
	cout << "uciok" << endl;

	return uciOk;
}

int uciSetOption(string s)
{
	string option;
	string parameter;

	// The string s for setoption comes in the form "name" option "value" parameter.
	// We just ignore "name" and "value" and get option and parameter.
	regex expr("\\w*\\s*(\\w*)\\s*\\w*\\s*(.*)");
	smatch matches;
	if (regex_search(s, matches, expr))
	{
		option = matches[1];
		parameter = matches[2];
	}

	if (option == "drawscore")
	{
		try
		{
			drawScore = stoi(parameter);
		}
		catch (const exception&)
		{
			drawScore = 0;
		}
	}
	else if (option == "hash")
	{
		uint64_t size;

		try 
		{
			size = stoi(parameter) * 1024 * 1024;
		}
		catch (const exception&)
		{
			size = 0;
		}

		tt.setSize(size);
	}
	else if (option == "clear") // Thanks to our parsing clear hash is shortened to clear. Can't be helped.
	{
		tt.clear();
		ptt.clear();
	}

	return uciOk;
}

int uciIsReady(string s)
{
	initializeEngine();
	cout << "readyok" << endl;
	return uciOk;
}

int uciNewGame(string s)
{
	tt.clear();
	ptt.clear();
	return uciOk;
}

int uciPosition(string s)
{
	return uciOk;
}

int uciGo(string s)
{
	return uciOk;
}

int uciStop(string s)
{
	searching = false;
	return uciOk;
}

int uciExit(string s)
{
	return uciQuit;
}

#endif