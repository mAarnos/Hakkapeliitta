Hakkapeliitta is my UCI-compatible chess engine. It works perfectly well on my machine(i3-380M). If compiled on anything else I cannot give any guarantees.

Known bugs:
 -Setting the hash size to zero and running the engine crashes it.
  How to fix: add detection for this case
 -The size of the pawn hash table is independent of the UCI-parameter Hash.       That is, if you specify Hash=32 what you get is 32 for the transposition        table and some for the pawn hash table, and not 32 in total.
 -UCI-parameter drawscore might not work, haven't tested.

Thanks to the following people(or organizations) my engine is what it is today:

 Chess Programming Wiki
 Talkchess
 Ed Schroeder(Inside ProDeo)
 Ronald de Man(Syzygy tablebases and their probing code)
 Winglet(time management, a bit of search)
 Maverick(Internal Iterative Deepening)
 TSCP(basic move generation, not used for a long time)


