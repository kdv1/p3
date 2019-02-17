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
void g_initialize_mapper(void);
void g_update_mapper_fn(void);
byte *g_get_tile(padr_t);
byte *g_get_bg_tile(padr_t);
byte *g_get_obj_tile(padr_t);
CHECK_LINE(void g_check_banks();)

/* Forward */
static void convert_banks(int from, int to, cadr_t *banks);

#define MMC_MODE_SKIP 0
#define MMC_MODE_BANK_8 1
#define MMC_MODE_TABS 2

/* Used for conversion: bank number <-> base memory address */
static const byte shift_luts[5][4] = {
	{ 12, 12, 12, 12 },
	{ 11, 11, 11, 11 },
	{ 11, 11, 10, 10 },
	{ 10, 10, 11, 11 },
	{ 10, 10, 10, 10 }
};

/* Used to mask bits of input memory address */
static const padr_t mask_luts[5][4] = {
	{ 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF },
	{ 0x07FF, 0x07FF, 0x07FF, 0x07FF },
	{ 0x07FF, 0x07FF, 0x03FF, 0x03FF },
	{ 0x03FF, 0x03FF, 0x07FF, 0x07FF },
	{ 0x03FF, 0x03FF, 0x03FF, 0x03FF }
};

/* Used to group 1 KB banks to larger banks */
static const byte group_luts[5][4] = {
	{ P3_BANK_0K, P3_BANK_0K, P3_BANK_0K, P3_BANK_0K },
	{ P3_BANK_0K, P3_BANK_0K, P3_BANK_2K, P3_BANK_2K },
	{ P3_BANK_0K, P3_BANK_0K, P3_BANK_2K, P3_BANK_3K },
	{ P3_BANK_0K, P3_BANK_1K, P3_BANK_2K, P3_BANK_2K },
	{ P3_BANK_0K, P3_BANK_1K, P3_BANK_2K, P3_BANK_3K }
};

/* Initial setup of bank numbers to first 8 KB of tileset */
static const cadr_t setup_banks_data[2][5][4] = {
	{ {0, 0, 0, 0}, {0, 0, 1, 1}, {0, 0, 2, 3}, {0, 1, 1, 1}, {0, 1, 2, 3} },
	{ {1, 1, 1, 1}, {2, 2, 3, 3}, {2, 2, 6, 7}, {4, 5, 3, 3}, {4, 5, 6, 7} }
};

/* Bank scale relative to 8 KB bank */
static const byte banking_mode_scale[5][4] = {
	{2, 2, 2, 2}, {4, 4, 4, 4}, {4, 4, 8, 8}, {8, 8, 4, 4}, {8, 8, 8, 8}
};

/* MMC_MODE_SKIP */
static cadr_t skip_mapping(padr_t address) { return address; }

/* MMC_MODE_BANK_8 */
static cadr_t map_address_8(padr_t address) { return S(bank_8x1) | address; }

/* MMC_MODE_TABS */
static cadr_t map_left_table_address(padr_t address)
{
	/* Round down input address to 1 KB segment, 0..3 */
	byte segment = (address >> 10) & 3;
	/* Compose address */
	return S(left_table_banks)[segment] | (address & S(left_mask_lut)[segment]);
}

/* MMC_MODE_TABS */
static cadr_t map_right_table_address(padr_t address)
{
	/* Round down input address to 1 KB segment, 0..3 */
	byte segment = (address >> 10) & 3;
	/* Compose address */
	return S(right_table_banks)[segment] | (address & S(right_mask_lut)[segment]);
}

static cadr_t(* const map_func_lut[2])(padr_t) = {
	map_left_table_address, map_right_table_address
};

static cadr_t map_address(padr_t address)
{
	return (address & 0x1000) ?
	       map_right_table_address(address) : map_left_table_address(address);
}

byte *g_get_tile(padr_t address) {	return &S(tileset_pointer)[S(main_map_func)(address)]; }
byte *g_get_bg_tile(padr_t address) { return &S(tileset_pointer)[S(bg_map_func)(address)]; }
byte *g_get_obj_tile(padr_t address) { return &S(tileset_pointer)[S(obj_map_func)(address)]; }

#ifdef P3_CHECKED

static void check_table_banks(int pattern_table)
{
	const struct mmc_table_state *table = &S(mmc_tables)[pattern_table];
	cadr_t *banks = table->banks;
	const padr_t *mask = *table->mask;
	int i;
	for (i = 0; i < 4; ++i) {
		/* Check using max input address */
		assert((banks[i] | (0x1fff & mask[i])) < (unsigned) S(tileset_size));
	}
}

