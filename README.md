### Overview

Hakkapeliitta is my UCI-compabtile chess engine written in C++11/14. It's rating
is around 2775 elo and has a highly tactical and entertaining playing style. It 
also supports Syzygy tablebases.

### UCI-parameters:

 Hash: set the amount of memory the main transposition table can use (in MB).
 PawnHash: set the amount of memory the pawn hash table can use (in MB).
 Contempt: positive values make Hakkapeliitta avoid draws, negative values make it prefer them.
 SyzygyProbeLimit: 0, 1 or 2 means tablebases aren't probed, 3 means 3-man tablebases are probed, 4 means 4-man etc. On a 32-bit machine syzygyProbeLimit can only be set at maximum to 5.
 SyzygyPath: set the path to the Syzygy tablebases. Multiple directories are possible if they are separated by ";" on Windows and ":" on Linux. Do not use spaces around the ";" or ":".

### Compiling it yourself

Hakkapeliitta can be compiled from the source code. No makefiles are provided 
as I am unfamiliar with them. Supported platforms are Windows and Linux.
Support for Macintosh isn't planned due to me not having a computer with 
Macintosh. Supported compilers are MSVC 12.0 (or higher), GCC 4.9.0 (or higher) 
and Clang 3.4 (or higher). 

### Acknowledgements	

Thanks to the following people(or organizations) my engine is what it is today:

 Chess Programming Wiki
 Talkchess
 Glaurung, Tord Romstad
 Ronald de Man 
 Stockfish, Tord Romstad, Marco Costalba, and Joona Kiiski
 Crafty, Robert Hyatt
 Texel, Peter Österlund
 Ivanhoe, ??? 
 Inside Prodeo, Ed Schroeder
 Winglet, ??? 
 Maverick, Steve Maughan
 TSCP, Tom Kerrigan 

 