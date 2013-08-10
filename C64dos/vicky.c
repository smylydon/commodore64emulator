
#include <dpmi.h>
#include <dos.h>
#include <go32.h>
#include <pc.h>
#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/farptr.h>
#include <sys/segments.h>
#include <sys/movedata.h>
#include <dir.h>
#include "include/mytypes.h"
#include "include/keys.h"
#include "include/M6510.h"
#include "include/modes.h"
#include "include/vic2.h"

#define ECM		0x40
#define BMM		0x20
#define DEN		0x10
#define MCM		0x10
#define COLMASK	0x0f
#define DIM		320*2

#define FIRST_RASTER_LINE		0x10
#define LAST_RASTER_LINE		0x11f

#define DISPLAY_ROW25_START		0x33
#define DISPLAY_ROW25_END		0xfa
#define DISPLAY_ROW24_START		0x37
#define DISPLAY_ROW24_END		0xf6

#define DISPLAY_DMA_START		0x30
#define DISPLAY_DMA_END			0xf7
#define t_50Hz				22000

#define SC_INDEX				0x03c4    /* VGA sequence controller */
#define SC_DATA				0x03c5
#define MAP_MASK				0x02      /* Sequence controller registers */

/* Externals */
extern dword cpu_base_cycles;
extern void UpdateVIC2(void);
extern void StandardTxtMode(void);
extern void ExtendedTxtMode(void);
extern void MultiColorTxtMode(void);
extern void	StandardBitMapMode(void);
extern void	MultiColorBitMapMode(void);
extern void InvalidTxtMode(void);
extern void	InvalidBitMapMode1(void);
extern void	InvalidBitMapMode2(void);
extern dword uclock_read(void);

/* prototypes */
void LoadVic(dword *p);
void SaveVic(dword *p);
void DrawPlaneSprite(void);
void DrawPlaneSpriteX2(void);
void DrawMulticolorSprite(void);
void DrawMulticolorSpriteX2(void);
void DumpSprites(void);
void DrawSprites(void);
void InitVicTables(struct screen *S);
void DrawC64DisplayMode(void);
void ComputeBadline(void);
void RenderChainedMode(void);
void RenderUnchainedMode(void);
void VicII(void);	

dword StandardColorTable[256];
dword MultiColorTable[256];
dword MC[32];
byte GfxLine[DIM];
byte SpriteLine[DIM];
byte sp2spcol[DIM];
byte sp2gfxcol[DIM];
byte VMLI[64];
byte VMLC[64];
dword VicBank,VideoMatrix,scrmode,txtmode,VicTemp1,VicTemp2;
dword image,scrollx,scrolly,matrixid,bankid;
dword VCBASE=0;
dword VCCOUNT=0;
dword RC=0;
dword raster;
dword screenmode;
dword BorderColor;
dword PaperColor;

dword DelYstart=0x33;
dword DelYend=0xfa;
dword cpu_badline_cycles;
dword ztime=0;
dword ztime2=0;
dword py,px,SpritePointers,bitpattern;
dword bitmapbase,screenbase,CharBank,Spriteptrs;
dword mcmcol0,mcmcol1,mcmcol2,mcmcol3;
dword *ptr2MC,*ptrx,*ptry;			
short our_global_selector;

byte *gfxchar,*mnshline,*spcoll,*gcoll,*sprite;
byte *BitMapMatrix,*BitMapColor,*MCBASE;
byte *vmliptr,*vmlcptr;
byte *sprdum0;
byte *sprdum1;
byte *sprdum2;
byte *sprena=VIC2CHIP+0x15;
byte *sprexpx=VIC2CHIP+0x1d;
byte *sprexpy=VIC2CHIP+0x17;
byte *sprmcm=VIC2CHIP+0x1c;
byte *sp2dat=VIC2CHIP+0x1f;
byte *sp2sp=VIC2CHIP+0x1e;
byte *spdatp=VIC2CHIP+0x1b;
byte sp,sp2,display;
byte col0,col1,col2,col3;
byte badline_flag,badline_enable_flag,idlestate_flag;
byte in_screen_window,screen_enable_flag,in_dma_window,in_deltay_window;
extern byte rawkey;
extern dword ticks;
extern dword oldticks;

