#include "position.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "bitboard.hpp"
#include "zobrist.hpp"
#include "color.hpp"
#include "square.hpp"
#include "eval.hpp"
#include "utils\synchronized_ostream.hpp"

const std::array<int, 64> Position::castlingMask = {
	2, 0, 0, 0, 3, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	8, 0, 0, 0, 12, 0, 0, 4
};

const std::array<int, 6> Position::piecePhase = {
    0, 3, 3, 5, 10, 0
};

const int Position::totalPhase = piecePhase[Piece::Pawn] * 16 +
                                 piecePhase[Piece::Knight] * 4 +
                                 piecePhase[Piece::Bishop] * 4 +
                                 piecePhase[Piece::Rook] * 4 +
                                 piecePhase[Piece::Queen] * 2;

Position::Position()
{
    bitboards.fill(0);
    board.fill(Piece::Empty);
    hashKey = pawnHashKey = materialHashKey = 0;
    sideToMove = Color::White;
    castlingRights = 0;
    enPassant = Square::NoSquare;
    phase = 0;
    fiftyMoveDistance = 0;
    ply = 0;
    pstMaterialScoreOp = pstMaterialScoreEd = 0;
    pieceCount.fill(0);
}

void Position::displayBoard() const
{
	static std::string pieceToMark = "PNBRQKpnbrqk.";

	sync_cout << "  +-----------------------+" << std::endl;
	for (auto i = 7; i >= 0; --i)
	{
		sync_cout << i + 1 << " ";
		for (auto j = 0; j < 8; ++j)
		{
			sync_cout << "|" << pieceToMark[board[i * 8 + j]] << " ";
		}
		sync_cout << "|" << std::endl << "  +--+--+--+--+--+--+--+--+" << std::endl;
	}
	sync_cout << "   A  B  C  D  E  F  G  H" << std::endl;
}

