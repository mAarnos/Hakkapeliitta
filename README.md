### Overview

Hakkapeliitta is an UCI chess engine written in C++11/14 with support for Syzygy tablebases. Version 3.0 has a rating of around 2930 at CCRL and 2800 at CEGT. 

### UCI-parameters:

 - Hash: This option should be set to the amount of memory the main transposition table can use (in MB).
 - Pawn Hash: This option should be set to the amount of memory the pawn hash table can use (in MB).
 - Clear Hash: Using this option clears the transposition table and the pawn hash table.
 - Contempt: Positive values of this option make Hakkapeliitta avoid draws, negative values make it prefer them. Larger values have a bigger effect.
 - Ponder: This option is used for enabling/disabling pondering.
 - SyzygyPath: This option should be set to the directory or directories that contain the .rtbw and .rtbz files. Multiple directories should be separated by ";" on Windows and by ":" on Unix-based operating systems. Do not use spaces around the ";" or ":".
 - SyzygyProbeDepth: Increasing this option lets the engine probe less aggressively. Set this option to a higher value if you experience too much slowdown (in terms of NPS) due to TB probing.
 - SyzygyProbeLimit: Only probe TB files which have a piece count less than or equal to this option. This option should normally be left at its default value.
 - Syzygy50MoveRule: Set this option to false if you want TB positions that are drawn by the 50-move rule to count as wins or losses. This may be useful for correspondence games. 

### Compiling it yourself

A makefile is provided for this purpose. 
Binaries produced by this makefile will most likely only work on the machine it was compiled on, so Hakkapeliitta should compiled individually for every machine it is needed on.

### Acknowledgements	

Thanks to the following people (or organizations) my engine is what it is today.
If you think I have forgotten somebody/something please let me know.

 - Chess Programming Wiki
 - Talkchess
 - Glaurung, Tord Romstad
 - Ronald de Man 
 - Stockfish, Tord Romstad, Marco Costalba, and Joona Kiiski
 - Crafty, Robert Hyatt
 - Texel, Peter Österlund
 - Ivanhoe, ??? 
 - Inside Prodeo, Ed Schroeder
 - Winglet, Stef Luijten
 - Maverick, Steve Maughan
 - TSCP, Tom Kerrigan 

 