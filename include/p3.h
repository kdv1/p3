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

/* P3 - Picture Processing Program, NES PPU imitation library */

#ifndef __P3_H__
#define __P3_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Palette index */
#define P3_PALETTE_0                        0
#define P3_PALETTE_1                        1
#define P3_PALETTE_2                        2
#define P3_PALETTE_3                        3

/* Color index */
#define P3_COLOR_0                          0
#define P3_COLOR_1                          1
#define P3_COLOR_2                          2
#define P3_COLOR_3                          3

/* Palette selector */
#define P3_BG_PALETTE_0                     0
#define P3_BG_PALETTE_1                     1
#define P3_BG_PALETTE_2                     2
#define P3_BG_PALETTE_3                     3
#define P3_OBJ_PALETTE_0                    4
#define P3_OBJ_PALETTE_1                    5
#define P3_OBJ_PALETTE_2                    6
#define P3_OBJ_PALETTE_3                    7

/* Color luma */
#define P3_LUMA_0                           0x00
#define P3_LUMA_1                           0x10
#define P3_LUMA_2                           0x20
#define P3_LUMA_3                           0x30

/* Color chroma */
#define P3_CHROMA_GRAY                      0
#define P3_CHROMA_BLUE                      1
#define P3_CHROMA_BLUE_VIOLET               2
#define P3_CHROMA_VIOLET                    3
#define P3_CHROMA_MAGENTA                   4
#define P3_CHROMA_MAGENTA_RED               5
#define P3_CHROMA_RED                       6
#define P3_CHROMA_ORANGE                    7
#define P3_CHROMA_YELLOW                    8
#define P3_CHROMA_YELLOW_GREEN              9
#define P3_CHROMA_GREEN                     10
#define P3_CHROMA_GREEN_CYAN                11
#define P3_CHROMA_CYAN                      12

/* Some system palette colors */
#define P3_COLOR_BLACK                      0x0f
#define P3_COLOR_WHITE                      (P3_LUMA_2 | P3_CHROMA_GRAY)
#define P3_COLOR_GRAY                       (P3_LUMA_0 | P3_CHROMA_GRAY)
#define P3_COLOR_SILVER                     (P3_LUMA_1 | P3_CHROMA_GRAY)
#define P3_COLOR_NAVY                       (P3_LUMA_0 | P3_CHROMA_BLUE_VIOLET)
#define P3_COLOR_MAROON                     (P3_LUMA_0 | P3_CHROMA_RED)
#define P3_COLOR_BLUE                       (P3_LUMA_1 | P3_CHROMA_BLUE)
#define P3_COLOR_PURPLE                     (P3_LUMA_1 | P3_CHROMA_MAGENTA)
#define P3_COLOR_RED                        (P3_LUMA_1 | P3_CHROMA_RED)
#define P3_COLOR_BROWN                      (P3_LUMA_1 | P3_CHROMA_ORANGE)
#define P3_COLOR_OLIVE                      (P3_LUMA_1 | P3_CHROMA_YELLOW)
#define P3_COLOR_YELLOW                     (P3_LUMA_2 | P3_CHROMA_YELLOW)
#define P3_COLOR_GREEN                      (P3_LUMA_1 | P3_CHROMA_GREEN)
#define P3_COLOR_AQUA                       (P3_LUMA_1 | P3_CHROMA_CYAN)
#define P3_COLOR_MAGENTA                    (P3_LUMA_2 | P3_CHROMA_MAGENTA)
#define P3_COLOR_LIME                       (P3_LUMA_2 | P3_CHROMA_GREEN)
#define P3_COLOR_TEAL                       (P3_LUMA_2 | P3_CHROMA_CYAN)

/* Size of tileset */
#define P3_CHR_SIZE_8                       0x2000
#define P3_CHR_SIZE_16                      0x4000
#define P3_CHR_SIZE_32                      0x8000
#define P3_CHR_SIZE_64                      0x10000
#define P3_CHR_SIZE_128                     0x20000
#define P3_CHR_SIZE_256                     0x40000
#define P3_CHR_SIZE_512                     0x80000
#define P3_CHR_SIZE_1024                    0x100000

