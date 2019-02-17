#### P3 - Picture Processing Program, NES PPU imitation library

Yes, this is simple picture generator that mimics NES PPU. With P3 you 
can make game that look like NES game.

Features:\
&nbsp; &nbsp; &nbsp; &nbsp; - Basic PPU emulation, NES-native tileset format\
&nbsp; &nbsp; &nbsp; &nbsp; - Simple memory mapper for tileset\
&nbsp; &nbsp; &nbsp; &nbsp; - Support raster effects with render callbacks\
&nbsp; &nbsp; &nbsp; &nbsp; - Support multiple P3 objects (not thread-safe)\

Usage:\
&nbsp; &nbsp; &nbsp; &nbsp; 1. Create P3 object. Call p3_create_object(), pass pointer to tileset\
&nbsp; &nbsp; &nbsp; &nbsp; and size of this tileset as arguments\
&nbsp; &nbsp; &nbsp; &nbsp; 2. Use P3 object\
&nbsp; &nbsp; &nbsp; &nbsp; 3. Generate picture. Call p3_render() function\
&nbsp; &nbsp; &nbsp; &nbsp; 4. Get pointer to frame buffer. Call p3_get_frame_pointer()\
&nbsp; &nbsp; &nbsp; &nbsp; 5. Convert picture to bitmap using any appropriate way - from simple\
&nbsp; &nbsp; &nbsp; &nbsp; palette blitter to NTSC filter.	Format of pixel: 2 bytes composed\
&nbsp; &nbsp; &nbsp; &nbsp; as xxxxxxxbgrpppppp, where p - index in system palette, rgb - color\
&nbsp; &nbsp; &nbsp; &nbsp; emphasis bits\
&nbsp; &nbsp; &nbsp; &nbsp; 6. Display bitmap on screen\
&nbsp; &nbsp; &nbsp; &nbsp; 7. Destroy P3 object when needed. Call p3_destroy_object() (pass NULL\
&nbsp; &nbsp; &nbsp; &nbsp; to destroy current object)\

Notes:\
&nbsp; &nbsp; &nbsp; &nbsp; - P3 in contrast to PPU has "address space" only for nametables, so \
&nbsp; &nbsp; &nbsp; &nbsp; address must be in 0..4095 range\
&nbsp; &nbsp; &nbsp; &nbsp; - P3 was not tested heavily and may contain myriads of bugs\
&nbsp; &nbsp; &nbsp; &nbsp; - VS2008 used for building, other compilers was not tested yet\

See also:\
&nbsp; &nbsp; &nbsp; &nbsp; [Port of Chase NES game to PC](https://github.com/kdv1/chase.git)
