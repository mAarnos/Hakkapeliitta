FILES = main.cpp benchmark.cpp bitboards.cpp counter.cpp evaluation.cpp history.cpp killer.cpp movegen.cpp movesort.cpp pht.cpp position.cpp search.cpp tt.cpp uci.cpp zobrist.cpp syzygy/tbprobe.cpp
FLAGS = -pthread -std=c++11 -Ofast -Wall -flto -march=native -s -DNDEBUG

make: $(FILES)
	g++ $(FLAGS) $(FILES) -o Hakkapeliitta