/* Pattern tables */
#define P3_CHR_TABLE_LEFT                   0
#define P3_CHR_TABLE_RIGHT                  1
#define P3_CHR_TABLE_BG                     2
#define P3_CHR_TABLE_OBJ                    3
#define P3_CHR_TABLE_PPU                    4
#define P3_CHR_TABLE_MAIN                   5

/* Banking modes */
#define P3_MMC_MODE_4X1                     0
#define P3_MMC_MODE_2X2                     1
#define P3_MMC_MODE_211                     2
#define P3_MMC_MODE_112                     3
#define P3_MMC_MODE_1X4                     4

/* Start address of bank */
#define P3_BANK_0K                          0
#define P3_BANK_1K                          1
#define P3_BANK_2K                          2
#define P3_BANK_3K                          3

/* Mirroring types */
#define P3_MIRRORING_TOP_LEFT               0
#define P3_MIRRORING_TOP_RIGHT              1
#define P3_MIRRORING_BOTTOM_LEFT            2
#define P3_MIRRORING_BOTTOM_RIGHT           3
#define P3_MIRRORING_HORIZONTAL             4
#define P3_MIRRORING_VERTICAL               5
#define P3_MIRRORING_LUT                    6
#define P3_MIRRORING_NONE                   7

/* Page index */
#define P3_PAGE_0                           0
#define P3_PAGE_1                           1
#define P3_TOP_LEFT_PAGE                    0
#define P3_TOP_RIGHT_PAGE                   1
#define P3_BOTTOM_LEFT_PAGE                 2
#define P3_BOTTOM_RIGHT_PAGE                3

/* P3 memory sizes */
#define P3_PAGE_SIZE                        1024
#define P3_NAMETABLE_SIZE                   960
#define P3_ATTRIBUTES_SIZE                  64

/* Base addresses */
#define P3_TOP_LEFT_NAMETABLE               0x0000
#define P3_TOP_RIGHT_NAMETABLE              0x0400
#define P3_BOTTOM_LEFT_NAMETABLE            0x0800
#define P3_BOTTOM_RIGHT_NAMETABLE           0x0C00
#define P3_TOP_LEFT_ATTRIBUTES              0x03C0
#define P3_TOP_RIGHT_ATTRIBUTES             0x07C0
#define P3_BOTTOM_LEFT_ATTRIBUTES           0x0BC0
#define P3_BOTTOM_RIGHT_ATTRIBUTES          0x0FC0

/* Position of attribute in byte */
#define P3_ATTRIBUTE_TOP_LEFT               0
#define P3_ATTRIBUTE_TOP_RIGHT              1
#define P3_ATTRIBUTE_BOTTOM_LEFT            2
#define P3_ATTRIBUTE_BOTTOM_RIGHT           3

/* Flags for p3_reset() */
#define P3_RESET_NAMETABLES                 (1 << 0)
#define P3_RESET_ATTRIBUTTES                (1 << 1)
#define P3_RESET_BG_PALETTE                 (1 << 2)
#define P3_RESET_OBJ_PALETTE                (1 << 3)
#define P3_RESET_OBJ                        (1 << 4)
#define P3_RESET_T_REGISTER                 (1 << 5)
#define P3_RESET_V_REGISTER                 (1 << 6)
#define P3_RESET_CALLBACK                   (1 << 7)
#define P3_RESET_STATE                      (1 << 8)
#define P3_RESET_PAGES                      (P3_RESET_NAMETABLES | P3_RESET_ATTRIBUTTES)
#define P3_RESET_PALETTES                   (P3_RESET_BG_PALETTE | P3_RESET_OBJ_PALETTE)
#define P3_RESET_REGISTERS                  (P3_RESET_T_REGISTER | P3_RESET_V_REGISTER)
#define P3_RESET_MEMORY                     (P3_RESET_PAGES | P3_RESET_PALETTES | P3_RESET_OBJ)
#define P3_RESET_ALL                        (P3_RESET_MEMORY | P3_RESET_REGISTERS | P3_RESET_CALLBACK | P3_RESET_STATE)

