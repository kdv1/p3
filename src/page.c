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

void p3_zero_page(int page)
{
	p3_zero_nametable(page);
	p3_zero_attribute_table(page);
}

void p3_fill_page(int page, int tile, int pal)
{
	p3_fill_nametable(page, tile);
	p3_fill_attribute_table(page, pal);
}

void p3_read_page(int page, void *buf)
{
	p3_read_nametable(page, buf);
	p3_read_attribute_table(page,  (byte*) buf + 960);
}

void p3_write_page(int page, const void *data)
{
	p3_write_nametable(page, data);
	p3_write_attribute_table(page, (const byte*) data + 960);
}
