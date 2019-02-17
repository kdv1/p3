P3 - Picture Processing Program, NES PPU imitation library

Yes, this is simple picture generator that mimics NES PPU. With P3 you 
can make game that look like NES game.

Features:
	- Basic PPU emulation, NES-native tileset format
	- Simple memory mapper for tileset
	- Support raster effects with render callbacks
	- Support multiple P3 objects (not thread-safe)

Usage:
	1) Create P3 object. Call p3_create_object(), pass pointer to tileset 
	and size of this tileset as arguments.	
	2) Use P3 object.	
	3) Generate picture. Call p3_render() function.	
	4) Get pointer to frame buffer. Call p3_get_frame_pointer().	
	5) Convert picture to bitmap using any appropriate way - from simple 
	palette blitter to NTSC filter.	Format of pixel: 2 bytes composed 
	as xxxxxxxbgrpppppp, where p - index in system palette, rgb - color 
	emphasis bits.
	6) Display bitmap on screen.	
	7) Destroy P3 object when needed. Call p3_destroy_object() (pass NULL 
	to destroy current object).

Notes:
	- P3 in contrast to PPU has "address space" only for nametables, so 
	address must be in 0..4095 range.
	- P3 was not tested heavily and may contain myriads of bugs.
	- VS2008 used for building, other compilers was not tested yet.

See also:
	[Port of Chase NES game to PC](https://github.com/kdv1/chase.git)