struct screen *pc_screen;
struct bitmap *pc_bitmap;
struct bitmap my_line;

/*-----------------------------------------------------------------------*/
/*			Load Vic from Savestate position				 */
/*-----------------------------------------------------------------------*/

/*
**
**	LoadVic restores Vic II emulation from 'save state'
**
*/
void LoadVic(dword *p)
{
	dword i;
	dword fixcolor1,fixcolor2;
	for(i=0;i<64;i++)
	{
		VMLI[i]=(byte)*p++;
	}
	for(i=0;i<64;i++)
	{
		VMLC[i]=(byte)*p++;
	}	
	VicBank=*p++;
	VideoMatrix=*p++;
	scrmode=*p++;
	txtmode=*p++;
	matrixid=*p++;
	bankid=*p++;
	VCBASE=*p++;
	VCCOUNT=*p++;
	RC=*p++;
	raster=*p++;
	screenmode=*p++;
	DelYstart=*p++;
	DelYend=*p++;
	SpritePointers=*p++;
	bitmapbase=*p++;
	screenbase=*p++;
	CharBank=*p++;
	Spriteptrs=*p++;
	display=(byte)*p++;
	badline_flag=(byte)*p++;
	badline_enable_flag=(byte)*p++;
	idlestate_flag=(byte)*p++;
	fixcolor2=0;
	fixcolor1=VIC2CHIP[0x20]&0xf;
	fixcolor2=fixcolor1|(fixcolor1<<0x8);
	fixcolor1=fixcolor2<<0x10;
	BorderColor=fixcolor1|fixcolor2;
	fixcolor2=0;
	fixcolor1=VIC2CHIP[0x21]&0xf;
	fixcolor2=fixcolor1|(fixcolor1<<0x8);
	fixcolor1=fixcolor2<<0x10;
	PaperColor=fixcolor1|fixcolor2;
}

/*
**
**	SaveVic saves the state of Vic II emulation to HDD
**
*/
void SaveVic(dword *p)
{
	dword i;
	for(i=0;i<64;i++)
	{
		*p++=(dword)VMLI[i];
	}
	for(i=0;i<64;i++)
	{
		*p++=(dword)VMLC[i];
	}	
	*p++=VicBank;
	*p++=VideoMatrix;
	*p++=scrmode;
	*p++=txtmode;
	*p++=matrixid;
	*p++=bankid;
	*p++=VCBASE;
	*p++=VCCOUNT;
	*p++=RC;
	*p++=raster;
	*p++=screenmode;
	*p++=DelYstart;
	*p++=DelYend;
	*p++=SpritePointers;
	*p++=bitmapbase;
	*p++=screenbase;
	*p++=CharBank;
	*p++=Spriteptrs;
	*p++=(dword)display;
	*p++=(dword)badline_flag;
	*p++=(dword)badline_enable_flag;
	*p++=(dword)idlestate_flag;
}

/*-----------------------------------------------------------------------*/
/*			Draw C64 sprites to screen					 */
/*-----------------------------------------------------------------------*/

/*
**
**	Draws a mono coloured sprite with NO horizontal expansion
**
*/
void DrawPlaneSprite(void)
{
	byte x,width;
	byte check=*spdatp&sp2;
	sprdum0=gcoll;							/* graphics collision			*/
	sprdum1=spcoll;							/* sprite collision			*/
	sprdum2=mnshline;							/* horizontal line			*/
	for(width=0;width<3;width++)
	{
		x=0x80;							/* test bit x in sprite bitpattern	*/
		bitpattern=*sprite++;					/* first byte of sprite data		*/
		while(x)
		{
			if(bitpattern&x)					/* is the text bit x set		*/
			{
				if(*sprdum1==0)					/* is there a sprite pixel already painted here line	*/				
				{
					if((check==0)||(*sprdum0==0))		/* has sprite sp got priority or no gfx pixel present	*/	
					{						
						*sprdum2=col3;		
					}						
				}							
				else							
				{
					*sp2sp|=*sprdum1|sp2;			/* indicate sprite sp collide with another sprite	*/			
				}

				if(*sprdum0==1)					/* is there gfx present here					*/				
				{							
					*sp2dat|=sp2;				
				}							
				*sprdum1|=sp2;					/* mark sprite sp has pixel at this point			*/
			}
			sprdum0++;
			sprdum1++;
			sprdum2++;
			x>>=1;
		}
	}
}