void Position::initializePositionFromFen(const std::string& fen)
{
	board.fill(Piece::Empty);

	// Split the FEN into parts.
	std::vector<std::string> strList;
	std::stringstream ss(fen);
	std::string item;
	while (getline(ss, item, ' ')) 
	{
		strList.push_back(item);
	}

	auto i = 0, j = 1;
	// Translate the FEN string into piece locations on the board.
	while ((j <= 64) && (i <= static_cast<int>(strList[0].length()))) 
	{
        auto letter = strList[0][i++];
		auto f = 1 + file(j - 1);
        auto r = 8 - rank(j - 1);
		auto sq = (((r - 1) * 8) + (f - 1));
		switch (letter)
		{
            case 'p': board[sq] = Piece::BlackPawn; break;
            case 'r': board[sq] = Piece::BlackRook; break;
            case 'n': board[sq] = Piece::BlackKnight; break;
            case 'b': board[sq] = Piece::BlackBishop; break;
            case 'q': board[sq] = Piece::BlackQueen; break;
            case 'k': board[sq] = Piece::BlackKing; break;
            case 'P': board[sq] = Piece::WhitePawn; break;
            case 'R': board[sq] = Piece::WhiteRook; break;
            case 'N': board[sq] = Piece::WhiteKnight; break;
            case 'B': board[sq] = Piece::WhiteBishop; break;
            case 'Q': board[sq] = Piece::WhiteQueen; break;
            case 'K': board[sq] = Piece::WhiteKing; break;
			case '/': --j; break;
			case '1': break;
			case '2': ++j; break;
			case '3': j += 2; break;
			case '4': j += 3; break;
			case '5': j += 4; break;
			case '6': j += 5; break;
			case '7': j += 6; break;
			case '8': j += 7; break;
            default: throw std::invalid_argument("fen string is incorrect: unknown character encountered");
		}
		++j;
	}

	// set the turn; default = White 
	sideToMove = Color::White;
	if (strList.size() >= 2)
	{
		if (strList[1] == "w")
		{
            sideToMove = Color::White;
		}
        else if (strList[1] == "b")
        {
            sideToMove = Color::Black;
        }
        else
        {
            throw std::invalid_argument("fen string is incorrect: side to move is corrupted");
        }
	}

	// Set castlingRights to default: no castling allowed.
	castlingRights = 0;
	// Add all given castling rights.
	if (strList.size() >= 3)
	{
		if (strList[2].find('K') != std::string::npos)
		{
			castlingRights += 1;
		}
        if (strList[2].find('Q') != std::string::npos)
		{
			castlingRights += 2;
		}
        if (strList[2].find('k') != std::string::npos)
		{
			castlingRights += 4;
		}
        if (strList[2].find('q') != std::string::npos)
		{
			castlingRights += 8;
		}
	}

	// Set the en passant square, if any.
	enPassant = Square::NoSquare;
	if ((strList.size() >= 4) && (strList[3].length() >= 2))
	{
		if ((strList[3].at(0) >= 'a') && (strList[3].at(0) <= 'h') && ((strList[3].at(1) == '3') || (strList[3].at(1) == '6')))
		{
			auto f = strList[3][0] - 96; // ASCII 'a' = 97 
			auto r = strList[3][1] - 48; // ASCII '1' = 49 
			enPassant = ((r - 1) * 8 + f - 1);
		}
        else
        {
            throw std::invalid_argument("fen string is incorrect: en passant square is corrupted");
        }
	}

	// Fifty move distance, we start at 0 by default.
	fiftyMoveDistance = 0;
	if (strList.size() >= 5)
	{
		fiftyMoveDistance = stoi(strList[4]);
	}

	ply = 0;
	if (strList.size() >= 6)
	{
		ply = 2 * stoi(strList[5]) - 1;
		// Avoid possible underflow.
		if (ply < 0)
		{
			ply = 0; 
		}
		if (sideToMove == Color::Black)
		{
			++ply;
		}
	}

    // Populate the bitboards.
	bitboards.fill(0);
    pieceCount.fill(0);
    pstMaterialScoreOp = pstMaterialScoreEd = 0;
	for (Square sq = Square::A1; sq <= Square::H8; ++sq) 
	{
		if (board[sq] != Piece::Empty)
		{
			bitboards[board[sq]] |= Bitboards::bit[sq];
            pstMaterialScoreOp += Evaluation::pieceSquareTableOpening[board[sq]][sq];
            pstMaterialScoreEd += Evaluation::pieceSquareTableEnding[board[sq]][sq];
            ++pieceCount[board[sq]];
		}
	}

    bitboards[12] = bitboards[Piece::WhiteKing] | bitboards[Piece::WhiteQueen] | bitboards[Piece::WhiteRook] 
                  | bitboards[Piece::WhiteBishop] | bitboards[Piece::WhiteKnight] | bitboards[Piece::WhitePawn];
    bitboards[13] = bitboards[Piece::BlackKing] | bitboards[Piece::BlackQueen] | bitboards[Piece::BlackRook] 
                  | bitboards[Piece::BlackBishop] | bitboards[Piece::BlackKnight] | bitboards[Piece::BlackPawn];
	bitboards[14] = bitboards[12] | bitboards[13];

    // Calculate all the different hash keys for the position.
	hashKey = calculateHash();
	pawnHashKey = calculatePawnHash();
	materialHashKey = calculateMaterialHash();

    // Calculate the phase of the game.
    phase = totalPhase;
    for (Piece p = Piece::Knight; p < Piece::King; ++p)
    {
        phase -= (pieceCount[Color::White + p] + pieceCount[Color::Black * 6 + p]) * piecePhase[p];
    }
}

