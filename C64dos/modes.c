
#include <dpmi.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <sys/nearptr.h>
#include <conio.h>

#define VIDEO_INT           0x10      /* the BIOS video interrupt. */
#define SET_MODE            0x00      /* BIOS func to set the video mode. */
#define VGA_256_COLOR_MODE  0x13      /* use to set 256-color mode. */
#define TEXT_MODE           0x03      /* use to set 80x25 text mode. */


#define MISC_OUTPUT         0x03c2    /* VGA misc. output register */
#define SC_INDEX            0x03c4    /* VGA sequence controller */
#define SC_DATA             0x03c5
#define PALETTE_INDEX       0x03c8    /* VGA digital-to-analog converter */
#define PALETTE_DATA        0x03c9
#define GRAC_ADD			0x3ce
#define CRTC_INDEX          0x03d4    /* VGA CRT controller */

#define MAP_MASK            0x02      /* Sequence controller registers */
#define MEMORY_MODE         0x04

#define H_TOTAL             0x00      /* CRT controller registers */
#define H_DISPLAY_END       0x01
#define H_BLANK_START       0x02
#define H_BLANK_END         0x03
#define H_RETRACE_START     0x04
#define H_RETRACE_END       0x05
#define V_TOTAL             0x06
#define OVERFLOW            0x07
#define MAX_SCAN_LINE       0x09
#define V_RETRACE_START     0x10
#define V_RETRACE_END       0x11
#define V_DISPLAY_END       0x12
#define OFFSET              0x13
#define UNDERLINE_LOCATION  0x14
#define V_BLANK_START       0x15
#define V_BLANK_END         0x16
#define MODE_CONTROL        0x17

#define NUM_COLORS          256       /* number of colors in mode 0x13 */

/* macro to return the sign of a number */
#define sgn(x) \
  ((x<0)?-1:((x>0)?1:0))

/* macro to write a word to a port */
#define word_out(port,register,value) \
  outpw(port,(((word)value<<8) + register))

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;
	
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
	
void setvga(byte x, struct screen *the_screen);
void unsetvga(void);
void set_palette();
void draw_screen(struct screen *the_screen);
void save_screen(struct screen *the_screen);
void draw_bitmap(struct bitmap *a_bitmap, struct screen *the_screen);
void draw_bitmap_bkg(struct bitmap *a_bitmap, struct screen *the_screen);
void save_bitmap_bkg(struct bitmap *a_bitmap, struct screen *the_screen);
void init_vga();

void set_mode(byte mode);						/* hidden from outside */
void set_unchained_mode(struct screen *the_screen, int width, int height);	
byte *VGA = (byte *)0xA0000;						/* this points to video memory. */

dword page_width, page_height;

dword mypalette=0;
float gamma=5.0;
float gfactor=16.0;

struct screen *my_screen;

byte palette0[]=
{
	0x00, 0x00, 0x00,		/* Black */
	0xfd, 0xfe, 0xfc,		/* White */
	0xbe, 0x1a, 0x24,		/* Red */
	0x30, 0xe6, 0xc6,		/* Cyan */
	0xb4, 0x1a, 0xe2,		/* Purple */
	0x1f, 0xd2, 0x1e,		/* Green */
	0x21, 0x1b, 0xae,		/* Blue */
	0xdf, 0xf6, 0x0a,		/* Yellow */
	0xb8, 0x41, 0x04,		/* Orange */
	0x6a, 0x33, 0x04,		/* Brown */
	0xfe, 0x4a, 0x57,		/* Light Red */
	0x42, 0x45, 0x40,		/* Dark Gray */
	0x70, 0x74, 0x6f,		/* Medium Gray */
	0x59, 0xfe, 0x59,		/* Light Green */
	0x5f, 0x53, 0xfe,		/* Light Blue */
	0xa4, 0xa7, 0xa2		/* Light Gray */
};

