#include "eval.hpp"
#include "piece.hpp"
#include "color.hpp"
#include "square.hpp"

PawnHashTable Evaluation::pawnHashTable;

std::array<int, 64> Evaluation::pieceSquareTableOpening[12];
std::array<int, 64> Evaluation::pieceSquareTableEnding[12];

std::array<int, 6> Evaluation::pieceValuesOpening = {
    88, 235, 263, 402, 892, 0
};

std::array<int, 6> Evaluation::pieceValuesEnding = {
    112, 258, 286, 481, 892, 0
};

std::array<int, 64> Evaluation::openingPST[6] = {
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		-33,-18,-13,-18,-18,-13,-18,-33,
		-28,-23,-13, -3, -3,-13,-23,-28,
		-33,-18,-13, 12, 12,-13,-18,-33,
		-18,-13, -8, 17, 17, -8,-13,-18,
		7, 12, 17, 22, 22, 17, 12, 7,
		42, 42, 42, 42, 42, 42, 42, 42,
		0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		-36, -26, -16, -6, -6, -16, -26, -36,
		-26, -16, -6, 9, 9, -6, -16, -26,
		-16, -6, 9, 29, 29, 9, -6, -16,
		-6, 9, 29, 39, 39, 29, 9, -6,
		-6, 9, 39, 49, 49, 39, 9, -6,
		-16, -6, 19, 59, 59, 19, -6, -16,
		-26, -16, -6, 9, 9, -6, -16, -26,
		-36, -26, -16, -6, -6, -16, -26, -36
	},
	{
		-24, -19, -14, -9, -9, -14, -19, -24,
		-9, 6, 1, 6, 6, 1, 6, -9,
		-4, 1, 6, 11, 11, 6, 1, -4,
		1, 6, 11, 16, 16, 11, 6, 1,
		1, 11, 11, 16, 16, 11, 11, 1,
		-4, 1, 6, 11, 11, 6, 1, -4,
		-9, -4, 1, 6, 6, 1, -4, -9,
		-14, -9, -4, 1, 1, -5, -9, -14
	},
	{
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		-3, -3, -1, 2, 2, -1, -3, -3,
		7, 7, 9, 12, 12, 9, 7, 7,
		-3, -3, -1, 2, 2, -1, -3, -3,
	},
	{
		-19, -14, -9, -4, -4, -9, -14, -19,
		-9, -4, 1, 6, 6, 1, -4, -9,
		-4, 1, 6, 11, 11, 6, 1, -4,
		1, 6, 11, 16, 16, 11, 6, 1,
		1, 6, 11, 16, 16, 11, 6, 1,
		-4, 1, 6, 11, 11, 6, 1, -4,
		-9, -4, 1, 6, 6, 1, -4, -9,
		-14, -9, -4, 1, 1, -4, -9, -14
	},
	{
		5, 10, 2, 0, 0, 6, 10, 4,
		5, 5, 0, -5, -5, 0, 5, 5,
		-5, -5, -5, -10, -10, -5, -5, -5,
		-10, -10, -20, -30, -30, -20, -10, -10,
		-20, -25, -30, -40, -40, -30, -25, -20,
		-40, -40, -50, -60, -60, -50, -40, -40,
		-50, -50, -60, -60, -60, -60, -50, -50,
		-60, -60, -60, -60, -60, -60, -60, -60
	}
};

std::array<int, 64> Evaluation::endingPST[6] = {
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		-22, -22, -22, -22, -22, -22, -22, -22,
		-17, -17, -17, -17, -17, -17, -17, -17,
		-12, -12, -12, -12, -12, -12, -12, -12,
		-7, -7, -7, -7, -7, -7, -7, -7,
		18, 18, 18, 18, 18, 18, 18, 18,
		38, 38, 38, 38, 38, 38, 38, 38,
		0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		-34, -24, -14, -4, -4, -14, -24, -34,
		-24, -14, -4, 11, 11, -4, -14, -24,
		-14, -4, 11, 31, 31, 11, -4, -14,
		-4, 11, 31, 41, 41, 31, 11, -4,
		-4, 11, 31, 41, 41, 31, 11, -4,
		-14, -4, 11, 31, 31, 11, -4, -14,
		-24, -14, -4, 11, 11, -4, -14, -24,
		-34, -24, -14, -4, -4, -14, -24, -34
	},
	{
		-15, -10, -5, 0, 0, -5, -10, -15,
		-10, -5, 0, 5, 5, 0, -5, -10,
		-5, 0, 5, 10, 10, 5, 0, -5,
		0, 5, 10, 15, 15, 10, 5, 0,
		0, 5, 10, 15, 15, 10, 5, 0,
		-5, 0, 5, 10, 10, 5, 0, -5,
		-10, -5, 0, 5, 5, 0, -5, -10,
		-15, -10, -5, 0, 0, -5, -10, -15
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	},
	{
		-15, -10, -5, 0, 0, -5, -10, -15,
		-10, -5, 0, 5, 5, 0, -5, -10,
		-5, 0, 5, 10, 10, 5, 0, -5,
		0, 5, 10, 15, 15, 10, 5, 0,
		0, 5, 10, 15, 15, 10, 5, 0,
		-5, 0, 5, 10, 10, 5, 0, -5,
		-10, -5, 0, 5, 5, 0, -5, -10,
		-15, -10, -5, 0, 0, -5, -10, -15
	},
	{
		-38, -28, -18, -8, -8, -18, -28, -38,
		-28, -18, -8, 13, 13, -8, -18, -28,
		-18, -8, 13, 43, 43, 13, -8, -18,
		-8, 13, 43, 53, 53, 43, 13, -8,
		-8, 13, 43, 53, 53, 43, 13, -8,
		-18, -8, 13, 43, 43, 13, -8, -18,
		-28, -18, -8, 13, 13, -8, -18, -28,
		-38, -28, -18, -8, -8, -18, -28, -38
	}
};