/*
**
**	Draws a mono coloured sprite with horizontal expansion
**
*/
void DrawPlaneSpriteX2(void)
{
	dword x,width;
	byte check=*spdatp&sp2;
	sprdum0=gcoll;
	sprdum1=spcoll;
	sprdum2=mnshline;
	for(width=0;width<3;width++)
	{
		x=0x8000;
		bitpattern=StandardColorTable[*sprite++];			/* use look up table for horizontal expansion */
		while(x)
		{
			if(bitpattern&x)
			{
				if(*sprdum1==0)				
				{								
					if((check==0)||(*sprdum0==0))	
					{						
						*sprdum2=col3;		
					}						
				}							
				else							
				{							
					*sp2sp|=*sprdum1|sp2;			
				}							
				if(*sprdum0==1)				
				{							
					*sp2dat|=sp2;				
				}							
				*sprdum1|=sp2;													
			}
			sprdum0++;
			sprdum1++;
			sprdum2++;
			x>>=1;
		}
	}
}

/*
**
**	Draws a multi colour sprite with NO horizontal expansion 
**
*/
void DrawMulticolorSprite(void)
{
	byte y;
	dword testbitx,x,p;
	byte check=*spdatp&sp2;
	sprdum0=gcoll;
	sprdum1=spcoll;
	sprdum2=mnshline;
	for(y=0;y<3;y++)
	{
		testbitx=0xc0;
		p=6;
		bitpattern=*sprite++;
		while(testbitx)
		{
			x=(bitpattern&testbitx)>>p;
			switch(x)
			{
				case 0x3:
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col2;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;
					sprdum0++;				
					sprdum1++;
					sprdum2++;			
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col2;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;	
					break;
				case 0x2:
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col3;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;					
					sprdum0++;					
					sprdum1++;
					sprdum2++;
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col3;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;	
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;			
					break;
				case 0x1:
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col1;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;
					sprdum0++;
					sprdum1++;
					sprdum2++;			
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col1;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;
					break;
				default:
					sprdum0++;
					sprdum1++;
					sprdum2++;
					break;	
			}
			p-=2;
			sprdum0++;
			sprdum1++;
			sprdum2++;
			testbitx>>=2;
		}
	}
}

/*
**
**	Draws a multi colour sprite with horizontal expansion
**
*/
void DrawMulticolorSpriteX2(void)
{
	byte y;
	dword testbitx,x,p;
	byte check=*spdatp&sp2;
	sprdum0=gcoll;
	sprdum1=spcoll;
	sprdum2=mnshline;
	for(y=0;y<3;y++)
	{
		testbitx=0xc000;
		p=14;
		bitpattern=MultiColorTable[*sprite++];
		while(testbitx)
		{
			x=(bitpattern&testbitx)>>p;
			switch(x)
			{
				case 0x3:
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col2;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;
					sprdum0++;				
					sprdum1++;
					sprdum2++;			
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col2;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;	
					break;
				case 0x2:
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col3;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;					
					sprdum0++;					
					sprdum1++;
					sprdum2++;
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col3;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;			
					break;
				case 0x1:
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col1;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;
					sprdum0++;
					sprdum1++;
					sprdum2++;			
					if(*sprdum1==0)				
					{								
						if((check==0)||(*sprdum0==0))	
						{						
							*sprdum2=col1;		
						}						
					}							
					else							
					{							
						*sp2sp|=*sprdum1|sp2;			
					}							
					if(*sprdum0==1)				
					{							
						*sp2dat|=sp2;				
					}							
					*sprdum1|=sp2;
					break;
				default:
					sprdum0++;
					sprdum1++;
					sprdum2++;
					break;	
			}
			p-=2;
			sprdum0++;
			sprdum1++;
			sprdum2++;
			testbitx>>=2;
		}
	}
}