/* Color tint flags */
#define P3_TINT_OFF                         0
#define P3_TINT_RED                         (1 << 0)
#define P3_TINT_GREEN                       (1 << 1)
#define P3_TINT_BLUE                        (1 << 2)
#define P3_TINT_YELLOW                      (P3_TINT_RED | P3_TINT_GREEN)
#define P3_TINT_CYAN                        (P3_TINT_GREEN | P3_TINT_BLUE)
#define P3_TINT_MAGENTA                     (P3_TINT_RED | P3_TINT_BLUE)
#define P3_TINT_DARK                        (P3_TINT_RED | P3_TINT_GREEN | P3_TINT_BLUE)

/* Scroll/Memory/Rendering registers */
#define P3_REGISTER_T                       0
#define P3_REGISTER_V                       1
#define P3_REGISTER_T_PAGE                  2
#define P3_REGISTER_T_X                     3
#define P3_REGISTER_T_Y                     4
#define P3_REGISTER_T_FINE_X                5
#define P3_REGISTER_T_FINE_Y                6
#define P3_REGISTER_V_PAGE                  7
#define P3_REGISTER_V_X                     8
#define P3_REGISTER_V_Y                     9
#define P3_REGISTER_V_FINE_X                10
#define P3_REGISTER_V_FINE_Y                11
#define P3_REGISTER_V_SCROLL_X              12
#define P3_REGISTER_V_SCROLL_Y              13

/* Address increment */
#define P3_INCREMENT_NONE                   0
#define P3_INCREMENT_LEFT                   -1
#define P3_INCREMENT_RIGHT                  1
#define P3_INCREMENT_UP                     -32
#define P3_INCREMENT_DOWN                   32
#define P3_INCREMENT_HORIZONTAL             P3_INCREMENT_RIGHT
#define P3_INCREMENT_VERTICAL               P3_INCREMENT_DOWN

/* Type of render callback function */
#define P3_CALLBACK_SCANLINE                0
#define P3_CALLBACK_PIXEL                   1
#define P3_CALLBACK_ONCE                    2

/* Phase of frame rendering, passed in y */
#define P3_CALLBACK_BEGIN                   255
#define P3_CALLBACK_END                     240

/* Size mode for all sprites */
#define P3_OBJ_MODE_8X8                     8
#define P3_OBJ_MODE_8X16                    16

/* Sprite priority */
#define P3_SPRITE_FRONT                     0
#define P3_SPRITE_BEHIND                    1

/* One of 64 sprites */
typedef struct p3_sprite {
	unsigned char x;
	unsigned char y;
	unsigned char tile;	
	unsigned char palette;
	unsigned char priority;
	unsigned char flip_horizontal;
	unsigned char flip_vertical;	
} P3_SPRITE;

/* Render callback function */
typedef void (*P3_CALLBACK)(int x, int y, void *param);
/* P3 instance */
typedef struct p3_object P3_OBJECT;

/* P3 object functions */
P3_OBJECT *p3_create_object(void *chr, int chr_size);
void p3_destroy_object(P3_OBJECT **obj);
void p3_select_object(P3_OBJECT *obj);
P3_OBJECT *p3_get_current_object(void);
P3_OBJECT *p3_clone_object(P3_OBJECT *obj);
void p3_copy_object(P3_OBJECT *dst, P3_OBJECT *src);