std::vector<int> Evaluation::mobilityOpening[6] = {
    {},
    { -12, -8, -4, 0, 4, 8, 10, 12, 13 },
    { -12, -6, 0, 6, 12, 18, 22, 26, 28, 30, 31, 31, 32, 32 },
    { -6, -4, -2, 0, 2, 4, 6, 8, 10, 11, 11, 11, 11, 12, 12 },
    { -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 },
    {}
};

std::vector<int> Evaluation::mobilityEnding[6] = {
    {},
    { -12, -8, -4, 0, 4, 8, 10, 12, 13 },
    { -12, -6, 0, 6, 12, 18, 22, 26, 28, 30, 31, 31, 32, 32 },
    { -12, -7, -3, 3, 7, 12, 16, 20, 24, 28, 30, 31, 31, 32, 32 },
    { -8, -6, -4, -2, 0, 2, 4, 6, 8, 9, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 },
    {}
};

const std::array<int, 64> Evaluation::flip = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
    8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 2, 3, 4, 5, 6, 7
};

std::unordered_map<HashKey, int> Evaluation::knownEndgames;

void Evaluation::initializeKnownEndgames()
{
    knownEndgames.clear();

    // King vs king: draw
    auto matHash = Zobrist::materialHash[Piece::WhiteKing][0] ^ Zobrist::materialHash[Piece::BlackKing][0];
    knownEndgames[matHash] = 0;

    // King and a minor piece vs king: draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            knownEndgames[matHash ^ Zobrist::materialHash[j + i * 6][0]] = 0;
        }
    }

    // King and two knights vs king: draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        knownEndgames[matHash ^ Zobrist::materialHash[Piece::Knight + i * 6][0] ^ 
                      Zobrist::materialHash[Piece::Knight + i * 6][1]] = 0;
    }

    // King and a minor piece vs king and a minor piece: draw
    for (Piece i = Piece::Knight; i <= Piece::Bishop; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            knownEndgames[matHash ^ Zobrist::materialHash[Color::White + i][0] ^ 
                          Zobrist::materialHash[Color::Black * 6 + j][0]] = 0;
        }
    }

    // King and two bishops vs king and a bishop: draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        knownEndgames[matHash ^ Zobrist::materialHash[Piece::Bishop + i * 6][0] ^ 
                      Zobrist::materialHash[Piece::Bishop + i * 6][1] ^ 
                      Zobrist::materialHash[Piece::Bishop + !i * 6][0]] = 0;
    }

    // King and either two knights or a knight and a bishop vs king and a minor piece: draw
    for (Color i = Color::White; i <= Color::Black; ++i)
    {
        for (Piece j = Piece::Knight; j <= Piece::Bishop; ++j)
        {
            for (Piece k = Piece::Knight; k <= Piece::Bishop; ++k)
            {
                knownEndgames[matHash ^ Zobrist::materialHash[Piece::Knight + i * 6][0] ^ 
                              Zobrist::materialHash[j + i * 6][j == Piece::Knight] ^ 
                              Zobrist::materialHash[k + !i * 6][0]] = 0;
            }
        }
    }
}

void Evaluation::initialize()
{ 
    initializeKnownEndgames();

    for (Piece p = Piece::Pawn; p <= Piece::King; ++p)
    {
        for (Square sq = Square::A1; sq <= Square::H8; ++sq)
        {
            pieceSquareTableOpening[p][sq] = openingPST[p][sq] + pieceValuesOpening[p];
            pieceSquareTableEnding[p][sq] = endingPST[p][sq] + pieceValuesEnding[p];

            pieceSquareTableOpening[p + Color::Black * 6][sq] = -(openingPST[p][flip[sq]] + pieceValuesOpening[p]);
            pieceSquareTableEnding[p + Color::Black * 6][sq] = -(endingPST[p][flip[sq]] + pieceValuesEnding[p]);
        }
    }
}