/*
**
**	DumpSprites: a diagnostic routine for viewing sprites
**
*/
void DumpSprites(void)
{
	byte t;
	byte *spritz;
	dword h=0;
	gcoll=sp2gfxcol;
	memset(gcoll,0,340);
	spcoll=sp2spcol;
	memset(spcoll,0,sizeof(sp2spcol));
	col1=VIC2CHIP[0x25]&0xf;
	col2=VIC2CHIP[0x26]&0xf;
	image=0;
	py=0x10;
	rawkey=0;
	delay(500);
	for(h=0;h<21;h++)
	{
		mnshline=GfxLine;
		memset(mnshline,0,320);
		spcoll=sp2spcol;
		memset(spcoll,0,sizeof(sp2spcol));
		sp2=1;
		sp=0;
		for(t=0;t<8;t++)
		{
			col3=VIC2CHIP[0x27+sp]&0xf;
			py&=0xff;
			px=(sp+1)*24;
			px&=0x1ff;
			spritz=RAM+Spriteptrs+sp;
			sprite=RAM+VicBank+(h*3)+(*(spritz)<<6);
			mnshline=GfxLine+px;
			spcoll=sp2spcol+px;
			gcoll=sp2gfxcol+px;
			if(*sprmcm&sp2)
			{
				DrawMulticolorSprite();
			}
			else
			{
				bitpattern=*sprite++;
				STD_SPRITE(col3);
				bitpattern=*sprite++;
				STD_SPRITE(col3);
				bitpattern=*sprite++;
				STD_SPRITE(col3);
			}
			sp2<<=1;
			sp=(sp++)&0x7;
		}
		_movedatal(_my_ds(),(unsigned) GfxLine, _dos_ds, 0xa0000+(py*320), 80);
		py++;
	}
	while(rawkey!=MAKE_F9);
	rawkey=0;
	delay(500);
}

/*
**
**	DrawSprites: 
**
*/
void DrawSprites(void)
{
	byte t;
	byte *spritz=RAM+Spriteptrs;		/* base of sprite pointers */
	__asm__ __volatile__("
		pushal					\n	
		cld						\n
		movl		$_sp2spcol,%%ebx		\n
		xorl		%%eax,%%eax			\n
		movl		%%ebx,_spcoll		\n
		movl		%%ebx,%%edi			\n
		movl		$0xa0,%%ecx			\n
		rep						\n
		stosl						\n
		movl	$_VIC2CHIP,%%ebx			\n
		movl	%%ebx,%%edx				\n
		movl	$0x0f0f,%%ecx			\n
		movl	0x25(%%ebx),%%eax			\n
		addl	$64,%%edx				\n
		addl	$96,%%ebx				\n
		andl	%%ecx,%%eax				\n		
		movb	%%al,_col1				\n
		movb	%%ah,_col2				\n
		movl	%%ebx,_ptry				\n
		movl	%%edx,_ptrx				\n				
		popal		"
		:
		:
		);
	sp2=1;
	sp=0;
	image=0;
	MCBASE=RAM+VicBank;

	for(t=0;t<8;t++)
	{
		py=*ptry++;
		px=*ptrx++;
		if((*sprena&sp2)&&(MC[sp]<63))
		{
			col3=VIC2CHIP[0x27+sp]&0xf;

			sprite=MCBASE+MC[sp]+(*(spritz)<<6);
			mnshline=GfxLine+px;
			spcoll=sp2spcol+px;
			gcoll=sp2gfxcol+px;
			image+=2;
			if(*sprmcm&sp2)
			{
				if(*sprexpx&sp2)
				{

					DrawMulticolorSpriteX2();
				}
				else
				{
					DrawMulticolorSprite();
				}
			}
			else
			{
				if(*sprexpx&sp2)
				{
					DrawPlaneSpriteX2();
				}
				else
				{
					DrawPlaneSprite();
				}
			}
			if(*sprexpy&sp2)
			{
				if(MC[sp+8]&0x01)
				{
					MC[sp]+=3;
				}
				MC[sp+8]+=1;
			}
			else
			{
				MC[sp]+=3;
				MC[sp+8]+=1;
			}
		}
		if(py==mnsvline)
		{
			MC[sp]=MC[sp+8]=0;
		}
		sp2<<=1;
		sp=(sp++)&0x7;
		spritz++;
	}
	sp=VIC2CHIP[0x1a];
	if(*sp2sp)
	{
		VIC2CHIP[0x19]|=0x4;
		if(sp&0x4)
		{
			VIC2CHIP[0x19]|=0x80;
			cpu.irq|=0xff0000;
		}
	}
	if(*sp2dat)
	{
		VIC2CHIP[0x19]|=0x2;
		if(sp&0x2)
		{
			VIC2CHIP[0x19]|=0x80;
			cpu.irq|=0xff0000;
		}
	}
	if(mnsvline==255)
	{
		/* if beem mnsvline is at bottom of paper, so turn off sprites */ 
		ptr2MC=MC;
		*ptr2MC++=63;
		*ptr2MC++=63;
		*ptr2MC++=63;
		*ptr2MC++=63;

		*ptr2MC++=63;
		*ptr2MC++=63;
		*ptr2MC++=63;
		*ptr2MC++=63;
	
		*ptr2MC++=63;
		*ptr2MC++=63;	
	}
}

