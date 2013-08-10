
struct screen
{
	int width;
	int height;
	byte mode;
	byte vblank;
	dword xinc;
	dword yinc;
	dword physical;
	dword virtual;
	byte *screen_bkg;
};

struct bitmap
{
	dword width;
	dword height;
	dword xinc;
	dword yinc;
	int	xcord;
	int	ycord;
	int	mode;
	byte *bitmap_ptr;
	byte *bitmap_bkg;
};	
	
extern void setvga(byte x, struct screen *S);
extern void unsetvga(void);
extern void set_palette();
extern void draw_screen(struct screen *S);
extern void save_screen(struct screen *S);
extern void draw_bitmap(struct bitmap *B, struct screen *S);
extern void draw_bitmap_bkg(struct bitmap *B, struct screen *S);
extern void save_bitmap_bkg(struct bitmap *B, struct screen *S);
extern void init_vga();