std::string Position::positionToFen() const
{
    std::string fen;

    for (auto r = 7; r >= 0; --r)
    {
        auto emptySquares = 0;
        for (auto f = 0; f < 8; ++f)
        {
            int piece = board[r * 8 + f];
            if (piece == Piece::Empty)
            {
                ++emptySquares;
            }
            else
            {
                if (emptySquares > 0)
                {
                    fen += static_cast<char>('0' + emptySquares);
                    emptySquares = 0;
                }

                switch (piece)
                {
                    case Piece::BlackPawn: fen += "p"; break;
                    case Piece::BlackRook: fen += "r"; break;
                    case Piece::BlackKnight: fen += "n"; break;
                    case Piece::BlackBishop: fen += "b"; break;
                    case Piece::BlackQueen: fen += "q"; break;
                    case Piece::BlackKing: fen += "k"; break;
                    case Piece::WhitePawn: fen += "P"; break;
                    case Piece::WhiteRook: fen += "R"; break;
                    case Piece::WhiteKnight: fen += "N"; break;
                    case Piece::WhiteBishop: fen += "B"; break;
                    case Piece::WhiteQueen: fen += "Q"; break;
                    case Piece::WhiteKing: fen += "K"; break;
                    default: return "";
                }
            }
        }

        if (emptySquares > 0)
        {
            fen += static_cast<char>('0' + emptySquares);
        }
        if (rank > 0)
        {
            fen += '/';
        }
    }

    fen += (sideToMove ? " b " : " w ");

    auto sizeBeforeCastling = fen.size();
    if (castlingRights & 1)
    {
        fen += "K";
    }
    if (castlingRights & 2)
    {
        fen += "Q";
    }
    if (castlingRights & 4)
    {
        fen += "k";
    }
    if (castlingRights & 8)
    {
        fen += "q";
    }
    if (sizeBeforeCastling == fen.size())
    {
        fen += "-";
    }

    fen += " ";
    if (enPassant != Square::NoSquare)
    {
        fen += static_cast<char>('a' + file(enPassant));
        fen += static_cast<char>('1' + rank(enPassant));
    }
    else
    {
        fen += "-";
    }

    fen += " ";
    fen += std::to_string(fiftyMoveDistance);
    fen += " ";
    fen += std::to_string((ply + 1) / 2);

    return fen;
}

bool Position::makeMove(const Move& m, History& history)
{
    return (sideToMove ? makeMove<true>(m, history) : makeMove<false>(m, history));
}

void Position::unmakeMove(const Move& m, const History& history)
{
    sideToMove ? unmakeMove<true>(m, history) : unmakeMove<false>(m, history);
}

bool Position::isAttacked(Square sq, Color side) const
{
    return (side ? isAttacked<true>(sq) : isAttacked<false>(sq));
}