/*
**
**	InitVicTables: Computes lookup tables for Sprite Expansion and Multicolor Mode
**
*/
void InitVicTables(struct screen *the_screen)
{
	dword i=0;
	dword temp1,temp2;
	VCBASE=VCCOUNT=RC=0;
	vmliptr=VMLI;
	vmlcptr=VMLC;
	in_screen_window=0;
	in_dma_window=0;
	pc_screen=the_screen;
	pc_bitmap=&my_line;
	
	pc_bitmap->width=pc_screen->width;
	pc_bitmap->height=1;
	pc_bitmap->bitmap_ptr=0;
	pc_bitmap->bitmap_bkg=0;
	
	our_global_selector=_dos_ds;
	for(i=0;i<256;i++)
	{
		temp1=temp2=0;
		if(i&0x01)
		{
			temp1|=0x0003;
			temp2|=0x0005;
		}
		if(i&0x02)
		{
			temp1|=0x000c;
			temp2|=0x000a;
		}
		if(i&0x04)
		{
			temp1|=0x0030;
			temp2|=0x0050;
		}
		if(i&0x08)
		{
			temp1|=0x00c0;
			temp2|=0x00a0;
		}
		if(i&0x10)
		{
			temp1|=0x0300;
			temp2|=0x0500;
		}
		if(i&0x20)
		{
			temp1|=0x0c00;
			temp2|=0x0a00;
		}
		if(i&0x40)
		{
			temp1|=0x3000;
			temp2|=0x5000;
		}
		if(i&0x80)
		{
			temp1|=0xc000;
			temp2|=0xa000;
		}
		StandardColorTable[i]=temp1;
		MultiColorTable[i]=temp2;		
	}
}

/*
**	Draw one rasterline of current display mode
*/
void DrawC64DisplayMode(void)
{
	if((mnsvline>=DelYstart)&&(mnsvline<=DelYend)&&display)
	{
		in_deltay_window=0xff;
		if(!idlestate_flag)
		{
			switch(screenmode)
			{
				case 0x00:
					StandardTxtMode();
					break;
				case 0x01:
					MultiColorTxtMode();
					break;
				case 0x02:
					StandardBitMapMode();
					break;
				case 0x03:
					MultiColorBitMapMode();
					break;
				case 0x04:
					ExtendedTxtMode();
					break;
				case 0x05:
					InvalidTxtMode();
					break;
				case 0x06:
					InvalidBitMapMode1();
					break;
				case 0x07:
					InvalidBitMapMode2();
				break;
			}
		}
		else
		{
			InvalidBitMapMode1();
		}
	}
}

