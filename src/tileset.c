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

USE_P3_OBJECT;

/* Internal interface of module */
BOOL g_initialize_tileset(void *tiles, int size);

/* mapper.c module */
void g_check_banks();

BOOL g_initialize_tileset(void *chr, int chr_size)
{
	if (chr && ((chr_size == P3_CHR_SIZE_8) ||
               (chr_size == P3_CHR_SIZE_16) ||
               (chr_size == P3_CHR_SIZE_32) ||
               (chr_size == P3_CHR_SIZE_64) ||
               (chr_size == P3_CHR_SIZE_128) ||
               (chr_size == P3_CHR_SIZE_256) ||
               (chr_size == P3_CHR_SIZE_512) ||
               (chr_size == P3_CHR_SIZE_1024)))
	{
		S(tileset_pointer) = (byte*) chr;
		S(tileset_size) = chr_size;
		return TRUE;
	}
	set_last_error("p3_set_chr_ptr(): bad arguments");
	return FALSE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void *p3_get_chr_ptr(void) { return S(tileset_pointer); }

void p3_set_chr_ptr(void *chr, int chr_size)
{
	if (g_initialize_tileset(chr, chr_size))
		g_check_banks();
}

int p3_get_chr_size(void) { return S(tileset_size); }
int p3_get_tile_count(void) { return S(tileset_size) >> 4; }

static byte bad_tile[16] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
	0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};

void *p3_get_tile(int index)
{
	if ((index >= 0) && (index < p3_get_tile_count()))
		return &S(tileset_pointer[index << 4]);

	set_last_error("p3_get_tile(): 'index' out of range");
	return bad_tile;
}

void p3_put_tile(int index, const void *tile)
{
	if (tile)
		p3_copy_tile(p3_get_tile(index), tile);
	else
		set_last_error("p3_put_tile(): bad 'tile' argument");
}

void p3_copy_tiles(int dst, int src, int num, int b_mapdst, int b_mapsrc)
{
	int i;

	dst &= 0xffff;
	src &= 0xffff;
	num &= 0xffff;
	for (i = 0; i < num; ++i, ++src, ++dst) {
		p3_copy_tile(p3_get_tile(b_mapdst ? p3_map_tile(dst) : dst),
			  p3_get_tile(b_mapsrc ? p3_map_tile(src) : src));
	}
}

void p3_read_tiles(void *buf, int start, int num, int b_usemmc)
{
	if (buf) {
		int i;
		byte *dst = (byte *) buf;

		start &= 0xffff;
		num &= 0xffff;
		for (i = 0; i < num; ++i, ++start, dst += 16) {
			p3_copy_tile(dst, p3_get_tile(b_usemmc ? p3_map_tile(start) : start));
		}
	} else {
		set_last_error("p3_read_tiles(): bad 'buf' argument");
	}
}

void p3_write_tiles(const void *tiles, int start, int num, int b_usemmc)
{
	if (tiles) {
		int i;
		const byte *src = (const byte *) tiles;

		start &= 0xffff;
		num &= 0xffff;
		for (i = 0; i < num; ++i, ++start, src += 16) {
			p3_copy_tile(p3_get_tile(b_usemmc ? p3_map_tile(start) : start), src);
		}
	} else {
		set_last_error("p3_write_tiles(): bad 'tiles' argument");
	}
}