int Position::SEE(const Move& m) const
{
    // Approximate piece values, SEE doesn't need to be as accurate as the main evaluation function.
    // Score for kings is not mateScore due to some annoying wrap-around problems. Doesn't really matter though.
    static const std::array<int, 13> pieceValues = {
        100, 300, 300, 500, 900, 10000, 100, 300, 300, 500, 900, 10000, 0
    };
    static std::array<int, 32> materialGains;
    auto occupied = getOccupiedSquares();
    auto from = m.getFrom();
    auto to = m.getTo();
    auto promotion = m.getPromotion();
    auto toAtPromoRank = (to <= 7 || to >= 56); 
    auto stm = sideToMove;
    int lastAttackerValue, next;

    if (promotion == Piece::King)
    {
        return 0;
    }
    else if (promotion == Piece::Pawn)
    {
        materialGains[0] = pieceValues[Piece::Pawn];
        lastAttackerValue = pieceValues[Piece::Pawn];
        occupied ^= Bitboards::bit[enPassant];
    }
    else
    {
        materialGains[0] = pieceValues[board[to]];
        lastAttackerValue = pieceValues[board[from]];
        if (promotion != Piece::Empty)
        {
            materialGains[0] += pieceValues[promotion] - pieceValues[Piece::Pawn];
            lastAttackerValue += pieceValues[promotion] - pieceValues[Piece::Pawn];
        }
    }

    occupied ^= Bitboards::bit[from];
    auto attackers = (Bitboards::rookAttacks(to, occupied) & (bitboards[Piece::WhiteQueen] 
                                                            | bitboards[Piece::BlackQueen] 
                                                            | bitboards[Piece::WhiteRook] 
                                                            | bitboards[Piece::BlackRook]))
                 | (Bitboards::bishopAttacks(to, occupied) & (bitboards[Piece::WhiteQueen] 
                                                            | bitboards[Piece::BlackQueen] 
                                                            | bitboards[Piece::WhiteBishop]
                                                            | bitboards[Piece::BlackBishop]))
                 | (Bitboards::knightAttacks[to] & (bitboards[Piece::WhiteKnight] | bitboards[Piece::BlackKnight]))
                 | (Bitboards::kingAttacks[to] & (bitboards[Piece::WhiteKing] | bitboards[Piece::BlackKing]))
                 | (Bitboards::pawnAttacks[Color::Black][to] & (bitboards[Piece::WhitePawn]))
                 | (Bitboards::pawnAttacks[Color::White][to] & (bitboards[Piece::BlackPawn]));
    attackers &= occupied;
    stm = !stm;
    auto numberOfCaptures = 1;

    while (attackers & bitboards[12 + stm])
    {
        if (!toAtPromoRank && bitboards[Piece::Pawn + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Pawn + stm * 6] & attackers);
        }
        else if (bitboards[Piece::Knight + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Knight + stm * 6] & attackers);
        }
        else if (bitboards[Piece::Bishop + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Bishop + stm * 6] & attackers);
        }
        else if (bitboards[Piece::Rook + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Rook + stm * 6] & attackers);
        }
        else if (toAtPromoRank && bitboards[Piece::Pawn + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Pawn + stm * 6] & attackers);
        }
        else if (bitboards[Piece::Queen + stm * 6] & attackers)
        {
            next = Bitboards::lsb(bitboards[Piece::Queen + stm * 6] & attackers);
        }
        else
        {
            next = Bitboards::lsb(bitboards[Piece::King + stm * 6]);
        }

        // Update the materialgains array.
        materialGains[numberOfCaptures] = -materialGains[numberOfCaptures - 1] + lastAttackerValue;
        // Remember the value of the capturing piece because it is going to be captured next.
        lastAttackerValue = pieceValues[board[next]];
        // If we are going to do a promotion we need to correct the values a bit.
        if (toAtPromoRank && lastAttackerValue == pieceValues[Piece::Pawn])
        {
            materialGains[numberOfCaptures] += pieceValues[Piece::Queen] - pieceValues[Piece::Pawn];
            lastAttackerValue += pieceValues[Piece::Queen] - pieceValues[Piece::Pawn];
        }

        occupied ^= Bitboards::bit[next];
        attackers |= (Bitboards::rookAttacks(to, occupied) & (bitboards[Piece::WhiteQueen] 
                                                            | bitboards[Piece::BlackQueen] 
                                                            | bitboards[Piece::WhiteRook] 
                                                            | bitboards[Piece::BlackRook]))
                  | (Bitboards::bishopAttacks(to, occupied) & (bitboards[Piece::WhiteQueen] 
                                                             | bitboards[Piece::BlackQueen] 
                                                             | bitboards[Piece::WhiteBishop] 
                                                             | bitboards[Piece::BlackBishop]));
        attackers &= occupied;

        stm = !stm;
        if ((board[next] % 6) == Piece::King && (attackers & bitboards[12 + stm]))
            break;
        numberOfCaptures++;
    }

    while (--numberOfCaptures)
    {
        materialGains[numberOfCaptures - 1] = std::min(-materialGains[numberOfCaptures], materialGains[numberOfCaptures - 1]);
    }

    return materialGains[0];
}