byte palette1[]=	
{
	0x00, 0x00, 0x00,		/* Black */
	0xff, 0xff, 0xff,		/* White */
	0xf0, 0x00, 0x00,		/* Red */
	0x00, 0xf0, 0xf0,		/* Cyan */
	0x60, 0x00, 0x60,		/* Purple */
	0x00, 0xa0, 0x00,		/* Green */
	0x00, 0x00, 0xf0,		/* Blue */
	0xd0, 0xd0, 0x00,		/* Yellow */
	0xc0, 0xa0, 0x00,		/* Orange */
	0xff, 0xa0, 0x00,		/* Light Orange */
	0xf0, 0x80, 0x80,		/* Pink */
	0x00, 0xff, 0xff,		/* Light Cyan */
	0xff, 0x00, 0xff,		/* Light Purple */
	0x00, 0xff, 0x00,		/* Light Green */
	0x00, 0xa0, 0xff,		/* Light Blue */
	0xff, 0xff, 0x00		/* Light Yellow */
};

byte palette2[]=		
{
	0x10,0x10,0x10,         // Black
	0xff,0xff,0xff,         // White
	0xe0,0x40,0x40,         // Red
	0x60,0xff,0xff,         // Cyan
	0xe0,0x60,0xe0,         // Purple
	0x40,0xe0,0x40,         // Green
	0x40,0x40,0xe0,         // Blue
	0xff,0xff,0x40,         // Yellow
	0xe0,0xa0,0x40,         // Orange
	0x9c,0x74,0x48,         // Brown
	0xff,0xa0,0xa0,         // Lt.Red
	0x54,0x54,0x54,         // Dk.Gray
	0x88,0x88,0x88,         // Gray
	0xa0,0xff,0xa0,         // Lt.Green
	0xa0,0xa0,0xff,         // Lt.Blue
	0xc0,0xc0,0xc0          // Lt.Gray	
};

byte palette3[]=		
{
	0x00,0x00,0x00,		/* 0	black		*/
	0x3f,0x3f,0x3f,		/* 1	white		*/
	0x3f,0x00,0x00,		/* 2	red		*/
	0x00,0x30,0x3f,		/* 3	cyan		*/
	0x3f,0x00,0x30,		/* 4	purple	*/
	0x00,0x3f,0x0a,		/* 5	green		*/
	0x10,0x10,0x30,		/* 6	blue		*/
	0x3f,0x3f,0x10,		/* 7	yellow	*/
	0x3f,0x20,0x10,		/* 8	orange	*/
	0x20,0x10,0x00,		/* 9	brown		*/
	0x3f,0x10,0x10,		/* a	lt red	*/
	0x10,0x10,0x10,		/* b	drk grey	*/
	0x20,0x20,0x20,		/* c	med grey	*/
	0x10,0x3f,0x10,		/* d	lt green	*/
	0x20,0x20,0x3f,		/* e	lt blue	*/
	0x30,0x30,0x30		/* f	lt grey	*/
};
	
/**************************************************************************
 *  set_mode                                                              *
 *     Sets the video mode.                                               *
 **************************************************************************/
void set_mode(byte mode)
{
	__dpmi_regs regs;
	regs.x.ax=mode;
	__dpmi_int(0x10,&regs);
}

/**************************************************************************
 *  set_unchained_mode                                                    *
 *    Resets VGA mode 0x13 to unchained mode to access all 256K of        *
 *    memory.  width may be 320 or 360, height may be 200, 400, 240 or    *
 *    480.  If an invalid width or height is specified, it defaults to    *
 *    320x200                                                             *
 **************************************************************************/