/* Tileset functions */
void *p3_get_chr_ptr(void);
void p3_set_chr_ptr(void *chr, int chr_size);
int p3_get_chr_size(void);
int p3_get_tile_count(void);
void *p3_get_tile(int index);
void p3_put_tile(int index, const void *tile);
void p3_copy_tiles(int dst, int src, int num, int b_mapdst, int b_mapsrc);
void p3_read_tiles(void *buf, int start, int num, int b_usemmc);
void p3_write_tiles(const void *tiles, int start, int num, int b_usemmc);

/* Mapper functions */
int p3_get_mmc_mode(int table);
void p3_set_mmc_mode(int table, int mode);
int p3_get_bank(int table, int bank_adr);
void p3_set_bank(int table, int bank_adr, int num);
void p3_reset_table_banks(int table);
void p3_reset_banks(void);
void p3_setup_banks(int bank8k);
int p3_get_last_banks_setup(void);
int p3_map_address(int address);
int p3_map_tile(int index);
int p3_get_mirroring_type(void);
void p3_set_mirroring_type(int type);
void p3_get_mirroring_lut(int lut[4]);
void p3_set_mirroring_lut(const int lut[4]);
void p3_set_mirroring_lut_4(int tl, int tr, int bl, int br);
int p3_mirror_page(int page);

/* P3 functions */
int p3_is_enabled(void);
void p3_enable(int flag);
void p3_reset(int flags);
int p3_is_show_bg(void);
void p3_show_bg(int flag);
int p3_is_show_obj(void);
void p3_show_obj(int flag);
int p3_is_clip_bg(void);
void p3_clip_bg(int flag);
int p3_is_clip_obj(void);
void p3_clip_obj(int flag);
int p3_is_grayscale(void);
void p3_enable_grayscale(int flag);
int p3_get_tint(void);
void p3_set_tint(int tint);
int p3_get_obj_mode(void);
void p3_set_obj_mode(int mode);
int p3_get_bg_chr_table(void);
void p3_set_bg_chr_table(int table);
int p3_get_obj_chr_table(void);
void p3_set_obj_chr_table(int table);
int p3_get_page(void);
void p3_set_page(int page);
int p3_get_scroll_x(void);
void p3_set_scroll_x(int value);
int p3_get_scroll_y(void);
void p3_set_scroll_y(int value);
void p3_set_scroll(int x, int y);
void p3_write_obj(const P3_SPRITE *obj);
P3_SPRITE p3_get_sprite(int index);
void p3_put_sprite(int index, const P3_SPRITE *sprite);
void p3_reset_sprite(int index);
int p3_is_sprite_overflow(void);
int p3_is_fix_obj_y(void);
void p3_fix_obj_y(int flag);
void p3_read_palette(void *buf, int b_bg);
void p3_write_palette(const void *pal, int b_bg);
int p3_get_color(int index);
void p3_set_color(int idx, int col);
int p3_get_palette_color(int pal, int col);
void p3_set_palette_color(int pal, int col, int val);
int p3_get_canvas_color(void);
void p3_set_canvas_color(int color);
int p3_get_bg_color(int pal, int col);
void p3_set_bg_color(int pal, int col, int val);
int p3_get_obj_color(int pal, int col);
void p3_set_obj_color(int pal, int col, int val);
int p3_get_register(int reg);
void p3_set_register(int reg, int value);
void p3_update_register(int reg);
void p3_save_register(int reg);
void p3_restore_register(int reg);
int p3_get_increment(void);
void p3_set_increment(int inc);
int p3_get_address(void);
void p3_set_address(int adr);
int p3_get_byte(void);
void p3_put_byte(int value);
int p3_get_byte_at(int adr);
void p3_put_byte_at(int adr, int val);
void p3_read(void *dst, int num);
void p3_write(const void *src, int num);
void p3_fill(int val, int num);
void *p3_get_v_pointer(void);
int p3_is_callback_enabled(void);
void p3_enable_callback(int flag);
P3_CALLBACK p3_get_callback(void);
void p3_set_callback(P3_CALLBACK proc, int type, int once_x, int once_y, void *param);
void p3_adjust_callback(int type, int once_x, int once_y);
void p3_reset_callback(void);
void p3_refetch_tile(void);
void p3_render(void);
const void *p3_get_frame_pointer(void);

