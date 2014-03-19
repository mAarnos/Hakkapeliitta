#ifndef EVAL_H
#define EVAL_H

// The value of a draw in centipawns. Zero is the normal setting, negative values make the program avoid draws and positive values make it prefer them.
extern int drawscore;

const int pawnOpening = 88;
const int pawnEnding = 112;
const int knightOpening = 235;
const int knightEnding = 258;
const int bishopOpening = 263;
const int bishopEnding = 286;
const int rookOpening = 402;
const int rookEnding = 481;
const int queenOpening = 892;
const int queenEnding = 892;
const int mateScore = 32767; 

const int pawnValue = ((pawnOpening + pawnEnding) / 2);
const int minorValue = ((knightOpening + knightEnding + bishopOpening + bishopEnding) / 4);
const int rookValue = ((rookOpening + rookEnding) / 2);
const int queenValue = ((queenOpening + queenEnding) / 2);

const int pieceValues[16] = {
	0, pawnOpening, mateScore, knightOpening, 0, bishopOpening, rookOpening, queenOpening,
	0, pawnOpening, mateScore, knightOpening, 0, bishopOpening, rookOpening, queenOpening
};

// these two are updated incrementally, and whenever we need to know the score for material we interpolate the two depending on the gamephase.
extern int materialOpening; 
extern int materialEnding;

// Piece Square Tables, heavily based on center-manhattan distance

// -15,-10, -5, 0, 0, -5,-10,-15 for every rank(except 1st, 7th and 8th), rank bonus = 5 for every rank starting from 3rd and ending at 6th
// last rank bonus = 60(no other bonuses), 6th rank bonus = 20
// d3/e3 bonus = 10, d4/e4 bonus = 20, e5/d5 bonus = 20
// a4/h4 penalty = 10, f2/c2 bonus = 10, g2/b2 bonus = 5
// centered around zero
const int pawnPSTOpening[64] = { 
	0,  0,  0,  0,  0,  0,  0,  0,
  -33,-18,-13,-18,-18,-13,-18,-33,
  -28,-23,-13, -3, -3,-13,-23,-28,
  -33,-18,-13, 12, 12,-13,-18,-33,
  -18,-13, -8, 17, 17, -8,-13,-18,
    7, 12, 17, 22, 22, 17, 12,  7,
   42, 42, 42, 42, 42, 42, 42, 42,
	0,  0,  0,  0,  0,  0,  0,  0
};

// rank bonus = 5 starting from the 3rd rank and ending at 6th, last rank bonus = 60(no other bonuses), 6th rank bonus = 20
// centered around zero
const int pawnPSTEnding[64] = { 
	0,  0,  0,  0,  0,  0,  0,  0,
  -22,-22,-22,-22,-22,-22,-22,-22,
  -17,-17,-17,-17,-17,-17,-17,-17,
  -12,-12,-12,-12,-12,-12,-12,-12,
   -7, -7, -7, -7, -7, -7, -7, -7,
   18, 18, 18, 18, 18, 18, 18, 18,
   38, 38, 38, 38, 38, 38, 38, 38,
	0,  0,  0,  0,  0,  0,  0,  0
};

// based on the center-manhattan distance
// d5/e5 bonus = 10
// d6/e6 bonus = 30
// f5/c5 bonus = 10
// f6/c6 bonus = 10
// centered around zero
const int knightPSTOpening[64] = { 
  -36,-26,-16, -6, -6,-16,-26,-36,
  -26,-16, -6,  9,  9, -6,-16,-26,
  -16, -6,  9, 29, 29,  9, -6,-16,
   -6,  9, 29, 39, 39, 29,  9, -6,
   -6,  9, 39, 49, 49, 39,  9, -6,
  -16, -6, 19, 59, 59, 19, -6,-16,
  -26,-16, -6,  9,  9, -6,-16,-26,
  -36,-26,-16, -6, -6,-16,-26,-36
};

// based on the center-manhattan distance
// centered around zero
const int knightPSTEnding[64] = { 
  -34,-24,-14, -4, -4,-14,-24,-34,
  -24,-14, -4, 11, 11, -4,-14,-24,
  -14, -4, 11, 31, 31, 11, -4,-14,
   -4, 11, 31, 41, 41, 31, 11, -4,
   -4, 11, 31, 41, 41, 31, 11, -4,
  -14, -4, 11, 31, 31, 11, -4,-14,
  -24,-14, -4, 11, 11, -4,-14,-24,
  -34,-24,-14, -4, -4,-14,-24,-34
};

