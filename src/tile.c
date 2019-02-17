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

#include "p3.h"
#include "common.h"

static int make_tile_index_1(int table, int index)
{
	index &= 0xff;
	switch (table) {
	case P3_CHR_TABLE_LEFT:
		return index;

	case P3_CHR_TABLE_RIGHT:
		return 256 + index;

	case P3_CHR_TABLE_BG:
		return p3_get_bg_chr_table() * 256 + index;

	case P3_CHR_TABLE_OBJ:
		return p3_get_obj_chr_table() * 256 + index;

	default:
		set_last_error("make_tile_index_1(): bad 'table' argument");
		return 0;
	}
}

static int make_tile_index_2(int table, int x, int y)
{
	switch (table) {
	case P3_CHR_TABLE_LEFT:
		return (y & 0x0f) * 16 + (x & 0x0f);

	case P3_CHR_TABLE_RIGHT:
		return 256 + (y & 0x0f) * 16 + (x & 0x0f);

	case P3_CHR_TABLE_PPU:
		return (y & 0x0f) * 32 + (x & 0x1f);

	case P3_CHR_TABLE_BG:
		return p3_get_bg_chr_table() * 256 + (y & 0x0f) * 16 + (x & 0x0f);

	case P3_CHR_TABLE_OBJ:
		return p3_get_obj_chr_table() * 256 + (y & 0x0f) * 16 + (x & 0x0f);

	default:
		set_last_error("make_tile_index_2(): bad 'table' argument");
		return 0;
	}
}

void p3_fill_tile(void *tile, int color)
{
	if (tile) {
		color &= 3;
		switch (color) {
		case P3_COLOR_0:
			memset(tile, 0x00, 16);
			break;

		case P3_COLOR_1:
			memset(tile, 0xFF, 8);
			memset((byte *) tile + 8, 0x00, 8);
			break;

		case P3_COLOR_2:
			memset(tile, 0x00, 8);
			memset((byte *) tile + 8, 0xFF, 8);
			break;

		case P3_COLOR_3:
			memset(tile, 0xFF, 16);
			break;
		}
	}
	set_last_error("p3_fill_tile(): bad 'tile' argument");
}

int p3_get_tile_pixel(void *tile, int x, int y)
{
	if (tile) {
		byte bit0, bit1;

		x &= 7;
		y &= 7;

		/* Get pixel bytes */
		bit0 = ((const byte *) tile)[y];
		bit1 = ((const byte *) tile)[y + 8];

		/* Compose pixel value */
		return (((bit1 >> (7 - x)) & 1) << 1) | ((bit0 >> (7 - x)) & 1);
	}
	set_last_error("p3_get_tile_pixel(): bad 'tile' argument");
	return 0;
}

void p3_set_tile_pixel(void *tile, int x, int y, int color)
{
	if (tile) {
		byte bit0, bit1;

		x &= 7;
		y &= 7;
		color &= 3;

		/* Get pixel bytes */
		bit0 = ((byte *) tile)[y];
		bit1 = ((byte *) tile)[y + 8];

		/* Reset pixel bits */
		bit0 &= ~((byte) 0x80 >> x);
		bit1 &= ~((byte) 0x80 >> x);

		/* Set new pixel bits */
		bit0 |= (color & 1) << (7 - x);
		bit1 |= (color >> 1) << (7 - x);

		/* Put back pixel bytes */
		((byte *) tile)[y] = bit0;
		((byte *) tile)[y + 8] = bit1;
	}
	set_last_error("p3_set_tile_pixel(): bad 'tile' argument");
}

int p3_is_tile_transparent(void *tile)
{
	if (tile) {
		const byte *bitmap = (const byte *) tile;
		int i;
		for (i = 0; i < 16; ++i, ++bitmap)
			if (*bitmap) {
				return FALSE;
			}

		/* No opaque pixels */
		return TRUE;
	}
	set_last_error("p3_is_tile_transparent(): bad 'tile' argument");
	return 0;
}

int p3_is_tile_opaque(void *tile) { return !p3_is_tile_has_alpha(tile); }

int p3_is_tile_has_alpha(void *tile)
{
	if (tile) {
		int i, j;
		byte row;
		for (i = 0; i < 8; ++i) {
			/* Get tile row transparency mask */
			row = ((const byte *) tile)[i] | ((const byte *) tile)[i + 8];
			j = 0;
			do {
				/* Pixel with mask bit == 0 is transparent */
				if (!(row & 1)) {
					return TRUE;
				}

				/* Shift to next pixel */
				row >>= 1;

				++j;
			} while (j < 8);
		}

		/* No transparent pixels */
		return FALSE;
	}
	set_last_error("p3_is_tile_has_alpha(): bad 'tile' argument");
	return 0;
}

void p3_copy_tile(void *dst, const void *src)
{
	if (dst && src) {
		if (dst != src) {
			memcpy(dst, src, 16);
		}
	} else {
		set_last_error("p3_copy_tile(): bad arguments");
	}
}

int p3_is_equal_tiles(const void *tile1, const void *tile2)
{
	if (tile1 && tile2) {
		if (tile1 != tile2) {
			return memcmp(tile1, tile2, 16) == 0;
		} else {
			return TRUE;
		}
	} else {
		set_last_error("p3_is_equal_tiles(): bad arguments");
	}
	return 0;
}

int p3_make_tile_index_1m(int index) { return p3_map_tile(index); }
int p3_make_tile_index_1t(int table, int index) { return make_tile_index_1(table, index); }
int p3_make_tile_index_1tm(int table, int index) { return p3_map_tile(p3_make_tile_index_1t(table, index)); }
int p3_make_tile_index_2(int x, int y) { return make_tile_index_2(P3_CHR_TABLE_PPU, x, y); }
int p3_make_tile_index_2m(int x, int y) { return p3_map_tile(p3_make_tile_index_2(x, y)); }
int p3_make_tile_index_2t(int table, int x, int y) { return make_tile_index_2(table, x, y); }
int p3_make_tile_index_2tm(int table, int x, int y) { return p3_map_tile(p3_make_tile_index_2t(table, x, y)); }