/* Tile utils */
void p3_fill_tile(void *tile, int color);
int p3_get_tile_pixel(void *tile, int x, int y);
void p3_set_tile_pixel(void *tile, int x, int y, int color);
int p3_is_tile_transparent(void *tile);
int p3_is_tile_opaque(void *tile);
int p3_is_tile_has_alpha(void *tile);
void p3_copy_tile(void *dst, const void *src);
int p3_is_equal_tiles(const void *tile1, const void *tile2);
int p3_make_tile_index_1m(int index);
int p3_make_tile_index_1t(int table, int index);
int p3_make_tile_index_1tm(int table, int index);
int p3_make_tile_index_2(int x, int y);
int p3_make_tile_index_2m(int x, int y);
int p3_make_tile_index_2t(int table, int x, int y);
int p3_make_tile_index_2tm(int table, int x, int y);

/* Page utils */
void p3_zero_page(int page);
void p3_fill_page(int page, int tile, int pal);
void p3_read_page(int page, void *buf);
void p3_write_page(int page, const void *data);

/* Nametable utils */
void p3_fill_nametable(int page, int tile);
void p3_zero_nametable(int page);
void p3_fill_nametable_row_2(int page, int row, int tile, int start, int end);
void p3_fill_nametable_row(int page, int row, int tile);
void p3_fill_nametable_column_2(int page, int col, int tile, int start, int end);
void p3_fill_nametable_column(int page, int col, int tile);
void p3_read_nametable(int page, void *buf);
void p3_write_nametable(int page, const void *data);
int p3_make_name_address(int index);
int p3_make_name_address_1t(int page, int index);
int p3_make_name_address_2(int x, int y);
int p3_make_name_address_2t(int page, int x, int y);
int p3_get_name(int index);
void p3_put_name(int index, int tile);
int p3_get_name_1t(int page, int index);
void p3_put_name_1t(int page, int index, int tile);
int p3_get_name_2(int x, int y);
void p3_put_name_2(int x, int y, int tile);
int p3_get_name_2t(int page, int x, int y);
void p3_put_name_2t(int page, int x, int y, int tile);

/* Attribute table utils */
void p3_fill_attribute_table(int page, int pal);
void p3_zero_attribute_table(int page);
void p3_fill_attribute_table_row_2(int page, int row, int pal, int start, int end);
void p3_fill_attribute_table_row(int page, int row, int pal);
void p3_fill_attribute_table_column_2(int page, int col, int pal, int start, int end);
void p3_fill_attribute_table_column(int page, int col, int pal);
void p3_read_attribute_table(int page, void *buf);
void p3_write_attribute_table(int page, const void *data);
int p3_get_attribute_position(void);
int p3_make_attribute_address(int index);
int p3_make_attribute_address_1t(int page, int index);
int p3_make_attribute_address_2(int x, int y);
int p3_make_attribute_address_2t(int page, int x, int y);
int p3_get_attribute(int index);
void p3_put_attribute(int index, int pal);
int p3_get_attribute_1t(int page, int index);
void p3_put_attribute_1t(int page, int index, int pal);
int p3_get_attribute_2(int x, int y);
void p3_put_attribute_2(int x, int y, int pal);
int p3_get_attribute_2t(int page, int x, int y);
void p3_put_attribute_2t(int page, int x, int y, int pal);
int p3_make_flat_attribute_byte(int pal);
int p3_make_attribute_byte(int tl, int tr, int bl, int br);
int p3_get_attribute_byte_item(int attr, int pos);
int p3_set_attribute_byte_item(int attr, int pos, int pal);
int p3_get_attribute_byte_item_x(int attr);
int p3_set_attribute_byte_item_x(int attr, int pal);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* !__P3_H__ */
