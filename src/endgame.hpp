#ifndef ENDGAME_HPP_
#define ENDGAME_HPP_

#include <unordered_set>
#include "zobrist.hpp"
#include "color.hpp"
#include "piece.hpp"

class EndgameModule 
{
public:
    EndgameModule();

    bool drawnEndgame(HashKey materialHashKey) const { return drawnEndgames.count(materialHashKey) > 0; }
private:
    std::unordered_set<HashKey> drawnEndgames;
};

inline EndgameModule::EndgameModule()
{
    // KK
    // FIDE draw
    const auto matHash = Zobrist::materialHashKey(Piece::WhiteKing, 0) ^ Zobrist::materialHashKey(Piece::BlackKing, 0);
    drawnEndgames.insert(matHash);

    // KBK, KNK, KKB, KKN
    // FIDE draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            drawnEndgames.insert(matHash ^ Zobrist::materialHashKey(j + i * 6, 0));
        }
    }

    // KNNK, KKNN
    // FIDE draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        drawnEndgames.insert(matHash ^ Zobrist::materialHashKey(Piece::Knight + i * 6, 0) ^
            Zobrist::materialHashKey(Piece::Knight + i * 6, 1));
    }

    // KBKB, KBKN, KNKB, KNKN
    for (Piece i = Piece::Knight; i <= Piece::Bishop; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            drawnEndgames.insert(matHash ^ Zobrist::materialHashKey(Color::White + i, 0) ^
                Zobrist::materialHashKey(Color::Black * 6 + j, 0));
        }
    }

    // KBBKB, KBKBB
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        drawnEndgames.insert(matHash ^ Zobrist::materialHashKey(Piece::Bishop + i * 6, 0) ^
            Zobrist::materialHashKey(Piece::Bishop + i * 6, 1) ^
            Zobrist::materialHashKey(Piece::Bishop + !i * 6, 0));
    }

    // KBNKB, KBNKN, KNNKB, KNNKN, KBKBN, KNKBN, KBKNN, KNKNN
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            for (Piece k = Piece::Knight; k <= Piece::Bishop; ++k)
            {
                drawnEndgames.insert(matHash ^ Zobrist::materialHashKey(Piece::Knight + i * 6, 0) ^
                    Zobrist::materialHashKey(j + i * 6, j == Piece::Knight) ^
                    Zobrist::materialHashKey(k + !i * 6, 0));
            }
        }
    }
}

#endif