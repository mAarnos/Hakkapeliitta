#include <windows.h>
#include "uci.hpp"
#include "bitboard.hpp"
#include "magic.hpp"
#include "hash.hpp"
#include "eval.hpp"
#include "ttable.hpp"
#include "search.hpp"
#include "time.hpp"
#include "tbcore.hpp"

int uciProcessInput();
int uciSendInformation(string s);
int uciSetOption(string s);
int uciIsReady(string s);
int uciNewGame(string s);
int uciPosition(string s);
int uciGo(string s);
int uciStop(string s);
int uciExit(string s);
int uciDisplayBoard(string s);
int uciStaticEval(string s);

bool searching; 

int pipe;
HANDLE hstdin;

const int amountOfCommands = 10;
uciCommand commands[amountOfCommands] =
{
	{ "uci", uciSendInformation },
	{ "setoption", uciSetOption },
	{ "isready", uciIsReady },
	{ "ucinewgame", uciNewGame },
	{ "position", uciPosition },
	{ "go", uciGo },
	{ "stop", uciStop },
	{ "quit", uciExit },
	{ "displayboard", uciDisplayBoard },
	{ "staticeval", uciStaticEval },
};

void initInput()
{
	unsigned long dw;

	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	pipe = !GetConsoleMode(hstdin, &dw);

	if (!pipe)
	{
		SetConsoleMode(hstdin, dw&~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
		FlushConsoleInputBuffer(hstdin);
	}
	else
	{
		setvbuf(stdin, NULL, _IONBF, 0);
		setvbuf(stdout, NULL, _IONBF, 0);
	}
}

bool inputAvailable()
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

void uciMainLoop()
{
	int status = uciOk;
	// The loop here is pretty much redundant at the moment as we never change uciOk to anything else.
	// Maybe if something goes badly wrong?
	while (status == uciOk)
	{
		status = uciProcessInput();
	}
}

int uciProcessInput()
{
	int status = uciOk;
	string line, command, parameters;

	// Read a line from stdin.
	if (!getline(cin, line))
	{
		return uciQuit;
	}

	// The command is only whitespace, move on.
	if (line.find_first_not_of(' ') == string::npos)
	{
		return uciOk;
	}

	// Remove all extra whitespace from the string.
	line = line.substr(line.find_first_not_of(' '));
	line = line.substr(0, line.find_last_not_of(' ') + 1);
	while (line.find("  ") != string::npos)
	{
		line.replace(line.find("  "), 2, " ");
	}

	// We haven't implemented the protocol properly here, if you give the command joho uci or something like that we will do nothing.
	// TODO: do that properly some day.
	regex expr("(\\w*)\\s?(.*)");
	smatch matches;
	if (regex_search(line, matches, expr))
	{
		command = matches[1];
		parameters = matches[2];
	}

	// Go through the list of commands and call the correct function if the command entered is available. 
	for (int i = 0; i < amountOfCommands; i++)
	{
		if (command == commands[i].name)
		{
			status = (commands[i].function)(parameters);
			break;
		}
	}

	return status;
}

int uciSendInformation(string s)
{
	// Send the name of the engine and the name of it's author.
	cout << "id name Hakkapeliitta v2.19" << endl;
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
	string option, parameter;

	// The string s for setoption comes in the form "name" option "value" parameter.
	// We just ignore "name" and "value" and get option and parameter.
	regex expr("\\w*\\s(\\w*)\\s\\w*\\s*(.*)");
	smatch matches;
	if (regex_search(s, matches, expr))
	{
		option = matches[1];
		parameter = matches[2];
	}

	if (option == "Drawscore")
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
	else if (option == "Hash")
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

		if (size <= 1024 * 1024)
		{
			size = 1024 * 1024;
		}

		tt.setSize(size);
		ptt.setSize(size / 8);
	}
	else if (option == "Clear") // Thanks to our parsing clear hash is shortened to clear. Can't be helped.
	{
		tt.clear();
		ptt.clear();
	}
	else if (option == "SyzygyProbeLimit")
	{
		try
		{
			syzygyProbeLimit = stoi(parameter);
		}
		catch (const exception&)
		{
			syzygyProbeLimit = 0;
		}
	}
	else if (option == "SyzygyPath")
	{
		init(parameter);
	}

	return uciOk;
}

int uciIsReady(string s)
{
	cout << "readyok" << endl;
	return uciOk;
}

int uciNewGame(string s)
{
	tt.clear();
	ptt.clear();
	cout << "info string hash cleared" << endl;
	return uciOk;
}

int uciPosition(string s)
{
	string moves;
	vector<string> move;

	size_t pos = s.find("moves");
	if (pos != string::npos)
	{
		moves = s.substr(pos + 6);
		s = s.substr(0, pos - 1);
	}

	pos = s.find("fen");
	// Could also be s.find("fen") != string::npos but in that case fen could be ANYWHERE within the string and that is not what we want.
	// I really don't know whether I should care about stuff like that.
	if (pos == 0)
	{
		root.initializeBoardFromFEN(s.substr(4));
	}
	else
	{
		root.initializeBoardFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	}

	regex expr("([a-z][0-9][a-z][0-9][a-z]?)");
	for (sregex_iterator it(moves.begin(), moves.end(), expr), end; it != end; ++it)
	{
		move.push_back((*it)[0]);
	}

	for (int i = 0; i < move.size(); i++)
	{
		Move m;
		m.clear();
		m.setPromotion(Empty);

		int from = (move[i][0] - 'a') + 8 * (move[i][1] - '1');
		int to = (move[i][2] - 'a') + 8 * (move[i][3] - '1');

		m.setFrom(from);
		m.setTo(to);

		if (move[i].size() == 5)
		{
			switch (move[i][4])
			{
				case 'q': m.setPromotion(Queen); break;
				case 'r': m.setPromotion(Rook); break;
				case 'b': m.setPromotion(Bishop); break;
				case 'n': m.setPromotion(Knight); break;
				default: return uciOk;
			}
		}
		else if ((root.getPieceType(from) == King) && (abs(from - to) == 2))
		{
			m.setPromotion(King);
		}
		else if ((root.getPieceType(from) == Pawn) && to == root.getEnPassantSquare())
		{
			m.setPromotion(Pawn);
		}

		root.makeMove(m);
	}

	return uciOk;
}

int uciGo(string s)
{
	allocateSearchTime(s);
	think();
	return uciOk;
}

int uciStop(string s)
{
	searching = false;
	return uciOk;
}

int uciExit(string s)
{
	exit(0);
}

int uciDisplayBoard(string s)
{
	root.displayBoard();
	return uciOk;
}

int uciStaticEval(string s)
{
	cout << "Static Evaluation = " << eval(root) << endl;
	return uciOk;
}