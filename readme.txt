Hakkapeliitta is my UCI-compatible chess engine written in C++11. At the moment of writing it is not yet a release version, but this should change soon.

On compiling:
 -This program is written in C++11, so you have to have a compiler which supports that. The Visual Studio compiler works for sure, haven't tested GCC or Intel. 
 -You MUST NOT compile tbcore.cpp. It is included by tbprobe.cpp already.
 -bitboard.h includes software bitscanforward and popcnt alternatives when compiling for a 32-bit machine or a machine with no hardware popcnt.
 -If compiling for a 32-bit platform you must also undefine IS_64BIT found in tbprobe.cpp
 -Linux is not supported yet, but I plan to add support to it right after I figure out how to check if the user has inputted data.

Known bugs:
 -A very small hash table size(<=8MB) starts causing crashes. These crashes get more frequent the longer the time control is. I have a solution for this(probably) but it isn't high on my list of priorities since who even uses less than 64MB hash nowadays?
 -UCI-parameter drawscore might not work, haven't tested. Not high on my list of priorities since Hakkapeliitta is pretty weak so drawscore isn't that useful.
 -The size of the pawn hash table is independent of the UCI-parameter Hash. That is, if you specify Hash=32 what you get is 32 for the transposition table and some for the pawn hash table, and not 32 in total.

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