// based on center-manhattan distance
// last rank penalty = 10
// b2/g2 bonus = 10
// b5/g5 bonus = 5
// centered around zero
const int bishopPSTOpening[64] = { 
  -24,-19,-14, -9, -9,-14,-19,-24,
   -9,  6,  1,  6,  6,  1,  6, -9,
   -4,  1,  6, 11, 11,  6,  1, -4,
    1,  6, 11, 16, 16, 11,  6,  1,
    1, 11, 11, 16, 16, 11, 11,  1,
   -4,  1,  6, 11, 11,  6,  1, -4,
   -9, -4,  1,  6,  6,  1, -4, -9,
  -14, -9, -4,  1,  1, -5, -9,-14
};

// based on center-manhattan distance
// centered around zero
const int bishopPSTEnding[64] = { 
  -15,-10, -5,  0,  0, -5,-10,-15,
  -10, -5,  0,  5,  5,  0, -5,-10,
   -5,  0,  5, 10, 10,  5,  0, -5,
    0,  5, 10, 15, 15, 10,  5,  0,
    0,  5, 10, 15, 15, 10,  5,  0,
   -5,  0,  5, 10, 10,  5,  0, -5,
  -10, -5,  0,  5,  5,  0, -5,-10,
  -15,-10, -5,  0,  0, -5,-10,-15
};

// c-/f-file bonus = 2, d-/e-file bonus = 5, 7th rank bonus = 10
// centered around zero
const int rookPSTOpening[64] = { 
   -3, -3, -1,  2,  2, -1, -3, -3,
   -3, -3, -1,  2,  2, -1, -3, -3,
   -3, -3, -1,  2,  2, -1, -3, -3,
   -3, -3, -1,  2,  2, -1, -3, -3,
   -3, -3, -1,  2,  2, -1, -3, -3,
   -3, -3, -1,  2,  2, -1, -3, -3,
    7,  7,  9, 12, 12,  9,  7,  7,
   -3, -3, -1,  2,  2, -1, -3, -3,
};

// all zeroes is correct, there is no correct placement even generally for the rook in the endgame
// centered around zero
const int rookPSTEnding[64] = { 
	0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,
};

// based on center-manhattan distance
// first rank penalty = 5
// centered around zero
const int queenPSTOpening[64] = { 
   -19,-14, -9, -4, -4, -9,-14,-19,
    -9, -4,  1,  6,  6,  1, -4, -9,
    -4,  1,  6, 11, 11,  6,  1, -4,
     1,  6, 11, 16, 16, 11,  6,  1,
     1,  6, 11, 16, 16, 11,  6,  1,
    -4,  1,  6, 11, 11,  6,  1, -4,
    -9, -4,  1,  6,  6,  1, -4, -9,
   -14, -9, -4,  1,  1, -4, -9,-14
};

// based on center-manhattan distance
// centered around zero
const int queenPSTEnding[64] = { 
  -15,-10, -5,  0,  0, -5,-10,-15,
  -10, -5,  0,  5,  5,  0, -5,-10,
   -5,  0,  5, 10, 10,  5,  0, -5,
	0,  5, 10, 15, 15, 10,  5,  0,
    0,  5, 10, 15, 15, 10,  5,  0,
   -5,  0,  5, 10, 10,  5,  0, -5,
  -10, -5,  0,  5,  5,  0, -5,-10,
  -15,-10, -5,  0,  0, -5,-10,-15
};

// loosely based on reverse engineered DF11 king PST.
// cannot and should not be centered around zero
const int kingPSTOpening[64] = { 
	5, 10,  2,  0,  0,  6, 10,  4,
    5,  5,  0, -5, -5,  0,  5,  5,
   -5, -5, -5,-10,-10, -5, -5, -5,
  -10,-10,-20,-30,-30,-20,-10,-10,
  -20,-25,-30,-40,-40,-30,-25,-20,
  -40,-40,-50,-60,-60,-50,-40,-40,
  -50,-50,-60,-60,-60,-60,-50,-50,
  -60,-60,-60,-60,-60,-60,-60,-60
};

