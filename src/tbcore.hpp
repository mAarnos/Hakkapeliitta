/*
	Copyright (c) 2011-2013 Ronald de Man
*/

#ifndef TBCORE_H_
#define TBCORE_H_

#include <cstdint>
#include <string>

#define __WIN32__
// Define IS_64BIT when compiling for a 64-bit platform.
// 32-bit is only supported for 5-piece tables, because tables are mmap()ed
// into memory.
#define IS_64BIT

#ifndef __WIN32__
#include <pthread.h>
#define SEP_CHAR ':'
#define FD int
#define FD_ERR -1
#else
#include <windows.h>
#define SEP_CHAR ';'
#define FD HANDLE
#define FD_ERR INVALID_HANDLE_VALUE
#endif

#ifndef __WIN32__
#define LOCK_T pthread_mutex_t
#define LOCK_INIT(x) pthread_mutex_init(&(x), NULL)
#define LOCK(x) pthread_mutex_lock(&(x))
#define UNLOCK(x) pthread_mutex_unlock(&(x))
#else
#define LOCK_T HANDLE
#define LOCK_INIT(x) x = CreateMutex(NULL, FALSE, NULL)
#define LOCK(x) WaitForSingleObject(x, INFINITE)
#define UNLOCK(x) ReleaseMutex(x)
#endif
extern LOCK_T TB_mutex;

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
#define HSHMAX 5

#define Swap(a,b) {int tmp=a;a=b;b=tmp;}

#define TB_PAWN 1
#define TB_KNIGHT 2
#define TB_BISHOP 3
#define TB_ROOK 4
#define TB_QUEEN 5
#define TB_KING 6

#define TB_WPAWN TB_PAWN
#define TB_BPAWN (TB_PAWN | 8)

#define DTZ_ENTRIES 64

struct TBHashEntry;

extern struct TBHashEntry TB_hash[1 << TBHASHBITS][HSHMAX];
extern struct DTZTableEntry DTZ_table[DTZ_ENTRIES];

#ifdef IS_64BIT
typedef uint64_t base_t;
#else
typedef uint32_t base_t;
#endif

struct PairsData {
    char *indextable;
    uint16_t *sizetable;
    uint8_t *data;
    uint16_t *offset;
    uint8_t *symlen;
    uint8_t *sympat;
    int blocksize;
    int idxbits;
    int min_len;
    base_t base[1]; // C++ complains about base[]...
};

struct TBEntry {
    char *data;
    uint64_t key;
    uint64_t mapping;
    uint8_t ready;
    uint8_t num;
    uint8_t symmetric;
    uint8_t has_pawns;
};

struct TBEntry_piece {
    char *data;
    uint64_t key;
    uint64_t mapping;
    uint8_t ready;
    uint8_t num;
    uint8_t symmetric;
    uint8_t has_pawns;
    uint8_t enc_type;
    struct PairsData *precomp[2];
    int factor[2][TBPIECES];
    uint8_t pieces[2][TBPIECES];
    uint8_t norm[2][TBPIECES];
};

struct TBEntry_pawn {
    char *data;
    uint64_t key;
    uint64_t mapping;
    uint8_t ready;
    uint8_t num;
    uint8_t symmetric;
    uint8_t has_pawns;
    uint8_t pawns[2];
    struct {
        struct PairsData *precomp[2];
        int factor[2][TBPIECES];
        uint8_t pieces[2][TBPIECES];
        uint8_t norm[2][TBPIECES];
    } file[4];
};

struct DTZEntry_piece {
    char *data;
    uint64_t key;
    uint64_t mapping;
    uint8_t ready;
    uint8_t num;
    uint8_t symmetric;
    uint8_t has_pawns;
    uint8_t enc_type;
    struct PairsData *precomp;
    int factor[TBPIECES];
    uint8_t pieces[TBPIECES];
    uint8_t norm[TBPIECES];
    uint8_t flags; // accurate, mapped, side
    uint16_t map_idx[4];
    uint8_t *map;
};

struct DTZEntry_pawn {
    char *data;
    uint64_t key;
    uint64_t mapping;
    uint8_t ready;
    uint8_t num;
    uint8_t symmetric;
    uint8_t has_pawns;
    uint8_t pawns[2];
    struct {
        struct PairsData *precomp;
        int factor[TBPIECES];
        uint8_t pieces[TBPIECES];
        uint8_t norm[TBPIECES];
    } file[4];
    uint8_t flags[4];
    uint16_t map_idx[4][4];
    uint8_t *map;
};

struct TBHashEntry {
    uint64_t key;
    struct TBEntry *ptr;
};

struct DTZTableEntry {
    uint64_t key1;
    uint64_t key2;
    struct TBEntry *entry;
};

inline uint32_t bswap32(uint32_t x)
{
    return  ((x << 24) & 0xff000000) |
            ((x << 8) & 0x00ff0000) |
            ((x >> 8) & 0x0000ff00) |
            ((x >> 24) & 0x000000ff);
}

inline uint64_t bswap64(uint64_t x)
{
    uint32_t tl, th;
    th = bswap32((uint32_t)(x & 0xffffffffULL));
    tl = bswap32((uint32_t)((x >> 32) & 0xffffffffULL));
    return ((uint64_t)th << 32) | tl;
}

extern int TBLargest;

const int wdl_to_map[5] = { 1, 3, 0, 2, 0 };
const uint8_t pa_flags[5] = { 8, 0, 0, 0, 4 };
const char pchr[] = { 'K', 'Q', 'R', 'B', 'N', 'P' };

uint64_t encode_piece(struct TBEntry_piece *ptr, uint8_t *norm, int *pos, int *factor);
uint8_t decompress_pairs(struct PairsData *d, uint64_t index);
uint64_t encode_pawn(struct TBEntry_pawn *ptr, uint8_t *norm, int *pos, int *factor);
void free_dtz_entry(struct TBEntry *entry);
void load_dtz_table(char *str, uint64_t key1, uint64_t key2);
int pawn_file(struct TBEntry_pawn *ptr, int *pos);
int init_table_wdl(struct TBEntry *entry, char *str);

void init(const std::string & path);

#endif