void ComputeBadline(void)
{
	badline_flag=0;
	VCCOUNT=VCBASE;
	in_screen_window=0xff;
	scrolly=txtmode&0x7;
	/* is the line within dma start and end? */
	if((mnsvline>=DISPLAY_DMA_START)&&(mnsvline<=DISPLAY_DMA_END))
	{
		in_dma_window=0xff;
		/* is this a badline? */
		if((scrolly==(mnsvline&0x7))&&(badline_enable_flag))
		{
			RC=0;
			cpu_badline_cycles=23;
			display=screen_enable_flag;
			idlestate_flag=0x00;
			badline_flag=0xff;
			UpdateVIC2();
		}
	}
}

void RenderUnchainedMode(void)
{
	/* we in unchained mode */
	if((mnsvline>=31)&&(mnsvline<=271))
	{

		/* work out where in display window raster is */
		raster=(mnsvline-31);
		raster=((raster<<8)+(raster<<6)+(raster<<5)+(raster<<3))>>2;
		gcoll=raster+0xa0000;
		/*draw left and right border */
		outp(SC_INDEX, MAP_MASK);
		outp(SC_DATA,1+2+4+8);				/* select all 4 planes */
		__asm__ __volatile__("
			movw		_our_global_selector,%%fs	\n
			movl		_gcoll,%%ebx			\n
			movl		_BorderColor,%%eax		\n	
			movl		%%ebx,%%edx				\n

			addl		$85,%%edx				\n

			movl		%%eax,%%fs:(%%ebx)		\n
			movl		%%eax,%%fs:(%%edx)		\n
			addl		$4,%%ebx				\n
			addl		$4,%%edx				\n
			movb		%%al,%%fs:(%%ebx)		\n
			movb		%%al,%%fs:(%%edx)		\n
			"
			:
			:
			);
		/* draw main screen area with left & right border */
		if((mnsvline>=51)&&(mnsvline<=251))
		{
			mnshline=GfxLine+24;
			gcoll=raster+0xa0005;
			for(raster=0;raster<4;raster++)
			{
				outp(SC_INDEX, MAP_MASK);
				outp(SC_DATA,0x01<<raster);
				__asm__ __volatile__("
					movw		_our_global_selector,%%fs	\n
					movl		_gcoll,%%ebx			\n
					movl		_mnshline,%%edi			\n
					movl		$80,%%ecx				\n
					drawer:						\n
					movb		(%%edi),%%al			\n
					movb		%%al,%%fs:(%%ebx)			\n
					incl		%%ebx					\n
					addl		$4,%%edi				\n
					loop		drawer				\n
					"
					:
					:
					); 
				mnshline++;
			}
		}
		else
		{
			/* the raster is in upper or lower border */
			/* draw border colour to all 4 planes     */
			gcoll=raster+0xa0005;
			outp(SC_INDEX, MAP_MASK);
			outp(SC_DATA,1+2+4+8);
			__asm__ __volatile__("
				movw		_our_global_selector,%%fs	\n
				movl		_gcoll,%%ebx			\n
				movl		_BorderColor,%%eax		\n
				movl		$20,%%ecx				\n
				drawerborder:					\n
				movl		%%eax,%%fs:(%%ebx)		\n
				addl		$4,%%ebx				\n
				loop		drawerborder			\n
				"
				:
				:
				);
		}	
	}
}

void RenderChainedMode(void)
{
	if((mnsvline>=51)&&(mnsvline<=251))
	{
		raster=(mnsvline-51);
		raster=((raster<<8)+(raster<<6));
		mnshline=GfxLine+24;
		gcoll=raster+0xa0000;
		__asm__ __volatile__("
		movw		_our_global_selector,%%fs	\n
		movl		_gcoll,%%ebx			\n
		movl		_mnshline,%%edi			\n
		movl		$80,%%ecx				\n
		drawer256:						\n
		movl		(%%edi),%%eax			\n
		movl		%%eax,%%fs:(%%ebx)		\n
		addl		$4,%%ebx				\n
		addl		$4,%%edi				\n
		loop		drawer256				\n
			"
			:
			:
			);
			
	}
	
}