template <bool side> 
bool Position::makeMove(const Move& m, History& history)
{
    auto from = m.getFrom();
    auto to = m.getTo();
    auto promotion = m.getPromotion();
    auto piece = board[from];
    auto captured = board[to];
    auto fromToBB = Bitboards::bit[from] | Bitboards::bit[to];

    // Save all information needed to take back a move.
    history.hash = hashKey;
    history.pHash = pawnHashKey;
    history.captured = captured;
    history.ep = enPassant;
    history.fifty = fiftyMoveDistance;
    history.castle = castlingRights;

    pstMaterialScoreOp += Evaluation::pieceSquareTableOpening[piece][to] 
                        - Evaluation::pieceSquareTableOpening[piece][from];
    pstMaterialScoreEd += Evaluation::pieceSquareTableEnding[piece][to]
                        - Evaluation::pieceSquareTableEnding[piece][from];

    if (enPassant != Square::NoSquare)
    {
        hashKey ^= Zobrist::enPassantHash[enPassant];
        enPassant = Square::NoSquare;
    }

    board[to] = board[from];
    board[from] = Piece::Empty;
    bitboards[piece] ^= fromToBB;
    bitboards[12 + side] ^= fromToBB;
    ++fiftyMoveDistance;
    hashKey ^= (Zobrist::pieceHash[piece][from] ^ Zobrist::pieceHash[piece][to]);

    if (captured != Piece::Empty)
    {
        bitboards[captured] ^= Bitboards::bit[to];
        bitboards[12 + !side] ^= Bitboards::bit[to];
        fiftyMoveDistance = 0;
        pstMaterialScoreOp -= Evaluation::pieceSquareTableOpening[captured][to];
        pstMaterialScoreEd -= Evaluation::pieceSquareTableEnding[captured][to];
        hashKey ^= Zobrist::pieceHash[captured][to];
        materialHashKey ^= Zobrist::materialHash[captured][--pieceCount[captured]];

        auto pieceType = (captured % 6);
        phase += piecePhase[pieceType];
        if (pieceType == Piece::Pawn)
        {
            pawnHashKey ^= Zobrist::pieceHash[captured][to];
        }

        bitboards[14] ^= Bitboards::bit[from];
    }
    else
    {
        bitboards[14] ^= fromToBB;
    }

    if ((piece % 6) == Piece::Pawn)
    {
        fiftyMoveDistance = 0;
        pawnHashKey ^= (Zobrist::pieceHash[piece][from] ^ Zobrist::pieceHash[piece][to]);

        if ((to ^ from) == 16) // Double pawn move
        {
            enPassant = from ^ 24;
            hashKey ^= Zobrist::enPassantHash[enPassant];
        }
        else if (promotion == Piece::Pawn) // En passant
        {
            auto enPassantSquare = to ^ 8;
            bitboards[Piece::Pawn + !side * 6] ^= Bitboards::bit[enPassantSquare];
            bitboards[12 + !side] ^= Bitboards::bit[enPassantSquare];
            bitboards[14] ^= Bitboards::bit[enPassantSquare];
            board[enPassantSquare] = Piece::Empty;
            hashKey ^= Zobrist::pieceHash[Piece::Pawn + !side * 6][enPassantSquare];
            pawnHashKey ^= Zobrist::pieceHash[Piece::Pawn + !side * 6][enPassantSquare];
            materialHashKey ^= Zobrist::materialHash[Piece::Pawn + !side * 6][--pieceCount[Piece::Pawn + !side * 6]];
            pstMaterialScoreOp -= Evaluation::pieceSquareTableOpening[Piece::Pawn + !side * 6][enPassantSquare];
            pstMaterialScoreEd -= Evaluation::pieceSquareTableEnding[Piece::Pawn + !side * 6][enPassantSquare];
        }
        else if (promotion != Piece::Empty) // Promotion
        {
            // This needs to be above the rest due to reasons. Try to fix that.
            materialHashKey ^= Zobrist::materialHash[promotion + side * 6][pieceCount[promotion + side * 6]++];
            phase -= piecePhase[promotion];
            bitboards[Piece::Pawn + side * 6] ^= Bitboards::bit[to];
            bitboards[promotion + side * 6] |= Bitboards::bit[to];
            board[to] = promotion + sideToMove * 6;
            hashKey ^= Zobrist::pieceHash[Piece::Pawn + side * 6][to] ^ Zobrist::pieceHash[promotion + side * 6][to];
            pawnHashKey ^= Zobrist::pieceHash[Piece::Pawn + side * 6][to];
            materialHashKey ^= Zobrist::materialHash[Piece::Pawn + side * 6][--pieceCount[Piece::Pawn + side * 6]];
            pstMaterialScoreOp += Evaluation::pieceSquareTableOpening[promotion + side * 6][to]
                                - Evaluation::pieceSquareTableOpening[Piece::Pawn + side * 6][to];
            pstMaterialScoreEd += Evaluation::pieceSquareTableEnding[promotion + side * 6][to]
                                - Evaluation::pieceSquareTableEnding[Piece::Pawn + side * 6][to];
        }
    }
    else if (promotion == Piece::King)
    {
        auto fromRook = (from > to ? (to - 2) : (to + 1));
        auto toRook = (from > to ? (from - 1) : (from + 1));
        auto fromToBBCastling = Bitboards::bit[fromRook] | Bitboards::bit[toRook];

        bitboards[Piece::Rook + side * 6] ^= fromToBBCastling;
        bitboards[12 + side] ^= fromToBBCastling;
        bitboards[14] ^= fromToBBCastling;
        board[toRook] = board[fromRook];
        board[fromRook] = Piece::Empty;
        hashKey ^= Zobrist::pieceHash[Piece::Rook + side * 6][fromRook] 
                 ^ Zobrist::pieceHash[Piece::Rook + side * 6][toRook];
        pstMaterialScoreOp += Evaluation::pieceSquareTableOpening[Piece::Rook + side * 6][toRook]
                            - Evaluation::pieceSquareTableOpening[Piece::Rook + side * 6][fromRook];
        pstMaterialScoreEd += Evaluation::pieceSquareTableEnding[Piece::Rook + side * 6][toRook]
                            - Evaluation::pieceSquareTableEnding[Piece::Rook + side * 6][fromRook];
    }

    sideToMove = !sideToMove;
    hashKey ^= Zobrist::turnHash;

    // Update castling rights if needed
    if (castlingRights && (castlingMask[from] | castlingMask[to]))
    {
        auto cf = castlingMask[from] | castlingMask[to];
        hashKey ^= Zobrist::castlingRightsHash[castlingRights & cf];
        castlingRights &= ~cf;
    }

    // Check if the move leaves us in check.
    if (isAttacked(Bitboards::lsb(bitboards[Piece::King + side * 6]), !side))
    {
        unmakeMove(m, history);
        return false;
    }

    return true;
}