void set_unchained_mode(struct screen *the_screen,int width, int height)
{
	word i;
	dword *ptr=(dword *)VGA;

	/* set mode 13 */
	set_mode(VGA_256_COLOR_MODE);

	/* turn off chain-4 mode */
	word_out(SC_INDEX, MEMORY_MODE,0x06);

	/* set map mask to all 4 planes for screen clearing */
	word_out(SC_INDEX, MAP_MASK, 0xff);

	/* clear all 256K of memory */
	for(i=0;i<0x4000;i++)	*ptr++ = 0;

	/* turn off long mode */
	word_out(CRTC_INDEX, UNDERLINE_LOCATION, 0x00);

	/* turn on byte mode */
	word_out(CRTC_INDEX, MODE_CONTROL, 0xe3);


	the_screen->width=320;
	the_screen->height=200;
	the_screen->mode=0x00;

	if (width==360)
	{
	/* turn off write protect */
		word_out(CRTC_INDEX, V_RETRACE_END, 0x2c);

		outp(MISC_OUTPUT, 0xe7);
		word_out(CRTC_INDEX, H_TOTAL, 0x6b);
		word_out(CRTC_INDEX, H_DISPLAY_END, 0x59);
		word_out(CRTC_INDEX, H_BLANK_START, 0x5a);
		word_out(CRTC_INDEX, H_BLANK_END, 0x8e);
		word_out(CRTC_INDEX, H_RETRACE_START, 0x5e);
		word_out(CRTC_INDEX, H_RETRACE_END, 0x8a);
		word_out(CRTC_INDEX, OFFSET, 0x2d);

		/* set vertical retrace back to normal */
		word_out(CRTC_INDEX, V_RETRACE_END, 0x8e);

		the_screen->width=360;
		the_screen->mode=0x1;
	}
	else
	{
		outp(MISC_OUTPUT, 0xe3);
	}

	if (height==240 || height==480)
	{
		/* turn off write protect */
		word_out(CRTC_INDEX, V_RETRACE_END, 0x2c);

		word_out(CRTC_INDEX, V_TOTAL, 0x0d);
		word_out(CRTC_INDEX, OVERFLOW, 0x3e);
		word_out(CRTC_INDEX, V_RETRACE_START, 0xea);
		word_out(CRTC_INDEX, V_RETRACE_END, 0xac);
		word_out(CRTC_INDEX, V_DISPLAY_END, 0xdf);
		word_out(CRTC_INDEX, V_BLANK_START, 0xe7);
		word_out(CRTC_INDEX, V_BLANK_END, 0x06);
		the_screen->height=height;
		the_screen->mode=0x1;
	}

	if (height==400 || height==480)
	{
		word_out(CRTC_INDEX, MAX_SCAN_LINE, 0x40);
		the_screen->height=height;
		the_screen->mode=0x1;
	}
}

/*--------------------------------------------------------------------------------------------*/

void draw_screen(struct screen *the_screen)
{
	dword x,y,base;
	byte *buffer_ptr;
	buffer_ptr=the_screen->screen_bkg;
	base=the_screen->yinc;
	if(the_screen->mode==0)
	{
		for(x=0;x<base;x++)
		{
			VGA[x]=*buffer_ptr++;
		}
	}
	else
	{
		for(y=0;y<4;y++)
		{
			outp(SC_INDEX, MAP_MASK);
			/* select plane */
			outp(SC_DATA,0x01<<y);
			for(x=0;x<base;x++)
			{
				VGA[x]=*buffer_ptr++;
			}
		}
	}
}

void save_screen(struct screen *the_screen)
{
	dword x,y,base;
	byte *buffer_ptr;
	buffer_ptr=the_screen->screen_bkg;
	base=the_screen->yinc;
	if(the_screen->mode==0)
	{
		for(x=0;x<base;x++)
		{
			*buffer_ptr++=VGA[x];
		}
	}
	else
	{
		for(y=0;y<4;y++)
		{
			outp(GRAC_ADD, 4);
			/* select plane */
			outp(GRAC_ADD+1,y&3);
			for(x=0;x<base;x++)
			{
				*buffer_ptr++=VGA[x];
			}
		}
	}
}