void VicII(void)
{
	VIC2();
	cpu_badline_cycles=cpu_base_cycles;
	if((mnsvline>=FIRST_RASTER_LINE)&&(mnsvline<=LAST_RASTER_LINE))
	{
		ComputeBadline();	
		/* is display turned on? */ 
		if((mnsvline>=DISPLAY_ROW25_START)&&(mnsvline<=DISPLAY_ROW25_END))		
		{

			mnshline=GfxLine+scrollx;
			gcoll+=scrollx;
			gfxchar=RAM+CharBank;
			if((bankid==0)||(bankid==2))
			{
				if(matrixid==6) gfxchar=PixelRom+0x800;
				if(matrixid==4) gfxchar=PixelRom;
			}
				
			if(txtmode&0x8)
			{
				DelYstart=DISPLAY_ROW25_START;
				DelYend=DISPLAY_ROW25_END;
			}
			else
			{
				DelYstart=DISPLAY_ROW24_START;
				DelYend=DISPLAY_ROW24_END;
			}
			
			DrawC64DisplayMode();
			DrawSprites();
			mnshline=GfxLine+24;
			if((!in_deltay_window)||(!(txtmode&DEN)))
			{
				__asm__ __volatile__("
				pushal					\n	
				movl		_mnshline,%%edi		\n
				movl		_BorderColor,%%eax	\n
				movl		$0x50,%%ecx			\n
				rep						\n
				stosl						\n
				popal		"
				:
				:
				);
			}

			if(!(scrmode&0x8))
			{
				mnshline=GfxLine+24;
				__asm__ __volatile__("
					pushal					\n	
					movl		_mnshline,%%edx		\n
					movl		_BorderColor,%%eax	\n
					movl		%%edx,%%ebx			\n
					addl		$0x139,%%edx		\n
					movl		%%eax,(%%edx)		\n
					movl		%%eax,(%%ebx)		\n
					movl		%%eax,4(%%edx)		\n
					movl		%%eax,4(%%ebx)		\n
					popal		"
					:
					:
					);
			}
		}
		else
		{
			if(mnsvline<=255)
			{
				mnshline=GfxLine+24;
				DrawSprites();
			}
		}
		
		if(RC==0x7)
		{
			idlestate_flag=0xff;
		}
		else
		{
			RC=(RC++)&0x7;
		}
		
		if(badline_flag)
		{
			idlestate_flag=0x00;
			VCBASE+=40;
		}
		
		badline_flag=0;

		if(mnsvline==0xfb)
		{
			VCBASE=VCCOUNT=0;
			RC=0;
			idlestate_flag=0;
			badline_flag=0;
			badline_enable_flag=0;
			display=0;
			gcoll=sp2gfxcol;
			
			/* clear collision buffer */
			__asm__ __volatile__("
				pushal	\n	
				cld		\n
				xorl		%%eax,%%eax\n
				movl		_gcoll,%%edi\n
					
				movl		$0x60,%%ecx\n
				rep\n
				stosl\n

				popal		"
				:
				:
				);
		}
	}

	/* Render C64 raster line to PC raster line */
	if(pc_screen->mode)
	{
		RenderUnchainedMode();
	}
	else
	{
		RenderChainedMode();
	}
	
	cputimer=cpu_badline_cycles;

	/* Check for C64 verticle blank */	
	if(mnsvline==311)
	{
		ticks=uclock_read();
		ztime=ticks-oldticks;	
		if(ztime<t_50Hz)
		{
			ztime2=t_50Hz-ztime2;
			while(ztime<ztime2)
			{
				ticks=uclock_read();
				ztime=ticks-oldticks;
			}
			ztime2=0;
		}
		else
		{
			if(ztime>t_50Hz)
			{
				ztime2=ztime-t_50Hz;
			}
			if(ztime2>=t_50Hz) ztime2=t_50Hz;
		}

		/* wait for VGA verticle blank */
		if(pc_screen->vblank)
		{
			oldticks=ticks;		
			while((inportb(0x3da)&0x08)!=0)
			{
			}
			while((inportb(0x3da)&0x08)==0)
			{
			}
		}
	}

}