void g_check_banks(void)
{
	if (S(glob_mmc_mode) == MMC_MODE_BANK_8) {
		assert((S(bank_8x1) >> 13) < (unsigned)(S(tileset_size) >> 13));
	} else if (S(glob_mmc_mode) == MMC_MODE_TABS) {
		check_table_banks(P3_CHR_TABLE_LEFT);
		check_table_banks(P3_CHR_TABLE_RIGHT);
	}
}

#endif /* P3_CHECKED */

static void set_banking_mode(int pattern_table, int mode)
{
	const struct mmc_table_state *table = &S(mmc_tables)[pattern_table];
	/* Setup lookup mmc_tables */
	*table->mode = mode;
	*table->shift = shift_luts[mode];
	*table->mask = mask_luts[mode];
	*table->group = group_luts[mode];
}

static void convert_table_banks(int pattern_table, int mode)
{
	const struct mmc_table_state *table = &S(mmc_tables)[pattern_table];
	if (*table->mode != mode) {
		cadr_t *banks = table->banks;
		const byte *shift = *table->shift;
		int i;
		/* Convert base address to bank number */
		for (i = 0; i < 4; ++i) {
			banks[i] >>= shift[i];
		}
		/* Convert bank numbers */
		convert_banks(*table->mode, mode, banks);
		/* Set new banking mode */
		set_banking_mode(pattern_table, mode);
		/* Convert bank number to base address */
		/* Update pointer to new shift table */
		shift = *table->shift;
		for (i = 0; i < 4; ++i) {
			banks[i] <<= shift[i];
		}
		/* TODO: remove this check */
		CHECK_LINE(check_table_banks(pattern_table);)
	}
}

static int get_table_bank(int pattern_table, int bank)
{
	const struct mmc_table_state *table = &S(mmc_tables)[pattern_table];
	const byte *group = *table->group;
	int i;
	for (i = 0; i < 4; ++i) {
		if (group[i] == bank) {
			/* Convert base address to bank number */
			return table->banks[i] >> (*table->shift)[i];
		}
	}
	set_last_error("p3_get_bank(): bad 'bank' argument");
	return 0;
}

static void set_table_bank(int pattern_table, int bank, int number)
{
	const struct mmc_table_state *table = &S(mmc_tables)[pattern_table];
	cadr_t *banks = table->banks;
	const byte *group = *table->group;
	const byte *shift = *table->shift;
	int i;
	BOOL success = FALSE;
	for (i = 0; i < 4; ++i) {
		if (group[i] == bank) {
			/* Convert bank number to base address */
			banks[i] = number << shift[i];
			success = TRUE;
		}
	}
	if (!success)
		set_last_error("p3_set_bank(): bad 'bank' argument");
	CHECK_LINE(check_table_banks(pattern_table));
}

static void setup_table_banks(int pattern_table, int num_8kb)
{
	const struct mmc_table_state *table = &S(mmc_tables)[pattern_table];
	cadr_t *banks = table->banks;
	const byte *shift = *table->shift;
	int i;
	/* Copy initial bank numbers */
	memcpy(banks, setup_banks_data[table->table][*table->mode], sizeof(cadr_t) * 4);
	/* Shift to next 8 KB bank, if needed */
	if (num_8kb) {
		const byte *scale = banking_mode_scale[*table->mode];
		for (i = 0; i < 4; ++i) {
			banks[i] += (num_8kb * scale[i]);
		}
	}
	/* Convert bank number to base address */
	for (i = 0; i < 4; ++i) {
		banks[i] <<= shift[i];
	}
}

static void reset_table_banks(int pattern_table)
{
	memset(S(mmc_tables)[pattern_table].banks, 0, sizeof(cadr_t) * 4);
}

void g_update_mapper_fn(void)
{
	switch (S(glob_mmc_mode)) {
	case MMC_MODE_SKIP:
		S(main_map_func) = skip_mapping;
		S(bg_map_func) = skip_mapping;
		S(obj_map_func) = skip_mapping;
		break;

	case MMC_MODE_BANK_8:
		S(main_map_func) = map_address_8;
		S(bg_map_func) = map_address_8;
		S(obj_map_func) = map_address_8;
		break;

	case MMC_MODE_TABS:
		S(main_map_func) = map_address;
		S(bg_map_func) = map_func_lut[S(bg_pattern_table)];
		S(obj_map_func) = map_func_lut[S(obj_pattern_table)];
	}
}

