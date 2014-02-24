#ifndef MAGIC_H
#define MAGIC_H

#include "defs.h"

extern U64 magicMovesRook[Squares][4096];
extern U64 magicMovesBishop[Squares][512];
extern U64 magicNumberRook[Squares];
extern U64 magicNumberBishop[Squares];
extern U64 occupancyMaskRook[Squares];
extern U64 occupancyMaskBishop[Squares];
extern U64 occupancyVariation[Squares][4096];
extern U64 occupancyAttackSet[Squares][4096];
extern int rookMagicShifts[Squares];
extern int bishopMagicShifts[Squares];

extern void initializeMagics();

#endif