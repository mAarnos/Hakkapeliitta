#include "position.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include "bitboard.hpp"
#include "zobrist.hpp"
#include "color.hpp"
#include "square.hpp"

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
    0, 1, 1, 2, 4, 0
};

const int Position::totalPhase = piecePhase[Piece::Pawn] * 16 +
                                 piecePhase[Piece::Knight] * 4 +
                                 piecePhase[Piece::Bishop] * 4 +
                                 piecePhase[Piece::Rook] * 4 +
                                 piecePhase[Piece::Queen] * 2;

void Position::displayBoard() const
{
	static std::string pieceToMark = "PNBRQKpnbrqk.";

	std::cout << "  +-----------------------+" << std::endl;
	for (auto i = 7; i >= 0; --i)
	{
		std::cout << i + 1 << " ";
		for (auto j = 0; j < 8; ++j)
		{
			std::cout << "|" << pieceToMark[board[i * 8 + j]] << " ";
		}
		std::cout << "|" << std::endl << "  +--+--+--+--+--+--+--+--+" << std::endl;
	}
	std::cout << "   A  B  C  D  E  F  G  H" << std::endl;
}

void Position::initializeBoardFromFEN(const std::string & fen)
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

	hply = 0;
	if (strList.size() >= 6)
	{
		hply = 2 * stoi(strList[5]) - 1;
		// Avoid possible underflow.
		if (hply < 0)
		{
			hply = 0; 
		}
		if (sideToMove == Color::Black)
		{
			hply++;
		}
	}

    // Populate the bitboards.
	bitboards.fill(0);
    pieceCount.fill(0);
	for (Square sq = Square::A1; sq <= Square::H8; ++sq) 
	{
		if (board[sq] != Piece::Empty)
		{
			bitboards[board[sq]] |= Bitboards::bit[sq];
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
}

bool Position::makeMove(const Move & m, History & history)
{
    return (sideToMove ? makeMove<true>(m, history) : makeMove<false>(m, history));
}

void Position::unmakeMove(const Move & m, History & history)
{
    sideToMove ? unmakeMove<true>(m, history) : unmakeMove<false>(m, history);
}

bool Position::isAttacked(Square sq, Color side) const
{
    return (side ? isAttacked<true>(sq) : isAttacked<false>(sq));
}

template <bool side> bool Position::makeMove(const Move & m, History & history)
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
    history.mHash = materialHashKey;
    history.captured = captured;
    history.ep = enPassant;
    history.fifty = fiftyMoveDistance;
    history.castle = castlingRights;

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

        hashKey ^= Zobrist::pieceHash[captured][to];
        materialHashKey ^= Zobrist::materialHash[captured][--pieceCount[captured]];

        auto pieceType = (captured % 6);
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
        }
        else if (promotion != Piece::Empty) // Promotion
        {
            // This needs to be above the rest due to reasons. Try to fix that.
            materialHashKey ^= Zobrist::materialHash[promotion + side * 6][pieceCount[promotion + side * 6]++];

            bitboards[Piece::Pawn + side * 6] ^= Bitboards::bit[to];
            bitboards[promotion + side * 6] |= Bitboards::bit[to];
            hashKey ^= Zobrist::pieceHash[board[to]][to] ^ Zobrist::pieceHash[promotion + side * 6][to];
            pawnHashKey ^= Zobrist::pieceHash[board[to]][to];
            materialHashKey ^= Zobrist::materialHash[Piece::Pawn + side * 6][--pieceCount[Piece::Pawn + side * 6]];
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
        hashKey ^= (Zobrist::pieceHash[Piece::Rook + side * 6][fromRook] ^ Zobrist::pieceHash[Piece::Rook + side * 6][toRook]);
    }

    sideToMove = sideToMove.oppositeColor();
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

template bool Position::makeMove<false>(const Move & m, History & history);
template bool Position::makeMove<true>(const Move & m, History & history);

template <bool side> void Position::unmakeMove(const Move & m, History & history)
{
    auto from = m.getFrom();
    auto to = m.getTo();
    auto promotion = m.getPromotion();
    auto piece = board[to];
    auto fromToBB = Bitboards::bit[from] | Bitboards::bit[to];

    // Back up irreversible information from history.
    hashKey = history.hash;
    pawnHashKey = history.pHash;
    materialHashKey = history.mHash;
    auto captured = history.captured;
    enPassant = history.ep;
    fiftyMoveDistance = history.fifty;
    castlingRights = history.castle;

    sideToMove = sideToMove.oppositeColor();

    if (promotion == Piece::Pawn)
    {
        auto enPassantSquare = to ^ 8;
        bitboards[Piece::Pawn + side * 6] |= Bitboards::bit[enPassantSquare];
        bitboards[12 + side] |= Bitboards::bit[enPassantSquare];
        bitboards[14] |= Bitboards::bit[enPassantSquare];
        board[enPassantSquare] = Piece::Pawn + side * 6;
        ++pieceCount[Piece::Pawn + side * 6];
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
    }
    else if (promotion != Piece::Empty)
    {
        // Hack, fixes a slight problem with the backup when doing a promotion.
        piece = Piece::Pawn + !side * 6;
        bitboards[piece] ^= Bitboards::bit[to];
        bitboards[promotion + !side * 6] ^= Bitboards::bit[to];
        --pieceCount[promotion + !side * 6];
        ++pieceCount[piece];
    }

    bitboards[piece] ^= fromToBB;
    bitboards[12 + !side] ^= fromToBB;

    board[from] = piece;
    board[to] = captured;

    if (captured != Piece::Empty)
    {
        bitboards[captured] |= Bitboards::bit[to];
        bitboards[12 + side] |= Bitboards::bit[to];
        bitboards[14] |= Bitboards::bit[from];
        ++pieceCount[captured];
    }
    else
    {
        bitboards[14] ^= fromToBB;
    }
}

template void Position::unmakeMove<false>(const Move & m, History & history);
template void Position::unmakeMove<true>(const Move & m, History & history);

template <bool side> bool Position::isAttacked(Square sq) const
{
    if (Bitboards::knightAttacks[sq] & bitboards[Piece::Knight + 6 * side]
        || Bitboards::pawnAttacks[!side][sq] & bitboards[Piece::Pawn + 6 * side]
        || Bitboards::kingAttacks[sq] & bitboards[Piece::King + 6 * side]
        || Bitboards::bishopAttacks(sq, bitboards[14]) & (bitboards[Piece::Bishop + 6 * side] | bitboards[Piece::Queen + 6 * side])
        || Bitboards::rookAttacks(sq, bitboards[14]) & (bitboards[Piece::Rook + 6 * side] | bitboards[Piece::Queen + 6 * side]))
    {
        return true;
    }

    return false;
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