// based on center-manhattan distance
// centered around zero
const int kingPSTEnding[64] = { 
   -38,-28,-18, -8, -8,-18,-28,-38,
   -28,-18, -8, 13, 13, -8,-18,-28,
   -18, -8, 13, 43, 43, 13, -8,-18,
    -8, 13, 43, 53, 53, 43, 13, -8,
    -8, 13, 43, 53, 53, 43, 13, -8,
   -18, -8, 13, 43, 43, 13, -8,-18,
   -28,-18, -8, 13, 13, -8,-18,-28,
   -38,-28,-18, -8, -8,-18,-28,-38,
};

/* The flip array is used to calculate the piece/square
   values for black pieces. The piece/square value of a
   white pawn is pst[sq] and the value of a black
   pawn is pst[flip[sq]] */
const int flip[64] = { 
	 56,  57,  58,  59,  60,  61,  62,  63,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 16,  17,  18,  19,  20,  21,  22,  23,
	  8,   9,  10,  11,  12,  13,  14,  15,
	  0,   1,   2,   3,   4,   5,   6,   7
};
// used to keep track of PST values. Two are necessary so that the score can be interpolated
extern int scoreOpeningPST;
extern int scoreEndingPST;

// opening and ending scores for each possible number of moves for every piece, used by mobility eval

// average mobility for a knight in the opening is about 3.3
const int knightMobilityOpening[9] = { 
	-12, -8, -4,  0,  4,  8, 12, 16,
	 20
};

// average mobility for a knight in the ending is about 3.3
// maybe move 0 up one slot
const int knightMobilityEnding[9] = { 
	-12, -8, -4,  0,  4,  8, 12, 16,
	 20
};

// average mobility for a bishop in the opening is about 3.1
const int bishopMobilityOpening[14] = { 
	-9, -6, -3,  0,  3,  6,  9, 12,
	15, 18, 21, 24, 27, 30
};

// average mobility for a bishop in the ending is about 3.8
const int bishopMobilityEnding[14] = { 
   -12, -9, -6, -3,  0,  3,  6,  9,
	12, 15, 18, 21, 24, 27
};

// average mobility for a rook in the opening is about 2.6
const int rookMobilityOpening[15] = { 
	-6, -4, -2,  0,  2,  4,  6,  8,
	10, 12, 14, 16, 18, 20, 22
};

// average mobility for a rook in the ending is about 5.1
const int rookMobilityEnding[15] = { 
   -10, -8, -6, -4, -2,  0,  2,  4,
     6,  8, 10, 12, 14, 16, 18
};
// average mobility for a queen in the opening is about 6.6
const int queenMobilityOpening[28] = { 
	-7, -6, -5, -4, -3, -2, -1,  0,
	 1,  2,  3,  4,  5,  6,  7,  8,
	 9, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20
};
// average mobility for a queen in the ending is about 9.6
const int queenMobilityEnding[28] = { 
   -10, -9, -8, -7, -6, -5, -4, -3,
	-2, -1,  0,  1,  2,  3,  4,  5,
	 6,  7,  8,  9, 10, 11, 12, 13,
	14, 15, 16, 17
};

// pawn structure bonuses and penalties
const int doubledPenaltyOpening = 15;
const int doubledPenaltyEnding = 16;
const int passedBonusOpening = -4; 
const int passedBonusEnding = 51; 
const int isolatedPenaltyOpening = 13; 
const int isolatedPenaltyEnding = 17; 
const int backwardPenaltyOpening = 13; 
const int backwardPenaltyEnding = 9;  

// material inbalance bonuses and penalties
const int bishopPairBonus = 32;
const int tradeDownBonus = 3;

// following have no endgame counterpart as in the endgame the rook is always on a open or a semi-open file 
const int rookOnOpenFileBonus = 10;
const int rookOnSemiOpenFileBonus = 5;

// following has no middlegame counterpart as there being behind a passe pawn isn't that important
const int rookBehindPassedBonus = 20;

// king tropism bonus tables
extern int queenTropism[64][64];
extern int rookTropism[64][64];
extern int knightTropism[64][64];
extern int bishopTropism[64][64];

// used to keep track of the game phase so tapered eval can work. 
extern int phase;

// used to calculate phase
const int pawnPhase = 0;
const int knightPhase = 1;
const int bishopPhase = 1;
const int rookPhase = 2;
const int queenPhase = 4;
const int totalPhase = pawnPhase*16 + knightPhase*4 + bishopPhase*4 + rookPhase*4 + queenPhase*2;

extern void initializeKingTropism();
int eval();

#endif