template bool Position::makeMove<false>(const Move& m, History& history);
template bool Position::makeMove<true>(const Move& m, History& history);

template <bool side> 
void Position::unmakeMove(const Move& m, const History& history)
{
    auto from = m.getFrom();
    auto to = m.getTo();
    auto promotion = m.getPromotion();
    auto piece = board[to];
    auto fromToBB = Bitboards::bit[from] | Bitboards::bit[to];

    // Back up irreversible information from history.
    hashKey = history.hash;
    pawnHashKey = history.pHash;
    auto captured = history.captured;
    enPassant = history.ep;
    fiftyMoveDistance = history.fifty;
    castlingRights = history.castle;

    sideToMove = !sideToMove;

    if (promotion == Piece::Pawn)
    {
        auto enPassantSquare = to ^ 8;
        bitboards[Piece::Pawn + side * 6] |= Bitboards::bit[enPassantSquare];
        bitboards[12 + side] |= Bitboards::bit[enPassantSquare];
        bitboards[14] |= Bitboards::bit[enPassantSquare];
        board[enPassantSquare] = Piece::Pawn + side * 6;
        materialHashKey ^= Zobrist::materialHash[Piece::Pawn + side * 6][pieceCount[Piece::Pawn + side * 6]++];
        pstMaterialScoreOp += Evaluation::pieceSquareTableOpening[Piece::Pawn + side * 6][enPassantSquare];
        pstMaterialScoreEd += Evaluation::pieceSquareTableEnding[Piece::Pawn + side * 6][enPassantSquare];
    }
    else if (promotion == Piece::King)
    {
        auto fromRook = (from > to ? (to - 2) : (to + 1));
        auto toRook = (from > to ? (from - 1) : (from + 1));
        auto fromToBBCastling = Bitboards::bit[fromRook] | Bitboards::bit[toRook];

        bitboards[Piece::Rook + !side * 6] ^= fromToBBCastling;
        bitboards[12 + !side] ^= fromToBBCastling;
        bitboards[14] ^= fromToBBCastling;
        board[fromRook] = board[toRook];
        board[toRook] = Piece::Empty;
        pstMaterialScoreOp += Evaluation::pieceSquareTableOpening[Piece::Rook + !side * 6][fromRook]
                            - Evaluation::pieceSquareTableOpening[Piece::Rook + !side * 6][toRook];
        pstMaterialScoreEd += Evaluation::pieceSquareTableEnding[Piece::Rook + !side * 6][fromRook]
                            - Evaluation::pieceSquareTableEnding[Piece::Rook + !side * 6][toRook];
    }
    else if (promotion != Piece::Empty)
    {
        piece = Piece::Pawn + !side * 6; // Hack, fixes a slight problem with the backup when doing a promotion.
        bitboards[piece] ^= Bitboards::bit[to];
        bitboards[promotion + !side * 6] ^= Bitboards::bit[to];
        phase += piecePhase[promotion];
        materialHashKey ^= Zobrist::materialHash[promotion + !side * 6][--pieceCount[promotion + !side * 6]];
        materialHashKey ^= Zobrist::materialHash[Piece::Pawn + !side * 6][pieceCount[piece]++];
        pstMaterialScoreOp += Evaluation::pieceSquareTableOpening[Piece::Pawn + !side * 6][to]
                            - Evaluation::pieceSquareTableOpening[promotion + !side * 6][to];
        pstMaterialScoreEd += Evaluation::pieceSquareTableEnding[Piece::Pawn + !side * 6][to]
                            - Evaluation::pieceSquareTableEnding[promotion + !side * 6][to];
    }

    bitboards[piece] ^= fromToBB;
    bitboards[12 + !side] ^= fromToBB;

    board[from] = piece;
    board[to] = captured;

    pstMaterialScoreOp += Evaluation::pieceSquareTableOpening[piece][from]
                        - Evaluation::pieceSquareTableOpening[piece][to];
    pstMaterialScoreEd += Evaluation::pieceSquareTableEnding[piece][from]
                        - Evaluation::pieceSquareTableEnding[piece][to];

    if (captured != Piece::Empty)
    {
        bitboards[captured] |= Bitboards::bit[to];
        bitboards[12 + side] |= Bitboards::bit[to];
        bitboards[14] |= Bitboards::bit[from];
        phase -= piecePhase[captured % 6];
        materialHashKey ^= Zobrist::materialHash[captured][pieceCount[captured]++];
        pstMaterialScoreOp += Evaluation::pieceSquareTableOpening[captured][to];
        pstMaterialScoreEd += Evaluation::pieceSquareTableEnding[captured][to];
    }
    else
    {
        bitboards[14] ^= fromToBB;
    }
}