void draw_bitmap(struct bitmap *a_bitmap, struct screen *the_screen)
{
	dword base;
	int d1x,d2x,d1y,d2y;
	int mode_x;
	byte plane;
	byte *buffer_ptr,*buffer_ptr2, *buffer_ptr3;

	page_height=the_screen->height;
	if(the_screen->mode)
	{
		/* x mode render plus clipper */
		mode_x=a_bitmap->xcord>>2;
		page_width=the_screen->xinc;

		buffer_ptr3=a_bitmap->bitmap_ptr;
		for(plane=0;plane<4;plane++)
		{
			outp(SC_INDEX, MAP_MASK);
			/* select plane */
			outp(SC_DATA,0x01<<plane);
			buffer_ptr2=buffer_ptr3++;
			d2y=a_bitmap->ycord;
			base=0;
			for(d1y=0;d1y<a_bitmap->height;d1y++)
			{
				buffer_ptr=buffer_ptr2;
				if((d2y>=0)&&(d2y<page_height))
				{
					d2x=mode_x;
					base=page_width*d2y;
					for(d1x=0;d1x<a_bitmap->xinc;d1x++)
					{
						if((d2x>0)&&(d2x<page_width))
						{
							VGA[d2x+base]=*buffer_ptr;
						}
						buffer_ptr+=4;
						d2x++;
					}
				}
				buffer_ptr2+=a_bitmap->width;
				d2y++;
			}
		}
	}
	else
	{
		/* write to screen */
		page_width=the_screen->width;
		d2y=a_bitmap->ycord;
		buffer_ptr=buffer_ptr2=a_bitmap->bitmap_ptr;
		for(d1y=0;d1y<a_bitmap->height;d1y++)
		{
			buffer_ptr=buffer_ptr2;
			if((d2y>=0)&&(d2y<page_height))
			{
				d2x=a_bitmap->xcord;
				base=page_width*d2y;
				for(d1x=0;d1x<=a_bitmap->width;d1x++)
				{
					if((d2x>0)&&(d2x<page_width))
					{
						VGA[d2x+base]=*buffer_ptr;
					}
					buffer_ptr++;
					d2x++;
				}
			}
			buffer_ptr2+=a_bitmap->width;
			d2y++;
		}
	}
}

void draw_bitmap_bkg(struct bitmap *a_bitmap, struct screen *the_screen)
{
	dword base;
	int d1x,d2x,d1y,d2y;
	int mode_x;
	byte plane;
	byte *buffer_ptr;

	page_height=the_screen->height;
	buffer_ptr=a_bitmap->bitmap_bkg;
	if(the_screen->mode)
	{
		/* x mode render plus clipper */
		mode_x=a_bitmap->xcord>>2;
		page_width=the_screen->xinc;
		for(plane=0;plane<4;plane++)
		{
			outp(SC_INDEX, MAP_MASK);
			/* select plane */
			outp(SC_DATA,0x01<<plane);
			d2y=a_bitmap->ycord;
			base=0;
			for(d1y=0;d1y<a_bitmap->height;d1y++)
			{
				if((d2y>=0)&&(d2y<page_height))
				{
					d2x=mode_x;
					base=page_width*d2y;
					for(d1x=0;d1x<a_bitmap->xinc;d1x++)
					{
						if((d2x>0)&&(d2x<page_width))
						{
							VGA[d2x+base]=*buffer_ptr++;
						}
						d2x++;
					}
				}
				d2y++;
			}
		}
	}
	else
	{
		/* write to screen */
		page_width=the_screen->width;
		d2y=a_bitmap->ycord;
		for(d1y=0;d1y<a_bitmap->height;d1y++)
		{
			if((d2y>=0)&&(d2y<page_height))
			{
				d2x=a_bitmap->xcord;
				base=page_width*d2y;
				for(d1x=0;d1x<=a_bitmap->width;d1x++)
				{
					if((d2x>0)&&(d2x<page_width))
					{
						VGA[d2x+base]=*buffer_ptr++;
					}
					d2x++;
				}
			}
			d2y++;
		}
	}
}

