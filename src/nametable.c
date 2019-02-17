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

void p3_fill_nametable(int page, int tile)
{
	page &= 3;
	tile &= 0xff;
	p3_set_address(page * 1024);
	memset(p3_get_v_pointer(), tile, 960);
}

void p3_zero_nametable(int page) { p3_fill_nametable(page, 0x00); }

void p3_fill_nametable_row_2(int page, int row, int tile, int start, int end)
{
	page &= 3;
	row &= 0x1f;
	if (row > 29) row -= 29;
	tile &= 0xff;
	start &= 0x1f;
	end &= 0x1f;
	if (start > end) {
		int tmp = start;
		start = end;
		end = start;
	}
	p3_set_increment(P3_INCREMENT_HORIZONTAL);
	p3_set_address(p3_make_name_address_2t(page, start, row));
	p3_fill(tile, end - start + 1);
}

void p3_fill_nametable_row(int page, int row, int tile) { p3_fill_nametable_row_2(page, row, tile, 0, 31); }

void p3_fill_nametable_column_2(int page, int col, int tile, int start, int end)
{
	page &= 3;
	col &= 0x1f;
	tile &= 0xff;
	start &= 0x1f;
	if (start > 29) start -= 29;
	end &= 0x1f;
	if (end > 29) end -= 29;
	if (start > end) {
		int tmp = start;
		start = end;
		end = start;
	}
	p3_set_increment(P3_INCREMENT_VERTICAL);
	p3_set_address(p3_make_name_address_2t(page, col, start));
	p3_fill(tile, end - start + 1);
}

void p3_fill_nametable_column(int page, int col, int tile) { p3_fill_nametable_column_2(page, col, tile, 0, 29); }

void p3_read_nametable(int page, void *buf)
{
	if (buf) {
		page &= 3;
		p3_set_address(page * 1024);
		memcpy(buf, p3_get_v_pointer(), 960);
	} else {
		set_last_error("p3_read_nametable(): bad 'buf' argument");
	}
}

void p3_write_nametable(int page, const void *data)
{
	if (data) {
		page &= 3;
		p3_set_address(page * 1024);
		memcpy(p3_get_v_pointer(), data, 960);
	} else {
		set_last_error("p3_write_nametable(): bad 'data' argument");
	}
}

int p3_make_name_address(int index)
{
	int y = index >> 6;
	index &= 0xfff;
	if (index > 3839) index -= 3839;
	return p3_make_name_address_2t((index >= (960 * 2) << 1) | ((index >> 5) & 1),
                               index & 31, y >= 30 ? y - 30 : y);
}

int p3_make_name_address_1t(int page, int index)
{
	page &= 3;
	index &= 0x3ff;
	if (index > 959) index -= 959;
	return (page << 10) | index;
}

int p3_make_name_address_2(int x, int y)
{
	x &= 0x3f;
	y &= 0x3f;
	if (y > 59) y -= 59;
	return p3_make_name_address_2t(((y >= 30) << 1) | (x >> 5), x & 31,
                               y >= 30 ? y - 30 : y);
}

int p3_make_name_address_2t(int page, int x, int y)
{
	page &= 3;
	x &= 0x1f;
	y &= 0x1f;
	if (y > 29) y -= 29;
	return (page << 10) | (y << 5) | x;
}

int p3_get_name(int index) { return p3_get_byte_at(p3_make_name_address(index)); }
void p3_put_name(int index, int tile) { p3_put_byte_at(p3_make_name_address(index), tile); }
int p3_get_name_1t(int page, int index) { return p3_get_byte_at(p3_make_name_address_1t(page, index)); }
void p3_put_name_1t(int page, int index, int tile) { p3_put_byte_at(p3_make_name_address_1t(page, index), tile); }
int p3_get_name_2(int x, int y) { return p3_get_byte_at(p3_make_name_address_2(x, y)); }
void p3_put_name_2(int x, int y, int tile){ p3_put_byte_at(p3_make_name_address_2(x, y), tile); }
int p3_get_name_2t(int page, int x, int y) { return p3_get_byte_at(p3_make_name_address_2t(page, x, y)); }
void p3_put_name_2t(int page, int x, int y, int tile){ p3_put_byte_at(p3_make_name_address_2t(page, x, y), tile); }
