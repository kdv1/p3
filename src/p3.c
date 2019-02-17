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

DEFINE_P3_OBJECT;

/* tileset.c module */
BOOL g_initialize_tileset(void *chr, int chr_size);

/* mapper.c module */
void g_initialize_mapper(void);
void g_update_mapper_fn(void);
byte *g_get_tile(padr_t);
byte *g_get_bg_tile(padr_t);
byte *g_get_obj_tile(padr_t);

/* Map byte 76543210 to 01234567 */
static const byte byte_reverse_lut[256] = {
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

/* Internal flag for p3_reset() */
#define P3_RESET_BUFS (1 << 9)

/* Grayscale bit mask */
#define GRAYSCALE_MASK_ON  0x30
#define GRAYSCALE_MASK_OFF 0x3F


static const P3_SPRITE default_sprite = DEFAULT_SPRITE;
static void default_callback(int x, int y, void *param) {}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static forceinline void move_to_next_tile(void)
{
	/* Wrap horizontally */
	if (S(vcx) == 31) {
		S(vcx) = 0;
		/* Switch horizontal nametable */
		S(vpg) ^= 1;
	} else {
		++S(vcx);
	}
}

static forceinline void move_to_next_row(void)
{
	if (S(vfy) < 7) {
		++S(vfy);
	} else {
		S(vfy) = 0;
		/* Wrap vertically */
		if (S(vcy) == 29) {
			S(vcy) = 0;
			/* Switch vertical nametable */
			S(vpg) ^= 2;
		} else if (S(vcy) == 31) {
			S(vcy) = 0;
			/* Note: no nametable switch */
		} else {
			/* Goto next tile row */
			++S(vcy);
		}
	}
}

static forceinline void increment_v_register(void)
{
	if (S(increment_size)) {
		p3_set_address((p3_get_address() + S(increment_size)) & 0x0FFF);
	}
}

static forceinline padr_t make_current_address(void)
{
	return ((padr_t) S(mirroring_function)(S(vpg)) << 10) |
		(S(vcy) << 5) | S(vcx);
}

static forceinline byte get_nametable_byte(void)
{
	return S(page_memory)[((padr_t) S(mirroring_function)(S(vpg)) << 10) |
		(S(vcy) << 5) | S(vcx)];
}

static forceinline byte get_attribute_byte(void)
{
	return S(page_memory)[((padr_t) S(mirroring_function)(S(vpg)) << 10) |
	                960 | ((S(vcx) >> 2) | (S(vcy) & 0x1c) << 1)];
}

static void init_sprite_unit(int i, byte chr_lo, byte chr_hi, const P3_SPRITE *sprite)
{
	struct sprite_unit unit;
	unit.attribute = sprite->palette << 2;
	/* Horizontal flipping (reversed, see overlay_sprite_unit) */
	if (sprite->flip_horizontal) {
		unit.chr_lo = chr_lo;
		unit.chr_hi = chr_hi;
	} else {
		unit.chr_lo = byte_reverse_lut[chr_lo];
		unit.chr_hi = byte_reverse_lut[chr_hi];
	}
	unit.priority = sprite->priority ? 0 : 0x80;
	unit.x = sprite->x;
	S(sprite_units)[i] = unit;
}

static void overlay_sprite_unit(int i)
{
	/* Destination memory */
	struct sprite_unit unit = S(sprite_units)[i];
	byte *dst = S(sprite_buffer) + unit.x;
	unsigned int j;

	/* Clip row length if sprite cross right screen border */
	unsigned int write_amount = 8;
	if (unit.x > (SCREEN_WIDTH - 8)) {
		write_amount = SCREEN_WIDTH - unit.x;
	}

	/* Render to buffer */
	for (j = 0; j < write_amount; ++j, ++dst) {
		/* Write if destination pixel transparent */
		if (!((*dst) & 3)) {
			*dst = unit.priority | 16 | unit.attribute | (unit.chr_lo & 1) | ((
			                        unit.chr_hi & 1) << 1);
		}
		unit.chr_lo >>= 1;
		unit.chr_hi >>= 1;
	}
}


static void evaluate_sprites(void)
{
	unsigned int range;         /* note: must be unsigned */
	sprite_index_t sprite_index;
	byte sprite_y;
	P3_SPRITE *sprite;
	byte *tile = NULL;

	S(sprite_count) = 0;
	for (sprite_index = 0; sprite_index < OBJ_MAX; ++sprite_index) {
		sprite_y = S(obj_memory)[sprite_index].y;
		if (S(fix_obj_y)) {
			--sprite_y;
		}
		range = S(frame_row) - sprite_y;
		/* If sprite on current scanline ? */
		if (range < S(obj_mode)) {
			sprite = &S(obj_memory)[sprite_index];
			switch (S(obj_mode)) {
			case P3_OBJ_MODE_8X8:
				if (sprite->flip_vertical)
					tile = g_get_obj_tile(S(obj_chr_base) |
					                       (sprite->tile << 4) |
					                       (7 - range));
				else
					tile = g_get_obj_tile(S(obj_chr_base) |
					                       (sprite->tile << 4) |
					                       range);
				break;

			case P3_OBJ_MODE_8X16:
				if (sprite->flip_vertical)
					tile = g_get_tile(
					               (((padr_t) sprite->tile & 1) << 12) |
					               ((sprite->tile & 0xFE) << 4) |
					               (((range & 8) ^ 8) << 1) | (7 - (range & 7)));
				else
					tile = g_get_tile(
					               (((padr_t) sprite->tile & 1) << 12) |
					               ((sprite->tile & 0xFE) << 4) |
					               ((range & 8) << 1) | (range & 7));
				break;
			}
			/* Initialize sprite unit */
			init_sprite_unit(S(sprite_count), *tile, *(tile + 8), sprite);

			/* Check sprite limit */
			if ((++S(sprite_count)) >= OBJ_MAX_PER_SCANLINE) {
				break;
			}
		}
	}
	/* Find for sprite overflow */
	for (; sprite_index < OBJ_MAX; ++sprite_index) {
		sprite_y = S(obj_memory)[sprite_index].y;
		if (S(fix_obj_y)) {
			--sprite_y;
		}
		range = S(frame_row) - sprite_y;
		/* If sprite on current scanline ? */
		if (range < S(obj_mode)) {
			S(obj_overflow) = TRUE;
			return;
		}
	}
}

static void render_sprite_buffer(void)
{
	sprite_index_t i;

	/* Skip last row */
	if (S(frame_row) == 239) {
		return;
	}

	/* Initialize sprite units */
	evaluate_sprites();

	/* Clear buffer */
	memset(S(sprite_buffer), 0, sizeof(S(sprite_buffer)));

	/* Overlay sprite units to buffer */
	for (i = 0; i < S(sprite_count); ++i) {
		overlay_sprite_unit(i);
	}
}

static forceinline void refetch_background_tile(void)
{
	/* Get background tile with row offset */
	byte *tile = g_get_bg_tile(S(bg_chr_base) | (get_nametable_byte() << 4) |
	                                 S(vfy));
	S(bg_tile_lo) = *tile;
	S(bg_tile_hi) = *(tile + 8);
	S(bg_tile_attributes) = (get_attribute_byte() >> ((S(vcx) & 2) | ((S(vcy) & 2) << 1))) & 3;
	S(bg_tile_attributes) <<= 2;
}

static void fetch_tile(void)
{
	if (S(frame_row_pos) != 256) {
		refetch_background_tile();
		move_to_next_tile();
	}
}

static forceinline void write_pixel(void)
{
	/* Get background tile color index */
	S(bg_color_index) = ((S(bg_tile_lo) >> (7 - S(vfx))) & 1) |
		(((S(bg_tile_hi) >> (7 - S(vfx))) << 1) & 2);
	S(bg_color_index) = (S(bg_tile_attributes) | S(bg_color_index)) & 
		~S(bg_clip_mask) & S(bg_show_mask);

	/* Get sprite tile color index + sprite priority in bit 7 */
	S(obj_color_index) = S(sprite_buffer)[S(frame_row_pos)] &
		~S(obj_clip_mask) & S(obj_show_mask);


	/* Mix color indexes using next rule:
	   If (foreground sprite or transparent background tile)
	   and opaque sprite tile, write sprite pixel, else write background tile pixel */
	S(frame_buffer)[S(frame_pos)] = S(tint_value) |
	    S(palette_memory)[(((S(obj_color_index) & 0x80) || !(S(bg_color_index) & 3)) && (S(obj_color_index) & 3)) ?
	                              S(obj_color_index) & 0x1F : S(bg_color_index)] & S(grayscale_mask);

	++S(frame_pos);
	++S(frame_row_pos);        /* Note: increment here, see fetch_tile */

	/* Goto next pixel */
	if (++S(vfx) & 8) {
		fetch_tile();
		S(vfx) = 0;
	}

	/* Make clip mask for next pixel */
	S(bg_clip_mask) >>= 4;
	S(obj_clip_mask) >>= 4;
}

static void render_frame(void)
{
	int t;

	S(frame_pos) = 0;
	S(obj_overflow) = FALSE;
	/* Clear sprite render buffer for first scanline */
	memset(S(sprite_buffer), 0, sizeof(S(sprite_buffer)));
	p3_update_register(P3_REGISTER_V);
	/* Render each scanline */
	for (S(frame_row) = 0; S(frame_row) < SCREEN_HEIGHT; ++S(frame_row)) {
		S(frame_row_pos) = 0;

		S(bg_show_mask) = S(show_bg) ? 0xFF : 0x00;
		S(obj_show_mask) = S(show_obj) ? 0xFF : 0x00;
		S(bg_clip_mask) = S(clip_bg) ? 0xFFFFFFFF : 0x00000000;
		S(obj_clip_mask) = S(clip_obj) ? 0xFFFFFFFF : 0x00000000;

		/* Fetch data for first tile */
		fetch_tile();

		/* Render eight pixels per tile */
		for (t = 0; t < 32; ++t) {
			write_pixel();
			write_pixel();
			write_pixel();
			write_pixel();
			write_pixel();
			write_pixel();
			write_pixel();
			write_pixel();
		}

		move_to_next_row();

		/* Restore horizontal nametable from register t */
		S(vpg) &= 2;
		S(vpg) |= (S(tpg) & 1);

		/* Restore horizontal position from register t */
		S(vcx) = S(tcx);
		S(vfx) = S(tfx);

		/* Render sprites for next scanline */
		render_sprite_buffer();
	}

	/* Restore v register from register t */
	S(vpg) = S(tpg);
	S(vcy) = S(tcy);
	S(vfy) = S(tfy);
}

static forceinline void write_pixel_cb(void)
{
	/* Pixel render event */
	switch (S(callback_type)) {
	case P3_CALLBACK_PIXEL:
		S(callback_proc)(S(frame_row_pos), S(frame_row), S(callback_param));
		break;

	case P3_CALLBACK_ONCE:
		if ((S(frame_row_pos) == S(callback_x)) && (S(frame_row) == S(callback_y))) {
			S(callback_proc)(S(frame_row_pos), S(frame_row), S(callback_param));
		}
		break;
	}

	/* Get background tile color index */
	S(bg_color_index) = S(bg_tile_attributes) | ((S(bg_tile_lo) >> (7 - S(vfx))) & 1) |
		(((S(bg_tile_hi) >> (7 - S(vfx))) << 1) & 2);

	S(bg_color_index) = S(bg_color_index) & ~S(bg_clip_mask) & S(bg_show_mask);

	/* Get sprite tile color index + sprite priority in bit 7 */
	S(obj_color_index) = S(sprite_buffer)[S(frame_row_pos)] &
		~S(obj_clip_mask) & S(obj_show_mask);


	/* Mix color indexes using next rule:
	   If (foreground sprite or transparent background tile)
	   and opaque sprite tile, write sprite pixel, else write background tile pixel */
	S(frame_buffer)[S(frame_pos)] = S(tint_value) |
		S(palette_memory)[(((S(obj_color_index) & 0x80) || !(S(bg_color_index) & 3)) &&
		    (S(obj_color_index) & 3)) ? S(obj_color_index) & 0x1F :
			    S(bg_color_index)] & S(grayscale_mask);

	++S(frame_pos);
	++S(frame_row_pos);        /* Note: increment here, see fetch_tile */

	/* Goto next pixel */
	if (++S(vfx) & 8) {
		fetch_tile();
		S(vfx) = 0;
	}

	/* Make clip mask for next pixel */
	S(bg_clip_mask) >>= 4;
	S(obj_clip_mask) >>= 4;
}

static void render_frame_cb(void)
{
	int t;

	S(frame_pos) = 0;
	S(obj_overflow) = FALSE;
	/* Clear sprite render buffer for first scanline */
	memset(S(sprite_buffer), 0, sizeof(S(sprite_buffer)));
	p3_update_register(P3_REGISTER_V);

	/* Begin frame rendering event */
	S(callback_proc)(0, P3_CALLBACK_BEGIN, S(callback_param));

	/* Render each scanline */
	for (S(frame_row) = 0; S(frame_row) < SCREEN_HEIGHT; ++S(frame_row)) {
		S(frame_row_pos) = 0;

		/* Begin scanline event */
		if (S(callback_type) == P3_CALLBACK_SCANLINE) {
			S(callback_proc)(0, S(frame_row), S(callback_param));
		}

		S(bg_show_mask) = S(show_bg) ? 0xFF : 0x00;
		S(obj_show_mask) = S(show_obj) ? 0xFF : 0x00;
		S(bg_clip_mask) = S(clip_bg) ? 0xFFFFFFFF : 0x00000000;
		S(obj_clip_mask) = S(clip_obj) ? 0xFFFFFFFF : 0x00000000;

		/* Fetch data for first tile */
		fetch_tile();

		/* Render eight pixels per tile */
		for (t = 0; t < 32; ++t) {
			write_pixel_cb();
			write_pixel_cb();
			write_pixel_cb();
			write_pixel_cb();
			write_pixel_cb();
			write_pixel_cb();
			write_pixel_cb();
			write_pixel_cb();
		}

		move_to_next_row();

		/* Restore horizontal nametable from register t */
		S(vpg) &= 2;
		S(vpg) |= (S(tpg) & 1);

		/* Restore horizontal position from register t */
		S(vcx) = S(tcx);
		S(vfx) = S(tfx);

		/* Render sprites for next scanline */
		render_sprite_buffer();
	}

	/* Restore v register from register t */
	S(vpg) = S(tpg);
	S(vcy) = S(tcy);
	S(vfy) = S(tfy);

	/* End frame rendering event */
	S(callback_proc)(0, P3_CALLBACK_END, S(callback_param));
}

P3_OBJECT *p3_create_object(void *chr, int chr_size)
{
	P3_OBJECT *prev_obj = g_p3obj;
	P3_OBJECT *obj = malloc(sizeof(P3_OBJECT));
	if (!obj) {
		set_last_error("p3_create_object(): out of memory");
		return NULL;
	}
	memset(obj, 0x00, sizeof(P3_OBJECT));

	obj->mmc_tables[0].table = P3_CHR_TABLE_LEFT;
	obj->mmc_tables[0].mode = &obj->left_table_mode;
	obj->mmc_tables[0].banks = obj->left_table_banks;
	obj->mmc_tables[0].shift = &obj->left_shift_lut;
	obj->mmc_tables[0].mask = &obj->left_mask_lut;
	obj->mmc_tables[0].group = &obj->left_group_lut;

	obj->mmc_tables[1].table = P3_CHR_TABLE_RIGHT;
	obj->mmc_tables[1].mode = &obj->right_table_mode;
	obj->mmc_tables[1].banks = obj->right_table_banks;
	obj->mmc_tables[1].shift = &obj->right_shift_lut;
	obj->mmc_tables[1].mask = &obj->right_mask_lut;
	obj->mmc_tables[1].group = &obj->right_group_lut;

	obj->idle = TRUE;
	obj->bg_palette = obj->palette_memory;
	obj->obj_palette = obj->palette_memory + 16;
	obj->callback_proc = default_callback;
	obj->last_attribute_pos = P3_ATTRIBUTE_TOP_LEFT;

	g_p3obj = obj;
	g_initialize_tileset(chr, chr_size);
	g_initialize_mapper();
	p3_reset(P3_RESET_ALL | P3_RESET_BUFS);
	p3_enable(TRUE);
	g_p3obj = prev_obj;

	if (g_p3obj == NULL) {
		g_p3obj = obj;
	}
	return obj;
}

void p3_destroy_object(P3_OBJECT **obj)
{
	if (obj) {
		if (*obj) {
			if (g_p3obj == *obj) {
				g_p3obj = NULL;
			}
			free(*obj);
			*obj = NULL;
		} else {
			set_last_error("p3_destroy_object(): bad 'obj' argument");
		}
	} else {
		if (g_p3obj) {
			free(g_p3obj);
			g_p3obj = NULL;
		}
	}
}

void p3_select_object(P3_OBJECT *obj)
{
	if (obj) {
		g_p3obj = obj;
	} else {
		set_last_error("p3_select_object(): bad 'obj' argument");
	}
}

P3_OBJECT *p3_get_current_object(void) { return g_p3obj; }

P3_OBJECT *p3_clone_object(P3_OBJECT *obj)
{
	P3_OBJECT *new_obj = malloc(sizeof(P3_OBJECT));
	if (!new_obj) {
		set_last_error("p3_clone_object(): out of memory");
		return NULL;
	}
	p3_copy_object(new_obj, obj);
	return new_obj;
}

void p3_copy_object(P3_OBJECT *dst, P3_OBJECT *src)
{
	if (dst && src) {
		memcpy(dst, src, sizeof(P3_OBJECT));
	} else {
		set_last_error("p3_copy_object(): bad arguments");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int p3_is_enabled(void) { return S(enabled); }

void p3_enable(int flag)
{
	if (S(idle)) {
		S(enabled) = TO_BOOL(flag);
	}
}

void p3_reset(int flags)
{
	if (flags & P3_RESET_NAMETABLES) {
		int i;
		for (i = 0; i < 4; ++i) {
			memset(S(page_memory) + S(mirroring_function)(i) * 1024, 0, 960);
		}
	}

	if (flags & P3_RESET_ATTRIBUTTES) {
		int i;
		for (i = 0; i < 4; ++i) {
			memset(S(page_memory) + S(mirroring_function)(i) * 1024 + 960, 0, 64);
		}
	}

	if (flags & P3_RESET_BG_PALETTE) {
		memset(S(bg_palette), 0, 16);
		p3_set_color(0, 0);
	}

	if (flags & P3_RESET_OBJ_PALETTE) {
		memset(S(obj_palette), 0, 16);
		/* Restore canvas color */
		p3_set_color(0, S(bg_palette)[0]);
	}

	if (flags & P3_RESET_OBJ) {
		sprite_index_t i;
		for (i = 0; i < OBJ_MAX; ++i) {
			S(obj_memory)[i] = default_sprite;
		}
	}

	if (flags & P3_RESET_T_REGISTER) {
		S(tpg) = S(tcx) = S(tcy) = S(tfx) = S(tfy) = 0;
	}

	if (flags & P3_RESET_V_REGISTER) {
		S(vpg) = S(vcx) = S(vcy) = S(vfx) = S(vfy) = 0;
	}

	if (flags & P3_RESET_CALLBACK) {
		p3_enable_callback(FALSE);
		S(callback_type) = P3_CALLBACK_SCANLINE;
		S(callback_param) = NULL;
		S(callback_x) = 0;
		S(callback_y) = 0;
		S(callback_proc) = default_callback;
	}

	if (flags & P3_RESET_STATE) {
		p3_enable(FALSE);
		p3_set_increment(P3_INCREMENT_HORIZONTAL);
		p3_show_bg(TRUE);
		p3_show_obj(TRUE);
		p3_clip_bg(FALSE);
		p3_clip_obj(FALSE);
		p3_enable_grayscale(FALSE);
		p3_set_tint(P3_TINT_OFF);
		p3_set_obj_mode(P3_OBJ_MODE_8X8);
		p3_set_bg_chr_table(P3_CHR_TABLE_LEFT);
		p3_set_obj_chr_table(P3_CHR_TABLE_RIGHT);
		p3_fix_obj_y(FALSE);
		p3_enable_callback(FALSE);
		if (S(idle)) {
			S(sprite_count) = 0;
			S(obj_overflow) = FALSE;
		}
	}

	if (flags & P3_RESET_BUFS) {
		memset(S(frame_buffer), 0, sizeof(S(frame_buffer)));
		memset(S(sprite_buffer), 0, sizeof(S(sprite_buffer)));
	}
}

int p3_is_show_bg(void) { return S(show_bg); }

void p3_show_bg(int flag)
{
	S(show_bg) = TO_BOOL(flag);
	if (S(callback_enabled)) {
		S(bg_show_mask) = S(show_bg) ? 0xFF : 0x00;
	}
}

int p3_is_show_obj(void) { return S(show_obj); }

void p3_show_obj(int flag)
{
	S(show_obj) = TO_BOOL(flag);
	if (S(callback_enabled)) {
		S(obj_show_mask) = S(show_obj) ? 0xFF : 0x00;
	}
}

int p3_is_clip_bg(void) { return S(clip_bg); }

void p3_clip_bg(int flag)
{
	if (flag) {
		if (S(callback_enabled) && (S(frame_row_pos) < 8)) {
			S(bg_clip_mask) = 0xFFFFFFFF >> (S(frame_row_pos) << 2);
		}
	} else {
		S(bg_clip_mask) = 0;
	}
	S(clip_bg) = TO_BOOL(flag);
}

int p3_is_clip_obj(void) { return S(clip_obj); }

void p3_clip_obj(int flag)
{
	if (flag) {
		if (S(callback_enabled) && (S(frame_row_pos) < 8)) {
			S(obj_clip_mask) = 0xFFFFFFFF >> (S(frame_row_pos) << 2);
		}
	} else {
		S(obj_clip_mask) = 0;
	}
	S(clip_obj) = TO_BOOL(flag);
}

int p3_is_grayscale(void) { return S(grayscale_mask) == GRAYSCALE_MASK_ON; }
void p3_enable_grayscale(int flag) { S(grayscale_mask) = flag ? GRAYSCALE_MASK_ON : GRAYSCALE_MASK_OFF; }
int p3_get_tint(void) { return S(tint_value) >> 6; }

void p3_set_tint(int tint)
{
	S(tint_value) = (uint16_t) (tint & P3_TINT_DARK) << 6;
}

int p3_get_obj_mode(void) { return S(obj_mode); }

void p3_set_obj_mode(int mode)
{
	if ((mode == 8) || (mode == 16))
		S(obj_mode) = (byte) mode;
	else
		set_last_error("p3_set_obj_mode(): bad 'mode' argument");
}

int p3_get_bg_chr_table(void) { return S(bg_pattern_table); }

void p3_set_bg_chr_table(int table)
{
	if ((table == P3_CHR_TABLE_LEFT) || (table == P3_CHR_TABLE_RIGHT)) {
		S(bg_pattern_table) = table;
		S(bg_chr_base) = table * 0x1000;
		g_update_mapper_fn();
	} else {
		set_last_error("p3_set_bg_chr_table(): bad 'table' argument");
	}
}

int p3_get_obj_chr_table(void) { return S(obj_pattern_table); }

void p3_set_obj_chr_table(int table)
{
	if ((table == P3_CHR_TABLE_LEFT) || (table == P3_CHR_TABLE_RIGHT)) {
		S(obj_pattern_table) = table;
		S(obj_chr_base) = table * 0x1000;
		g_update_mapper_fn();
	} else {
		set_last_error("p3_set_obj_chr_table(): bad 'table' argument");
	}
}

int p3_get_page(void) { return S(tpg); }
void p3_set_page(int page) { S(tpg) = page & 3; }
int p3_get_scroll_x(void) { return (S(tcx) << 3) | S(tfx); }

void p3_set_scroll_x(int value)
{
	value &= 0xff;
	S(tcx) = value >> 3;
	S(tfx) = value & 7;
}

int p3_get_scroll_y(void) { return (S(tcy) << 3) | S(tfy); }

void p3_set_scroll_y(int value)
{
	value &= 0xff;
	S(tcy) = value >> 3;
	S(tfy) = value & 7;
}

void p3_set_scroll(int x, int y)
{
	p3_set_scroll_x(x);
	p3_set_scroll_y(y);
}

void p3_write_obj(const P3_SPRITE *obj)
{
	if (obj)
		memcpy(S(obj_memory), obj, sizeof(S(obj_memory)));
	else
		set_last_error("p3_write_obj(): bad 'obj' argument");
}

P3_SPRITE p3_get_sprite(int index)
{
	if ((index >= 0) && (index < OBJ_MAX)) {
		return S(obj_memory)[index];
	} else {
		P3_SPRITE sprite = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
		set_last_error("p3_get_sprite(): 'index' out of range");		
		return sprite;
	}
}

void p3_put_sprite(int index, const P3_SPRITE *sprite)
{
	if (sprite)
		if ((index >= 0) && (index < OBJ_MAX))
			S(obj_memory)[index] = *sprite;
		else
			set_last_error("p3_put_sprite(): 'index' out of range");
	else
		set_last_error("p3_put_sprite(): bad 'sprite' argument");
}

void p3_reset_sprite(int index)
{
	if ((index >= 0) && (index < OBJ_MAX))
		S(obj_memory)[index] = default_sprite;
	else
		set_last_error("p3_reset_sprite(): 'index' out of range");
}

int p3_is_sprite_overflow(void) { return S(obj_overflow); }
int p3_is_fix_obj_y(void) { return S(fix_obj_y); }

void p3_fix_obj_y(int flag)
{
	if (S(idle)) {
		S(fix_obj_y) = TO_BOOL(flag);
	}
}

void p3_read_palette(void *buf, int b_bg)
{
	if (buf) {
		if (b_bg) {
			memcpy(buf, S(bg_palette), 16);
		} else {
			memcpy(buf, S(obj_palette), 16);
		}
	} else {
		set_last_error("p3_read_palette(): bad 'buf' argument");
	}
}

void p3_write_palette(const void *pal, int b_bg)
{
	if (pal) {
		byte *dst = b_bg ? S(bg_palette) : S(obj_palette);
		int i;
		for (i = 0; i < 16; ++i)
			dst[i] = ((const byte*) pal)[i] & 0x3f;

		/* Restore/Update canvas color */
		p3_set_color(0, S(bg_palette)[0]);
	} else {
		set_last_error("p3_write_palette(): bad 'pal' argument");
	}
}

int p3_get_color(int index)
{
	return S(palette_memory)[index & 0x1f];
}

void p3_set_color(int idx, int col)
{
	idx &= 0x1f;
	col &= 0x3f;
	if (!(idx & 3)) {
		int i;
		for (i = 0; i < 8; ++i) {
			S(palette_memory)[i * 4] = col;
		}
	} else {
		S(palette_memory)[idx] = col;
	}
}

int p3_get_palette_color(int pal, int col)
{
	return p3_get_color((pal & 7) * 4 + (col & 3));
}

void p3_set_palette_color(int pal, int col, int val)
{
	p3_set_color((pal & 7) * 4 + (col & 3), val & 0x3f);
}

int p3_get_canvas_color(void) { return p3_get_color(0); }
void p3_set_canvas_color(int color) { p3_set_color(0, color); }

int p3_get_bg_color(int pal, int col) { return p3_get_palette_color(pal, col); }
void p3_set_bg_color(int pal, int col, int val) { p3_set_palette_color(pal, col, val); }
int p3_get_obj_color(int pal, int col) { return p3_get_palette_color(4 + pal, col); }
void p3_set_obj_color(int pal, int col, int val) { p3_set_palette_color(4 + pal, col, val); }

int p3_get_register(int reg)
{
	switch (reg) {
	case P3_REGISTER_T:
		return ((int) S(tpg) << 10) | (S(tcy) << 5) | S(tcx);
	case P3_REGISTER_V:
		return ((int) S(vpg) << 10) | (S(vcy) << 5) | S(vcx);
	case P3_REGISTER_T_PAGE:
		return S(tpg);
	case P3_REGISTER_T_X:
		return S(tcx);
	case P3_REGISTER_T_Y:
		return S(tcy);
	case P3_REGISTER_T_FINE_X:
		return S(tfx);
	case P3_REGISTER_T_FINE_Y:
		return S(tfy);
	case P3_REGISTER_V_PAGE:
		return S(vpg);
	case P3_REGISTER_V_X:
		return S(vcx);
	case P3_REGISTER_V_Y:
		return S(vcy);
	case P3_REGISTER_V_FINE_X:
		return S(vfx);
	case P3_REGISTER_V_FINE_Y:
		return S(vfy);
	case P3_REGISTER_V_SCROLL_X:
		return (S(vcx) << 3) | S(vfx);
	case P3_REGISTER_V_SCROLL_Y:
		return (S(vcy) << 3) | S(vfy);
	default:
		set_last_error("p3_get_register(): bad 'reg' argument");
		return 0;
	}
}

void p3_set_register(int reg, int value)
{
	switch (reg) {
	case P3_REGISTER_T:
		value &= 0xfff;
		S(tpg) = (value >> 10) & 3;
		S(tcy) = (value >> 5) & 0x1f;
		S(tcx) = value & 0x1f;
		break;

	case P3_REGISTER_V:
		value &= 0xfff;
		S(vpg) = (value >> 10) & 3;
		S(vcy) = (value >> 5) & 0x1f;
		S(vcx) = value & 0x1f;
		break;

	case P3_REGISTER_T_PAGE:
		S(tpg) = (byte) value & 3;
		break;

	case P3_REGISTER_T_X:
		S(tcx) = (byte) value & 0x1f;
		break;

	case P3_REGISTER_T_Y:
		/* Allow scroll y > 239 */
		S(tcy) = (byte) value & 0x1f;
		break;

	case P3_REGISTER_T_FINE_X:
		S(tfx) = (byte) value & 7;
		break;

	case P3_REGISTER_T_FINE_Y:
		S(tfy) = (byte) value & 7;
		break;

	case P3_REGISTER_V_PAGE:
		S(vpg) = (byte) value & 3;
		break;

	case P3_REGISTER_V_X:
		S(vcx) = (byte) value & 0x1f;
		break;

	case P3_REGISTER_V_Y:
		/* Allow scroll y > 239 */
		S(vcy) = (byte) value & 0x1f;
		break;

	case P3_REGISTER_V_FINE_X:
		S(vfx) = (byte) value & 7;
		break;

	case P3_REGISTER_V_FINE_Y:
		S(vfy) = (byte) value & 7;
		break;

	case P3_REGISTER_V_SCROLL_X:
		value &= 0xff;
		S(vcx) = value >> 3;
		S(vfx) = value & 7;
		break;

	case P3_REGISTER_V_SCROLL_Y:
		/* Allow scroll y > 239 */
		value &= 0xff;
		S(vcy) = value >> 3;
		S(vfy) = value & 7;
		break;

	default:
		set_last_error("p3_set_register(): bad 'reg' argument");
	}
}

void p3_update_register(int reg)
{
	switch (reg) {
	case P3_REGISTER_T:
		S(tpg) = S(vpg);
		S(tcx) = S(vcx);
		S(tcy) = S(vcy);
		S(tfx) = S(vfx);
		S(tfy) = S(vfy);
		break;

	case P3_REGISTER_V:
		S(vpg) = S(tpg);
		S(vcx) = S(tcx);
		S(vcy) = S(tcy);
		S(vfx) = S(tfx);
		S(vfy) = S(tfy);
		break;

	default:
		set_last_error("p3_update_register(): 'reg' must be T or V");
	}
}

void p3_save_register(int reg)
{
	switch (reg) {
	case P3_REGISTER_T:
		S(tmp_tpg) = S(tpg);
		S(tmp_tcx) = S(tcx);
		S(tmp_tcy) = S(tcy);
		S(tmp_tfx) = S(tfx);
		S(tmp_tfy) = S(tfy);
		break;

	case P3_REGISTER_V:
		S(tmp_vpg) = S(vpg);
		S(tmp_vcx) = S(vcx);
		S(tmp_vcy) = S(vcy);
		S(tmp_vfx) = S(vfx);
		S(tmp_vfy) = S(vfy);
		break;

	default:
		set_last_error("p3_save_register(): 'reg' must be T or V");
	}
}

void p3_restore_register(int reg)
{
	switch (reg) {
	case P3_REGISTER_T:
		S(tpg) = S(tmp_tpg);
		S(tcx) = S(tmp_tcx);
		S(tcy) = S(tmp_tcy);
		S(tfx) = S(tmp_tfx);
		S(tfy) = S(tmp_tfy);
		break;

	case P3_REGISTER_V:
		S(vpg) = S(tmp_vpg);
		S(vcx) = S(tmp_vcx);
		S(vcy) = S(tmp_vcy);
		S(vfx) = S(tmp_vfx);
		S(vfy) = S(tmp_vfy);
		break;

	default:
		set_last_error("p3_restore_register(): 'reg' must be T or V");
	}
}

int p3_get_increment(void) { return S(increment_size); }
void p3_set_increment(int inc) { S(increment_size) = inc; }
int p3_get_address(void) { return p3_get_register(P3_REGISTER_V); }
void p3_set_address(int adr) { p3_set_register(P3_REGISTER_V, adr); }

int p3_get_byte(void)
{
	byte value = S(page_memory)[make_current_address()];
	increment_v_register();
	return value;
}

void p3_put_byte(int value)
{
	S(page_memory)[make_current_address()] = value & 0xff;
	increment_v_register();
}

int p3_get_byte_at(int adr)
{
	p3_set_address(adr);
	return p3_get_byte();
}

void p3_put_byte_at(int adr, int val)
{
	p3_set_address(adr);
	p3_put_byte(val);
}

void p3_read(void *dst, int num)
{
	if (dst) {
		byte *dst_ptr = (byte*) dst;
		int i;
		num &= 0xfff;
		for (i = 0; i < num; ++i, ++dst_ptr) {
			*dst_ptr = p3_get_byte();
		}
	} else {
		set_last_error("p3_read(): bad 'dst' argument");
	}
}

void p3_write(const void *src, int num)
{
	if (src) {
		const byte *src_ptr = (const byte*) src;
		int i;
		num &= 0xfff;
		for (i = 0; i < num; ++i, ++src_ptr) {
			p3_put_byte(*src_ptr);
		}
	} else {
		set_last_error("p3_write(): bad 'src' argument");
	}
}

void p3_fill(int val, int num)
{
	int i;
	val &= 0xff;
	num &= 0xfff;
	for (i = 0; i < num; ++i) {
		p3_put_byte(val);
	}
}

void *p3_get_v_pointer(void) { return &S(page_memory)[make_current_address()]; }
int p3_is_callback_enabled(void) { return S(callback_enabled); }

void p3_enable_callback(int flag)
{
	if (S(idle)) {
		S(callback_enabled) = TO_BOOL(flag);
	}
}

P3_CALLBACK p3_get_callback(void) { return S(callback_proc); }

void p3_set_callback(P3_CALLBACK proc, int type, int once_x, int once_y, void *param)
{
	if (S(idle)) {
		if (proc) {
			if ((type == P3_CALLBACK_SCANLINE) ||
				(type == P3_CALLBACK_PIXEL) ||
				(type == P3_CALLBACK_ONCE))
			{
				once_x &= 0xff;
				once_y &= 0xff;
				if (once_y > 239) once_y -= 239;

				S(callback_proc) = proc;
				S(callback_type) = type;
				S(callback_x) = once_x;
				S(callback_y) = once_y;
				S(callback_param) = param;
			} else {
				set_last_error("p3_set_callback(): bad 'type' argument");
			}
		} else {
			p3_reset(P3_RESET_CALLBACK);
		}
	}
}

void p3_adjust_callback(int type, int once_x, int once_y)
{
	if (S(callback_proc)) {
		if (!S(idle)) {
			S(callback_type) = type;
			S(callback_x) = once_x;
			S(callback_y) = once_y;
		}
	} else {
		set_last_error("p3_adjust_callback(): logic error, p3_set_callback must be called prior");
	}
}

void p3_reset_callback(void) { p3_reset(P3_RESET_CALLBACK); }
void p3_refetch_tile(void) { refetch_background_tile(); }

void p3_render(void)
{
	if (S(idle)) {
		if (S(enabled)) {
			S(idle) = FALSE;
			if (S(callback_enabled)) {
				render_frame_cb();
			} else {
				render_frame();
			}
			S(idle) = TRUE;
		} else {
			/* Note: NES PPU may use other palette entry here */
			uint16_t col = S(bg_palette)[0];
			uint16_t *ptr = S(frame_buffer);
			int i;
			for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
				*ptr++ = col;
		}
	}
}

const void *p3_get_frame_pointer(void) { return S(frame_buffer); }