void save_bitmap_bkg(struct bitmap *a_bitmap, struct screen *the_screen)
{
	dword base;
	int d1x,d2x,d1y,d2y;
	int mode_x;
	byte plane;
	byte *buffer_ptr;
	page_height=the_screen->height;
	buffer_ptr=a_bitmap->bitmap_bkg;
	
	if(the_screen->mode)
	{
		/* x mode render plus clipper */
		mode_x=a_bitmap->xcord>>2;
		page_width=the_screen->xinc;
		for(plane=0;plane<4;plane++)
		{
			outp(GRAC_ADD, 4);
			/* select plane */
			outp(GRAC_ADD+1,plane&3);
			
			d2y=a_bitmap->ycord;
			base=0;
			for(d1y=0;d1y<a_bitmap->height;d1y++)
			{
				if((d2y>=0)&&(d2y<page_height))
				{
					d2x=mode_x;
					base=page_width*d2y;
					for(d1x=0;d1x<a_bitmap->xinc;d1x++)
					{
						if((d2x>0)&&(d2x<page_width))
						{
							*buffer_ptr++=VGA[d2x+base];
						}
						d2x++;
					}
				}
				d2y++;
			}
		}
	}
	else
	{
		/* write to screen */
		page_width=the_screen->width;
		d2y=a_bitmap->ycord;
		for(d1y=0;d1y<a_bitmap->height;d1y++)
		{
			if((d2y>=0)&&(d2y<page_height))
			{
				d2x=a_bitmap->xcord;
				base=page_width*d2y;
				for(d1x=0;d1x<=a_bitmap->width;d1x++)
				{
					if((d2x>0)&&(d2x<page_width))
					{
						*buffer_ptr++=VGA[d2x+base];
					}
					d2x++;
				}
			}
			d2y++;
		}
	}
}

/**************************************************************************
 *  set_palette                                                           *
 *    Sets all 256 colors of the palette.                                 *
 **************************************************************************/
void set_palette()
{
	int x=0;
	dword temp=0;
	byte num=0;
	byte *p;
	switch(mypalette)
	{
		case 0:
			p=palette0;
			break;
		case 1:
			p=palette1;
			break;
		case 2:
			p=palette2;
			break;
		case 3:
			p=palette3;
			break;
		default:
			mypalette=0;
			p=palette0;
			break;
	}
	while(x<=45)
	{
		outportb(0x3c6,0xff);
		outportb(0x3c8,num);
		temp=((*p++)/gfactor)*gamma;
		if(temp>0x3f) temp=0x3f;
		outportb(0x3c9,temp);
		x++;
		temp=((*p++)/gfactor)*gamma;
		if(temp>0x3f) temp=0x3f;
		outportb(0x3c9,temp);
		x++;
		temp=((*p++)/gfactor)*gamma;
		if(temp>0x3f) temp=0x3f;
		outportb(0x3c9,temp);
		x++;
		num++;
	}
}


/**************************************************************************
 *  Main                                                                  *
 **************************************************************************/

void screen_stats(struct screen *the_screen)
{
	dword x,y;
	x=the_screen->width;
	y=the_screen->height;
	if(the_screen->mode==0)
	{
		the_screen->xinc=x;
		the_screen->yinc=x*y;
	}
	else
	{
		x>>=2;
		the_screen->xinc=x;
		the_screen->yinc=x*y;
	}
	if(the_screen->vblank) the_screen->vblank=0xff;
}

void setvga(byte x,struct screen *the_screen)
{
	my_screen=the_screen;
	the_screen->mode=x;

	if(the_screen->mode)
	{
	      set_unchained_mode(the_screen,360,240);
	}
	else
	{
		the_screen->width=320;
		the_screen->height=200;
		set_mode(VGA_256_COLOR_MODE);
	}
	screen_stats(the_screen);
      set_palette();
}

void unsetvga(void)
{
	__djgpp_nearptr_disable();
	set_mode(TEXT_MODE);
}

void init_vga()
{
	if (__djgpp_nearptr_enable() == 0)
	{
		printf("Could get access to first 640K of memory.\n");
		exit(-1);
	}
	VGA+=__djgpp_conventional_base;
	printf("VGA pointer initialized\n");
}