static int resolve_pattern_table(int table)
{
	switch (table) {
	case P3_CHR_TABLE_BG:
		return S(bg_pattern_table);

	case P3_CHR_TABLE_OBJ:
		return S(obj_pattern_table);

	default:
		return table;
	}
}

/* Mirroring */
static byte top_left_mirroring(byte page) {	return P3_TOP_LEFT_PAGE; }
static byte top_right_mirroring(byte page) { return P3_TOP_RIGHT_PAGE; }
static byte bottom_left_mirroring(byte page) { return P3_BOTTOM_LEFT_PAGE; }
static byte bottom_right_mirroring(byte page) { return P3_BOTTOM_RIGHT_PAGE; }
static byte horizontal_mirroring(byte page) { return page >> 1; }
static byte vertical_mirroring(byte page) { return page & 1; }
static byte custom_mirroring(byte page) { return S(mirroring_lut)[page]; }
static byte four_mirroring(byte page) { return page; }

static byte(* const mirroring_func_lut[8])(byte) = {
	top_left_mirroring, top_right_mirroring,
	bottom_left_mirroring, bottom_right_mirroring,
	horizontal_mirroring, vertical_mirroring,
	custom_mirroring, four_mirroring
};

void g_initialize_mapper(void)
{
	set_banking_mode(P3_CHR_TABLE_LEFT, P3_MMC_MODE_4X1);
	set_banking_mode(P3_CHR_TABLE_RIGHT, P3_MMC_MODE_4X1);
	p3_setup_banks(0);
	g_update_mapper_fn();
	p3_set_mirroring_type(P3_MIRRORING_HORIZONTAL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void late_setup(void)
{
	if (S(glob_mmc_mode) != MMC_MODE_TABS) {
		int num = S(bank_8x1) >> 13;
		setup_table_banks(P3_CHR_TABLE_LEFT, num);
		setup_table_banks(P3_CHR_TABLE_RIGHT, num);
		S(glob_mmc_mode) = MMC_MODE_TABS;
		g_update_mapper_fn();
		/* TODO remove */
		CHECK_LINE(g_check_banks();)
	}
}

int p3_get_mmc_mode(int table)
{
	if ((table >= P3_CHR_TABLE_LEFT) && (table <= P3_CHR_TABLE_OBJ))
		return *S(mmc_tables)[resolve_pattern_table(table)].mode;
	set_last_error("p3_get_mmc_mode(): bad 'table' argument");
	return 0;
}

void p3_set_mmc_mode(int table, int mode)
{
	if ((mode >= P3_MMC_MODE_4X1) && (mode <= P3_MMC_MODE_1X4)) {
		if ((table >= P3_CHR_TABLE_LEFT) && (table <= P3_CHR_TABLE_OBJ)) {
			late_setup();
			convert_table_banks(resolve_pattern_table(table), mode);
		} else {
			set_last_error("p3_set_mmc_mode(): bad 'table' argument");
		}
	} else {
		set_last_error("p3_set_mmc_mode(): bad 'mode' argument");
	}
}

int p3_get_bank(int table, int bank_adr)
{
	if ((table >= P3_CHR_TABLE_LEFT) && (table <= P3_CHR_TABLE_OBJ)) {
		late_setup();
		return get_table_bank(resolve_pattern_table(table), bank_adr & 3);
	}
	set_last_error("p3_get_bank(): bad 'table' argument");
	return 0;
}

void p3_set_bank(int table, int bank_adr, int num)
{
	if ((table >= P3_CHR_TABLE_LEFT) && (table <= P3_CHR_TABLE_OBJ)) {
		late_setup();
		set_table_bank(resolve_pattern_table(table), bank_adr & 3, num);
	} else {
		set_last_error("p3_set_bank(): bad 'table' argument");
	}
}

void p3_reset_table_banks(int table)
{
	if ((table >= P3_CHR_TABLE_LEFT) && (table <= P3_CHR_TABLE_OBJ)) {
		late_setup();
		reset_table_banks(resolve_pattern_table(table));
	} else {
		set_last_error("p3_reset_table_banks(): bad 'table' argument");
	}
}

void p3_reset_banks(void)
{
	p3_reset_table_banks(P3_CHR_TABLE_LEFT);
	p3_reset_table_banks(P3_CHR_TABLE_RIGHT);
}

void p3_setup_banks(int bank8k)
{
	if (((unsigned) bank8k) < 128) {		
		int newmode = MMC_MODE_BANK_8;
		bank8k &= 0x7f;
		if (!bank8k) {
			newmode = MMC_MODE_SKIP;
			S(bank_8x1) = 0;
		} else {
			/* Convert bank number to base address */
			S(bank_8x1) = bank8k << 13;
		}
		if (newmode != S(glob_mmc_mode)) {
			S(glob_mmc_mode) = newmode;
			g_update_mapper_fn();
		}
		CHECK_LINE(g_check_banks();)
	} else {
		set_last_error("p3_setup_banks(): 'bank8k' out of range");
	}
}

int p3_get_last_banks_setup(void) { return S(bank_8x1) >> 13; }
int p3_map_address(int address) { return S(main_map_func)(address & 0x1fff); }
int p3_map_tile(int index) { return S(main_map_func)((index & 0x1ff) << 4) >> 4; }
int p3_get_mirroring_type(void) { return S(mirroring_type); }

void p3_set_mirroring_type(int type)
{
	if ((type >= P3_MIRRORING_TOP_LEFT) && (type <= P3_MIRRORING_NONE)) {
		S(mirroring_type) = type;
		S(mirroring_function) = mirroring_func_lut[S(mirroring_type)];
	} else {
		set_last_error("p3_set_mirroring_type(): bad 'type' argument");
	}
}

void p3_get_mirroring_lut(int lut[4])
{
	if (lut) {
		lut[0] = S(mirroring_lut)[0];
		lut[1] = S(mirroring_lut)[1];
		lut[2] = S(mirroring_lut)[2];
		lut[3] = S(mirroring_lut)[3];
	} else {
		set_last_error("p3_get_mirroring_lut(): bad 'lut' argument");
	}
}

void p3_set_mirroring_lut(const int lut[4])
{
	if (lut) {
		S(mirroring_lut)[0] = lut[0] & 3;
		S(mirroring_lut)[1] = lut[1] & 3;
		S(mirroring_lut)[2] = lut[2] & 3;
		S(mirroring_lut)[3] = lut[3] & 3;
	} else {
		set_last_error("p3_set_mirroring_lut(): bad 'lut' argument");
	}
}

void p3_set_mirroring_lut_4(int tl, int tr, int bl, int br)
{
	int lut[4] = { tl, tr, bl, br };
	p3_set_mirroring_lut(lut);
}

int p3_mirror_page(int page)
{
	page &= 3;
	return S(mirroring_function)(page);
}

/* * * * * * * * * * * * * * Bank-number converter * * * * * * * * * * * * * */

/* Conversion command */
#define SKIP         0
#define MUL2         9
#define MUL4         10
#define MUL8         11
#define DIV2         5
#define DIV4         6
#define DIV8         7
#define CPYB         1

/* Increment, add to result bank number */
#define ADD1         1
#define ADD2         2
#define ADD3         3

/* Argument for CPYB command */
#define BAN0         0
#define BAN1         1
#define BAN2         2
#define BAN3         3

/* Used to convert bank numbers between different banking modes */
struct conversion_data_item {
	byte cmd : 6;           /* Command to execute */
	byte num : 2;           /* Increment or argument for CPYB */
};

/* Conversion data for all combinations of banking modes */
static const struct conversion_data_item conversion_data[5][5][4] = {
	{ /* P3_MMC_MODE_4X1 -> P3_MMC_MODE_4X1 */
		{{SKIP}, {SKIP}, {SKIP}, {SKIP}},
		/* P3_MMC_MODE_4X1 -> P3_MMC_MODE_2X2 */
		{{MUL2}, {MUL2}, {MUL2, ADD1}, {MUL2, ADD1}},
		/* P3_MMC_MODE_4X1 -> P3_MMC_MODE_211 */
		{{MUL2}, {MUL2}, {MUL4, ADD2}, {MUL4, ADD3}},
		/* P3_MMC_MODE_4X1 -> P3_MMC_MODE_112 */
		{{MUL4}, {MUL4, ADD1}, {MUL2, ADD1}, {MUL2, ADD1}},
		/* P3_MMC_MODE_4X1 -> P3_MMC_MODE_1X4 */
		{{MUL4}, {MUL4, ADD1}, {MUL4, ADD2}, {MUL4, ADD3}},
	},
	{ /* P3_MMC_MODE_2X2 -> P3_MMC_MODE_4X1 */
		{{DIV2}, {CPYB, BAN0}, {CPYB, BAN0}, {CPYB, BAN0}},
		/* P3_MMC_MODE_2X2 -> P3_MMC_MODE_2X2 */
		{{SKIP}, {SKIP}, {SKIP}, {SKIP}},
		/* P3_MMC_MODE_2X2 -> P3_MMC_MODE_211 */
		{{SKIP}, {SKIP}, {MUL2}, {MUL2, ADD1}},
		/* P3_MMC_MODE_2X2 -> P3_MMC_MODE_112 */
		{{MUL2}, {MUL2, ADD1}, {SKIP}, {SKIP}},
		/* P3_MMC_MODE_2X2 -> P3_MMC_MODE_1X4 */
		{{MUL2}, {MUL2, ADD1}, {MUL2}, {MUL2, ADD1}},
	},
	{ /* P3_MMC_MODE_211 -> P3_MMC_MODE_4X1 */
		{{DIV2}, {CPYB, BAN0}, {CPYB, BAN0}, {CPYB, BAN0}},
		/* P3_MMC_MODE_211 -> P3_MMC_MODE_2X2 */
		{{SKIP}, {SKIP}, {DIV2}, {CPYB, BAN2}},
		/* P3_MMC_MODE_211 -> P3_MMC_MODE_211 */
		{{SKIP}, {SKIP}, {SKIP}, {SKIP}},
		/* P3_MMC_MODE_211 -> P3_MMC_MODE_112 */
		{{MUL2}, {MUL2, ADD1}, {DIV2}, {CPYB, BAN2}},
		/* P3_MMC_MODE_211 -> P3_MMC_MODE_1X4 */
		{{MUL2}, {MUL2, ADD1}, {SKIP}, {SKIP}},
	},
	{ /* P3_MMC_MODE_112 -> P3_MMC_MODE_4X1 */
		{{DIV4}, {CPYB, BAN0}, {CPYB, BAN0}, {CPYB, BAN0}},
		/* P3_MMC_MODE_112 -> P3_MMC_MODE_2X2 */
		{{DIV2}, {CPYB, BAN0}, {SKIP}, {SKIP}},
		/* P3_MMC_MODE_112 -> P3_MMC_MODE_211 */
		{{DIV2}, {CPYB, BAN0}, {MUL2}, {MUL2, ADD1}},
		/* P3_MMC_MODE_112 -> P3_MMC_MODE_112 */
		{{SKIP}, {SKIP}, {SKIP}, {SKIP}},
		/* P3_MMC_MODE_112 -> P3_MMC_MODE_1X4 */
		{{SKIP}, {SKIP}, {MUL2}, {MUL2, ADD1}},
	},
	{ /* P3_MMC_MODE_1X4 -> P3_MMC_MODE_4X1 */
		{{DIV4}, {CPYB, BAN0}, {CPYB, BAN0}, {CPYB, BAN0}},
		/* P3_MMC_MODE_1X4 -> P3_MMC_MODE_2X2 */
		{{DIV2}, {CPYB, BAN0}, {DIV2}, {CPYB, BAN2}},
		/* P3_MMC_MODE_1X4 -> P3_MMC_MODE_211 */
		{{DIV2}, {CPYB, BAN0}, {SKIP}, {SKIP}},
		/* P3_MMC_MODE_1X4 -> P3_MMC_MODE_112 */
		{{SKIP}, {SKIP}, {DIV2}, {CPYB, BAN2}},
		/* P3_MMC_MODE_1X4 -> P3_MMC_MODE_1X4 */
		{{SKIP}, {SKIP}, {SKIP}, {SKIP}},
	}
};

static void convert_banks(int from, int to, cadr_t *banks)
{
	const struct conversion_data_item *conv = conversion_data[from][to];
	int i;
	for (i = 0; i < 4; ++i) {
		byte cmd = conv[i].cmd;
		if (!cmd) {
			/* Skipping */
			continue;
		} else if (cmd & 8) {
			/* Multiplication */
			banks[i] = (banks[i] << (cmd & 3)) + conv[i].num;
		} else if (cmd & 4) {
			/* Division */
			banks[i] = (banks[i] >> (cmd & 3)) + conv[i].num;
		} else {
			/* Copying */
			banks[i] = banks[conv[i].num];
		}
	}
}
