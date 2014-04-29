/*
  Copyright (c) 2011-2013 Ronald de Man
*/

#ifndef TBCORE_H_
#define TBCORE_H_

#include <cstdint>

// Comment the following line out to get a version for a non-windows os.
#define __WIN32__
// Comment the following line out to get a 32-bit version.
#define DECOMP64

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
#define LOCK_INIT(x) do { x = CreateMutex(NULL, FALSE, NULL); } while (0)
#define LOCK(x) WaitForSingleObject(x, INFINITE)
#define UNLOCK(x) ReleaseMutex(x)
#endif

#define WDLSUFFIX ".rtbw"
#define DTZSUFFIX ".rtbz"
#define WDLDIR "RTBWDIR"
#define DTZDIR "RTBZDIR"
#define TBPIECES 6

#define WDL_MAGIC 0x5d23e871
#define DTZ_MAGIC 0xa50c66d7

#define TBHASHBITS 10

struct TBHashEntry;

#ifdef DECOMP64
typedef uint64_t base_t;
#else
typedef uint32_t base_t;
#endif

struct PairsData {
	char * indextable;
	uint16_t * sizetable;
	uint8_t * data;
	uint16_t * offset;
	uint8_t * symlen;
	uint8_t * sympat;
	int blocksize;
	int idxbits;
	int min_len;
	base_t base[1]; // C++ complains about base[]...
};

struct TBEntry {
	char * data;
	uint64_t key;
	uint64_t mapping;
	uint8_t ready;
	uint8_t num;
	uint8_t symmetric;
	uint8_t has_pawns;
};

struct TBEntry_piece {
	char * data;
	uint64_t key;
	uint64_t mapping;
	uint8_t ready;
	uint8_t num;
	uint8_t symmetric;
	uint8_t has_pawns;
	uint8_t enc_type;
	struct PairsData * precomp[2];
	int factor[2][TBPIECES];
	uint8_t pieces[2][TBPIECES];
	uint8_t norm[2][TBPIECES];
};

struct TBEntry_pawn {
	char * data;
	uint64_t key;
	uint64_t mapping;
	uint8_t ready;
	uint8_t num;
	uint8_t symmetric;
	uint8_t has_pawns;
	uint8_t pawns[2];
	struct {
		struct PairsData * precomp[2];
		int factor[2][TBPIECES];
		uint8_t pieces[2][TBPIECES];
		uint8_t norm[2][TBPIECES];
	} file[4];
};

struct DTZEntry_piece {
	char * data;
	uint64_t key;
	uint64_t mapping;
	uint8_t ready;
	uint8_t num;
	uint8_t symmetric;
	uint8_t has_pawns;
	uint8_t enc_type;
	struct PairsData * precomp;
	int factor[TBPIECES];
	uint8_t pieces[TBPIECES];
	uint8_t norm[TBPIECES];
	uint8_t flags; // accurate, mapped, side
	uint16_t map_idx[4];
	uint8_t * map;
};

struct DTZEntry_pawn {
	char * data;
	uint64_t key;
	uint64_t mapping;
	uint8_t ready;
	uint8_t num;
	uint8_t symmetric;
	uint8_t has_pawns;
	uint8_t pawns[2];
	struct {
		struct PairsData * precomp;
		int factor[TBPIECES];
		uint8_t pieces[TBPIECES];
		uint8_t norm[TBPIECES];
	} file[4];
	uint8_t flags[4];
	uint16_t map_idx[4][4];
	uint8_t * map;
};

struct TBHashEntry {
	uint64_t key;
	struct TBEntry * ptr;
};

struct DTZTableEntry {
	uint64_t key1;
	uint64_t key2;
	struct TBEntry * entry;
};

#endif

