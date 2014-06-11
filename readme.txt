Hakkapeliitta is my UCI-compatible chess engine written in C++11. At the moment of writing it is not yet a release version, but this should change soon.

On UCI-parameters:
	1. Drawscore: negative values make the program avoid draws, positive values make it prefer them.
	2. SyzygyPath: path to the Syzygy tablebases. Multiple directories possible, for example C:\tb;D:\tb 
	3. SyzygyProbeLimit: 0, 1 or 2 means tablebases aren't probed, 3 means 3-man tablebases are probed, 4 means 4-man etc.

Compilation instructions:
	General instructions: 
		1. Do NOT compiler tbcore.cpp, it is included by tbprobe.cpp.
		2. This program is written in C++11, so a compiler which supports it is required. The Visual Studio compiler works for sure, haven't tested GCC or Intel. 
		3. Only Windows is supported, this might change someday.
	w32:
		1. Go to bitboard.h, comment out the default bitScanForward, bitScanReverse and popcnt, and comment in the alternatives.
		2. Go to tbprobe.cpp and undefine IS_64BIT.
		3. Compile.
	x64:
		1. Go to bitboard.h, comment out the default popcnt and comment in the alternative.
		2. Compile.
	x64 popcnt:
		1. Compile.
		
Known bugs:
 -A very small hash table size(<=8MB) starts causing crashes. These crashes get more frequent the longer the time control is. I have a solution for this(probably) but it isn't high on my list of priorities since who even uses less than 64MB hash nowadays?

Thanks to the following people(or organizations) my engine is what it is today:

 Chess Programming Wiki
 Talkchess
 Glaurung, Tord Romstad
 Ronald de Man(Syzygy tablebases and their probing code, fixed shift magic code)
 Stockfish,  Tord Romstad, Marco Costalba, and Joona Kiiski
 Inside Prodeo, Ed Schroeder
 Winglet(time management, a bit of search, move structure)
 Maverick(Internal Iterative Deepening), Steve Maughan
 Peter Osterlund(basic tuning method although significant changes were required)
 TSCP(basic move generation, not used for a long time), Tom Kerrigan