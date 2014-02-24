/*
  Copyright (c) 2011-2013 Ronald de Man
*/

// done

#ifndef TBCORE_H
#define TBCORE_H

#include "defs.h"
#include <windows.h>

// Define DECOMP64 when compiling for a 64-bit platform.
// 32-bit is only supported for 5-piece tables, because tables are mmap()ed
// into memory.
#define DECOMP64

#define WDLSUFFIX ".rtbw"
#define DTZSUFFIX ".rtbz"
#define WDLDIR "RTBWDIR"
#define DTZDIR "RTBZDIR"
#define TBPIECES 6

#define WDL_MAGIC 0x5d23e871
#define DTZ_MAGIC 0xa50c66d7

#define TBHASHBITS 10

#define TBMAX_PIECE 254
#define TBMAX_PAWN 256
#define HSHMAX 33

#define Swap(a,b) {int tmp=a;a=b;b=tmp;}

#define TB_PAWN 1
#define TB_KNIGHT 2
#define TB_BISHOP 3
#define TB_ROOK 4
#define TB_QUEEN 5
#define TB_KING 6

#define TB_WPAWN TB_PAWN
#define TB_BPAWN (TB_PAWN | 8)

extern char TBdir[128];
extern char WDLdir[128];

extern int TBnum_piece, TBnum_pawn;
extern struct TBEntry_piece TB_piece[TBMAX_PIECE];
extern struct TBEntry_pawn TB_pawn[TBMAX_PAWN];

extern struct TBHashEntry TB_hash[1 << TBHASHBITS][HSHMAX];

#define DTZ_ENTRIES 64

extern struct DTZTableEntry DTZ_table[DTZ_ENTRIES];

struct TBHashEntry;

typedef U64 base_t;

inline U32 bswap32(U32 x)
{
	return  ((x << 24) & 0xff000000 ) |
	((x <<  8) & 0x00ff0000 ) |
	((x >>  8) & 0x0000ff00 ) |
	((x >> 24) & 0x000000ff );
}

inline U64 bswap64(U64 x)
{
	U32 tl, th;
	th = bswap32((U32)(x & 0x00000000ffffffffULL));
	tl = bswap32((U32)((x >> 32) & 0x00000000ffffffffULL));
	return ((U64)th << 32) | tl;
}

struct PairsData {
  char *indextable;
  U16 *sizetable;
  U8 *data;
  U16 *offset;
  U8 *symlen;
  U8 *sympat;
  int blocksize;
  int idxbits;
  int min_len;
  base_t base[1]; // C++ complains about base[]...
};

struct TBEntry {
  char *data;
  U64 key;
  U8 ready;
  U8 num;
  U8 symmetric;
  U8 has_pawns;
};

struct TBEntry_piece {
  char *data;
  U64 key;
  U8 ready;
  U8 num;
  U8 symmetric;
  U8 has_pawns;
  U8 enc_type;
  struct PairsData *precomp[2];
  int factor[2][TBPIECES];
  U8 pieces[2][TBPIECES];
  U8 norm[2][TBPIECES];
};

struct TBEntry_pawn {
  char *data;
  U64 key;
  U8 ready;
  U8 num;
  U8 symmetric;
  U8 has_pawns;
  U8 pawns[2];
  struct {
    struct PairsData *precomp[2];
    int factor[2][TBPIECES];
    U8 pieces[2][TBPIECES];
    U8 norm[2][TBPIECES];
  } file[4];
};

struct DTZEntry_piece {
  char *data;
  U64 key;
  U8 ready;
  U8 num;
  U8 symmetric;
  U8 has_pawns;
  U8 enc_type;
  struct PairsData *precomp;
  int factor[TBPIECES];
  U8 pieces[TBPIECES];
  U8 norm[TBPIECES];
  U64 mapped_size;
  U8 flags; // accurate, mapped, side
  U16 map_idx[4];
  U8 *map;
};

struct DTZEntry_pawn {
  char *data;
  U64 key;
  U8 ready;
  U8 num;
  U8 symmetric;
  U8 has_pawns;
  U8 pawns[2];
  struct {
    struct PairsData *precomp;
    int factor[TBPIECES];
    U8 pieces[TBPIECES];
    U8 norm[TBPIECES];
  } file[4];
  U64 mapped_size;
  U8 flags[4];
  U16 map_idx[4][4];
  U8 *map;
};

struct TBHashEntry {
  U64 key;
  struct TBEntry *ptr;
};

struct DTZTableEntry {
  U64 key1;
  U64 key2;
  struct TBEntry *entry;
};

const int wdl_to_map[5] = { 1, 3, 0, 2, 0 };
const U8 pa_flags[5] = { 8, 0, 0, 0, 4 };
const char pchr[] = {'K', 'Q', 'R', 'B', 'N', 'P'};

extern U64 encode_piece(struct TBEntry_piece *ptr, U8 *norm, int *pos, int *factor);
extern U8 decompress_pairs(struct PairsData *d, U64 index);
extern U64 encode_pawn(struct TBEntry_pawn *ptr, U8 *norm, int *pos, int *factor);
extern void free_dtz_entry(struct TBEntry *entry);
extern void load_dtz_table(char *str, U64 key1, U64 key2);
extern int pawn_file(struct TBEntry_pawn *ptr, int *pos);
extern int init_table_wdl(struct TBEntry *entry, char *str);

extern char TBdir[128];
extern char WDLdir[128];

#endif