int Evaluation::evaluate(const Position & pos)
{
    return (Bitboards::isHardwarePopcntSupported() ? evaluate<true>(pos) : evaluate<false>(pos));
}

template <bool hardwarePopcntEnabled> int Evaluation::evaluate(const Position & pos)
{
    // Checks if we are in a known endgame.
    // If we are we can straight away return the score for the endgame.
    // At the moment only detects draws, if wins will be included this must be made to return things in negamax fashion.
    if (knownEndgames.count(pos.getMaterialHashKey()))
    {
        return knownEndgames[pos.getMaterialHashKey()];
    }

    auto phase = pos.calculateGamePhase();
    auto scoreOp = 0, scoreEd = 0;
    auto score = mobilityEval<hardwarePopcntEnabled>(pos, phase);

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (pos.getBoard(sq) != Piece::Empty)
        {
            scoreOp += pieceSquareTableOpening[pos.getBoard(sq)][sq];
            scoreEd += pieceSquareTableOpening[pos.getBoard(sq)][sq];
        }
    }

    score += ((scoreOp * (256 - phase)) + (scoreEd * phase)) / 256;

    return (pos.getSideToMove() ? -score : score);
}

template int Evaluation::evaluate<false>(const Position & pos);
template int Evaluation::evaluate<true>(const Position & pos);

template <bool hardwarePopcntEnabled> int Evaluation::mobilityEval(const Position & pos, int phase)
{
    auto scoreOp = 0, scoreEd = 0;
    auto occupied = pos.getOccupiedSquares();

    for (Color c = Color::White; c <= Color::Black; ++c)
    {
        auto targetBitboard = ~pos.getPieces(c);
        auto scoreOpForColor = 0, scoreEdForColor = 0;

        auto tempPiece = pos.getBitboard(c, Piece::Knight);
        while (tempPiece)
        {
            auto from = Bitboards::lsb(tempPiece);
            tempPiece &= (tempPiece - 1);
            auto tempMove = Bitboards::knightAttacks[from] & targetBitboard;
            auto count = (hardwarePopcntEnabled ? Bitboards::hardwarePopcnt(tempMove) : Bitboards::softwarePopcnt(tempMove));
            scoreOpForColor += mobilityOpening[Piece::Knight][count];
            scoreEdForColor += mobilityEnding[Piece::Knight][count];
        }

        tempPiece = pos.getBitboard(c, Piece::Bishop);
        while (tempPiece)
        {
            auto from = Bitboards::lsb(tempPiece);
            tempPiece &= (tempPiece - 1);
            auto tempMove = Bitboards::bishopAttacks(from, occupied) & targetBitboard;
            auto count = (hardwarePopcntEnabled ? Bitboards::hardwarePopcnt(tempMove) : Bitboards::softwarePopcnt(tempMove));
            scoreOpForColor += mobilityOpening[Piece::Bishop][count];
            scoreEdForColor += mobilityEnding[Piece::Bishop][count];
        }

        tempPiece = pos.getBitboard(c, Piece::Rook);
        while (tempPiece)
        {
            auto from = Bitboards::lsb(tempPiece);
            tempPiece &= (tempPiece - 1);
            auto tempMove = Bitboards::rookAttacks(from, occupied) & targetBitboard;
            auto count = (hardwarePopcntEnabled ? Bitboards::hardwarePopcnt(tempMove) : Bitboards::softwarePopcnt(tempMove));
            scoreOpForColor += mobilityOpening[Piece::Rook][count];
            scoreEdForColor += mobilityEnding[Piece::Rook][count];
        }

        tempPiece = pos.getBitboard(c, Piece::Queen);
        while (tempPiece)
        {
            auto from = Bitboards::lsb(tempPiece);
            tempPiece &= (tempPiece - 1);
            auto tempMove = Bitboards::queenAttacks(from, occupied) & targetBitboard;
            auto count = (hardwarePopcntEnabled ? Bitboards::hardwarePopcnt(tempMove) : Bitboards::softwarePopcnt(tempMove));
            scoreOpForColor += mobilityOpening[Piece::Queen][count];
            scoreEdForColor += mobilityEnding[Piece::Queen][count];
        }

        scoreOp += (c == Color::Black ? -scoreOpForColor : scoreOpForColor);
        scoreEd += (c == Color::Black ? -scoreEdForColor : scoreEdForColor);
    }

    return ((scoreOp * (256 - phase)) + (scoreEd * phase)) / 256;
}

template int Evaluation::mobilityEval<false>(const Position & pos, int phase);
template int Evaluation::mobilityEval<true>(const Position & pos, int phase);