template void Position::unmakeMove<false>(const Move& m, const History& history);
template void Position::unmakeMove<true>(const Move& m, const History& history);

template <bool side> 
bool Position::isAttacked(Square sq) const
{
    return (Bitboards::knightAttacks[sq] & bitboards[Piece::Knight + 6 * side]
        || Bitboards::pawnAttacks[!side][sq] & bitboards[Piece::Pawn + 6 * side]
        || Bitboards::kingAttacks[sq] & bitboards[Piece::King + 6 * side]
        || Bitboards::bishopAttacks(sq, bitboards[14]) & (bitboards[Piece::Bishop + 6 * side] | bitboards[Piece::Queen + 6 * side])
        || Bitboards::rookAttacks(sq, bitboards[14]) & (bitboards[Piece::Rook + 6 * side] | bitboards[Piece::Queen + 6 * side]));
}

template bool Position::isAttacked<false>(Square sq) const;
template bool Position::isAttacked<true>(Square sq) const;

HashKey Position::calculateHash() const
{
    HashKey h = 0;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (board[sq] != Piece::Empty)
        {
            h ^= Zobrist::pieceHash[board[sq]][sq];
        }
    }

    if (enPassant != Square::NoSquare)
    {
        h ^= Zobrist::enPassantHash[enPassant];
    }
    if (sideToMove)
    {
        h ^= Zobrist::turnHash;
    }
    if (castlingRights)
    {
        h ^= Zobrist::castlingRightsHash[castlingRights];
    }

    return h;
}

HashKey Position::calculatePawnHash() const
{
    HashKey p = 0;

    for (Square sq = Square::A1; sq <= Square::H8; ++sq)
    {
        if (board[sq] == Piece::WhitePawn || board[sq] == Piece::BlackPawn)
        {
            p ^= Zobrist::pieceHash[board[sq]][sq];
        }
    }

    return p;
}

HashKey Position::calculateMaterialHash() const
{
    HashKey m = 0;

    for (Piece p = Piece::WhitePawn; p <= Piece::BlackKing; ++p)
    {
        for (auto i = 0; i < pieceCount[p]; ++i)
        {
            m ^= Zobrist::materialHash[p][i];
        }
    }

    return m;
}

void Position::makeNullMove(History& history)
{
    sideToMove = !sideToMove;
    hashKey ^= Zobrist::turnHash;
    history.ep = enPassant;
    if (enPassant != Square::NoSquare)
    {
        hashKey ^= Zobrist::enPassantHash[enPassant];
        enPassant = Square::NoSquare;
    }
}

void Position::unmakeNullMove(const History& history)
{
    enPassant = history.ep;
    if (enPassant != Square::NoSquare)
    {
        hashKey ^= Zobrist::enPassantHash[enPassant];
    }
    sideToMove = !sideToMove;
    hashKey ^= Zobrist::turnHash;
}
