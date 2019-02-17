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

/* Used in p3_set_attribute_byte_item */
static const byte attribute_reset_mask[4] = {
	(byte) ~3,                              /* P3_ATTRIBUTE_TOP_LEFT */
	(byte) ~(3 << 2),                       /* P3_ATTRIBUTE_TOP_RIGHT */
	(byte) ~(3 << 4),                       /* P3_ATTRIBUTE_BOTTOM_LEFT */
	(byte) ~(3 << 6)                        /* P3_ATTRIBUTE_BOTTOM_RIGHT */
};

void p3_fill_attribute_table(int page, int pal)
{
	page &= 3;
	pal &= 3;
	p3_set_address(page * 1024 + 960);
	memset(p3_get_v_pointer(), p3_make_flat_attribute_byte(pal), 64);
}

void p3_zero_attribute_table(int page) { p3_fill_attribute_table(page, P3_PALETTE_0); }

void p3_fill_attribute_table_row_2(int page, int row, int pal, int start, int end)
{
	page &= 3;
	row &= 15;
	start &= 15;
	end &= 15;
	if (start > end) {
		int tmp = start;
		start = end;
		end = tmp;
	}
	for (; start <= end; ++start) {
		p3_put_attribute_2t(page, start, row, pal);
	}
}

void p3_fill_attribute_table_row(int page, int row, int pal)
{
	p3_fill_attribute_table_row_2(page, row, pal, 0, 15);
}

void p3_fill_attribute_table_column_2(int page, int col, int pal, int start, int end)
{
	page &= 3;
	col &= 15;
	start &= 15;
	end &= 15;
	if (start > end) {
		int tmp = start;
		start = end;
		end = tmp;
	}
	for (; start <= end; ++start) {
		p3_put_attribute_2t(page, col, start, pal);
	}
}

void p3_fill_attribute_table_column(int page, int col, int pal)
{
	p3_fill_attribute_table_column_2(page, col, pal, 0, 15);
}

void p3_read_attribute_table(int page, void *buf)
{
	if (buf) {
		page &= 3;
		p3_set_address(page * 1024 + 960);
		memcpy(buf, p3_get_v_pointer(), 64);
	} else {
		set_last_error("p3_read_attribute_table(): bad 'buf' argument");
	}
}

void p3_write_attribute_table(int page, const void *data)
{
	if (data) {
		page &= 3;
		p3_set_address(page * 1024 + 960);
		memcpy(p3_get_v_pointer(), data, 64);
	} else {
		set_last_error("p3_write_attribute_table(): bad 'data' argument");
	}
}

int p3_get_attribute_position(void) { return S(last_attribute_pos); }

int p3_make_attribute_address(int index)
{
	index &= 0x3ff;
	return p3_make_attribute_address_2t(((index >> 8) & 2) | ((index >> 4) & 1),
                                    index & 15, (index >> 5) & 15);
}

int p3_make_attribute_address_1t(int page, int index)
{
	index &= 0xff;
	return p3_make_attribute_address_2t(page, index & 15, index >> 4);
}

int p3_make_attribute_address_2(int x, int y)
{
	x &= 0x1f;
	y &= 0x1f;
	return p3_make_attribute_address_2t(((y >> 3) & 2) | (x >> 4), x & 15, y & 15);
}

int p3_make_attribute_address_2t(int page, int x, int y)
{
	page &= 3;
	x &= 0x0f;
	y &= 0x0f;
	S(last_attribute_pos) = ((y & 1) << 1) | (x & 1);
	return (page << 10) | 960 | ((y << 2) & 0x38) | (x >> 1);
}

int p3_get_attribute(int index)
{
	return p3_get_attribute_byte_item_x(p3_get_byte_at(p3_make_attribute_address(index)));
}

void p3_put_attribute(int index, int pal)
{
	int inc = p3_get_increment();
	p3_set_increment(P3_INCREMENT_NONE);
	p3_put_byte(p3_set_attribute_byte_item_x(p3_get_byte_at(p3_make_attribute_address(index)), pal));
	p3_set_increment(inc);
}

int p3_get_attribute_1t(int page, int index)
{
	return p3_get_attribute_byte_item_x(p3_get_byte_at(p3_make_attribute_address_1t(page, index)));
}

void p3_put_attribute_1t(int page, int index, int pal)
{
	int inc = p3_get_increment();
	p3_set_increment(P3_INCREMENT_NONE);
	p3_put_byte(p3_set_attribute_byte_item_x(p3_get_byte_at(p3_make_attribute_address_1t(page, index)), pal));
	p3_set_increment(inc);
}

int p3_get_attribute_2(int x, int y)
{
	return p3_get_attribute_byte_item_x(p3_get_byte_at(p3_make_attribute_address_2(x, y)));
}

void p3_put_attribute_2(int x, int y, int pal)
{
	int inc = p3_get_increment();
	p3_set_increment(P3_INCREMENT_NONE);
	p3_put_byte(p3_set_attribute_byte_item_x(p3_get_byte_at(p3_make_attribute_address_2(x, y)), pal));
	p3_set_increment(inc);
}

int p3_get_attribute_2t(int page, int x, int y)
{
	return p3_get_attribute_byte_item_x(p3_get_byte_at(p3_make_attribute_address_2t(page, x, y)));
}

void p3_put_attribute_2t(int page, int x, int y, int pal)
{
	int inc = p3_get_increment();
	p3_set_increment(P3_INCREMENT_NONE);
	p3_put_byte(p3_set_attribute_byte_item_x(p3_get_byte_at(p3_make_attribute_address_2t(page, x, y)), pal));
	p3_set_increment(inc);
}

int p3_make_flat_attribute_byte(int pal)
{
	pal &= 3;
	return (pal << 6) | (pal << 4) | (pal << 2) | pal;
}

int p3_make_attribute_byte(int tl, int tr, int bl, int br)
{
	tl &= 3;
	tr &= 3;
	bl &= 3;
	br &= 3;
	return (br << 6) | (bl << 4) | (tr << 2) | tl;
}

int p3_get_attribute_byte_item(int attr, int pos)
{
	pos &= 3;
	return (attr >> (pos << 1)) & 3;
}

int p3_set_attribute_byte_item(int attr, int pos, int pal)
{
	pos &= 3;
	pal &= 3;
	return (attr & attribute_reset_mask[pos]) | (pal << (pos << 1));
}

int p3_get_attribute_byte_item_x(int attr)
{
	return p3_get_attribute_byte_item(attr, S(last_attribute_pos));
}

int p3_set_attribute_byte_item_x(int attr, int pal)
{
	return p3_set_attribute_byte_item(attr, S(last_attribute_pos), pal);
}
