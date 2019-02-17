/*
 Copyright (C) 2019 Dmitry Korunos

 This software is provided 'as-is', without any express or implied
 warranty. In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Type of build macro */
#ifdef NDEBUG
	#define P3_RELEASE
#else
	#define P3_DEBUG
#endif

#define P3_CHECKED

/* Keep string only if P3_CHECKED defined */
#if defined(P3_CHECKED)
	#define CHECK_LINE(L) L
#else
	#define CHECK_LINE(L)
#endif

/* Include assert.h depend of P3_CHECKED */
#if defined(P3_CHECKED) && defined(P3_RELEASE)
	#undef NDEBUG
	#include <assert.h>
	#define NDEBUG 1
#else
	#include <assert.h>
#endif

/* Alignment related macro */
#if(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
	/* C11 has alignas specifier and alignof operator */
	#include <stdalign.h>
#elif defined(_MSC_VER)
	#define alignas(N) __declspec(align(N))
	#define __alignas_is_defined
#elif defined(__GNUC__)
	#define alignas(N) __attribute__((aligned(N)))
	#define __alignas_is_defined
#endif

#ifdef __alignas_is_defined
	#define cache_aligned alignas(64)
#else
	#define cache_aligned
#endif

/* inline specifier */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
	/* C99 compiler has inline specifier */
#elif defined(_MSC_VER) || defined(__GNUC__)
	#define inline __inline
#else
	#error Unknown compiler
#endif

/* forceinline specifier */
#ifdef P3_RELEASE
	#if defined(_MSC_VER)
		#define forceinline __forceinline
	#elif defined(__GNUC__)
		#define forceinline __attribute__((always_inline)) inline
	#else
		#define forceinline inline
	#endif
#else
	#define forceinline
#endif

/* Exact-width integer C types */
#if defined(__GNUC__) || (defined(_MSC_VER) && (_MSC_VER >= 1600)) ||\
	(defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
	/* GCC, VS2010 and later, C99 */
	#include <stdint.h>
#elif defined(_MSC_VER) && (_MSC_VER < 1600)
	/* VS2008 and earlier */
	typedef __int8  int8_t;
	typedef __int16 int16_t;
	typedef __int32 int32_t;
	typedef __int64 int64_t;
	typedef unsigned __int8  uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;
#endif

/* Boolean */
typedef int8_t BOOL;
#define FALSE 0
#define TRUE  1
#define TO_BOOL(V) ((V)!=0)

/* Basic NES data type */
typedef uint8_t byte;

/* Type for holding offset of byte in PPU tileset, valid range is 0..8191
   This type is also used to hold byte offset in nametables and attribute tables */
typedef uint16_t padr_t;

/* Type for holding offset of byte in tileset */
typedef uint32_t cadr_t;

typedef unsigned int sprite_index_t;

#define SCREEN_WIDTH                        256
#define SCREEN_HEIGHT                       240

#define OBJ_MAX                             64
#define OBJ_MAX_PER_SCANLINE                8

#define DEFAULT_SPRITE                      {0, 255, P3_PALETTE_0, P3_SPRITE_FRONT, FALSE, FALSE}

struct mmc_table_state {
	int table;
	int *mode;
	cadr_t *banks;
	const byte **shift;
	const padr_t **mask;
	const byte **group;
};

/* Sprite bitmap row on scanline */
struct sprite_unit {
	byte attribute;
	byte chr_lo;
	byte chr_hi;
	byte priority;
	byte x;
};

struct p3_object {
	/* tileset.c */
	byte *tileset_pointer;
	int tileset_size;

	/* mapper.c */
	int glob_mmc_mode;
	cadr_t bank_8x1;
	/* left table */
	int left_table_mode;
	cadr_t left_table_banks[4];
	const byte *left_shift_lut;
	const padr_t *left_mask_lut;
	const byte *left_group_lut;
	/* right table */
	int right_table_mode;
	cadr_t right_table_banks[4];
	const byte *right_shift_lut;
	const padr_t *right_mask_lut;
	const byte *right_group_lut;
	/* pointers to tables */
	struct mmc_table_state mmc_tables[2];
	/* switchable mapping functions */
	cadr_t (*main_map_func)(padr_t);
	cadr_t (*bg_map_func)(padr_t);
	cadr_t (*obj_map_func)(padr_t);
	/* mirroring */
	int mirroring_type;
	byte mirroring_lut[4];
	byte(*mirroring_function)(byte);

	/* p3.c */
	int bg_pattern_table;
	int obj_pattern_table;
	byte page_memory[4096];
	P3_SPRITE obj_memory[OBJ_MAX];
	BOOL idle;
	BOOL enabled;
	BOOL show_bg;
	BOOL show_obj;
	BOOL clip_bg;
	BOOL clip_obj;
	BOOL fix_obj_y;
	BOOL callback_enabled;
	BOOL obj_overflow;
	sprite_index_t sprite_count;
	byte tmp_tpg, tmp_tcx, tmp_tcy, tmp_tfx, tmp_tfy;
	byte tmp_vpg, tmp_vcx, tmp_vcy, tmp_vfx, tmp_vfy;
	int increment_size;
	byte obj_mode;
	padr_t bg_chr_base;
	padr_t obj_chr_base;
	/* temp variables */
	byte cache_aligned bg_color_index;
	byte bg_tile_hi, bg_tile_lo;
	byte bg_tile_attributes;
	byte bg_show_mask;
	uint32_t bg_clip_mask;
	/* temp variables */
	byte obj_color_index;
	byte obj_show_mask;
	uint32_t obj_clip_mask;
	/* registers */
	byte vpg, vcx, vcy, vfx, vfy;
	byte tpg, tcx, tcy, tfx, tfy;
	/* color */
	uint16_t tint_value;
	byte palette_memory[32];
	byte grayscale_mask;
	byte *bg_palette;
	byte *obj_palette;
	/* frame */
	size_t frame_pos;
	byte frame_row;
	uint16_t frame_row_pos;
	uint16_t frame_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	/* sprites */
	struct sprite_unit sprite_units[OBJ_MAX_PER_SCANLINE];
	byte sprite_buffer[SCREEN_WIDTH];
	/* render callback state */
	P3_CALLBACK callback_proc;
	int callback_type;
	byte callback_x;
	byte callback_y;
	void *callback_param;

	/* attribute_table.c */
	byte last_attribute_pos;
};

/* Access to global state */
#define DEFINE_P3_OBJECT                    P3_OBJECT *g_p3obj = NULL
#define USE_P3_OBJECT                       extern P3_OBJECT *g_p3obj
#define S(N)                                (g_p3obj->N)

void set_last_error(const char *err);
