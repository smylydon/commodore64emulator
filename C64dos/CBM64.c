#include "include/mytypes.h"
#include <dos.h>
#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/farptr.h>
#include <sys/segments.h>
#include <sys/movedata.h>
#include <dir.h>
#include "include/CBM64.h"

dword ticks=0;
dword oldticks=0;
dword oldtime=0;
dword oldclk=0;
dword dirprgmode=0;
dword joystick=0;
dword	joy1=0;
dword	joy2=0;
dword pause=0;
dword	cpu_base_cycles=CPUCYCLES;
dword	cia_base_cycles=CIACYCLES;
dword	vic_base_lines=VICLINES;

dword KILLEMU=0;

extern dword KILLGUI;
extern dword mypalette;
extern float gfactor;
extern float gamma;
extern byte rawkey;

byte music_flag=0;
byte music_on=0;

/*
WRITE TO PORT A               READ PORT B (56321, $DC01)
56320/$DC00
         Bit 7   Bit 6   Bit 5   Bit 4   Bit 3   Bit 2   Bit 1   Bit 0

Bit 7    STOP    Q       C=      SPACE   2       CTRL    <-      1

Bit 6    /       ^       =       RSHIFT  HOME    ;       *       LIRA

Bit 5    ,       @       :       .       -       L       P       +

Bit 4    N       O       K       M       0       J       I       9

Bit 3    V       U       H       B       8       G       Y       7

Bit 2    X       T       F       C       6       D       R       5

Bit 1    LSHIFT  E       S       Z       4       A       W       3

Bit 0    CRSR DN F5      F3      F1      F7      CRSR RT RETURN  DELETE
*/
	
byte keycolumn[]={
		0x0f,			// 0x00	NULL
		0x0f,			// 0x01	ESC
		0x07,			// 0x02	1
		0x07,			// 0x03	2
		0x01,			// 0x04	3
		0x01,			// 0x05	4
		0x02,			// 0x06	5
		0x02,			// 0x07	6
		0x03,			// 0x08	7
		0x03,			// 0x09	8
		0x04,			// 0x0A	9
		0x04,			// 0x0B	0
		0x05,			// 0x0C	MINUS
		0x05,			// 0x0D	EQUALS -> PLUS 
		0x00,			// 0x0E	BKSP   -> DELETE
		0x07,			// 0x0F	TAB    -> STOP
			
		0x07,			// 0x10	Q
		0x01,			// 0x11	W
		0x01,			// 0x12	E
		0x02,			// 0x13	R
		0x02,			// 0x14	T
		0x03,			// 0x15	Y
		0x03,			// 0x16	U
		0x04,			// 0x17	I
		0x04,			// 0x18	O
		0x05,			// 0x19	P
		0x05,			// 0x1A	[ -> @
		0x06,			// 0x1B	] -> *
		0x00,			// 0x1C	ENTER
		0x07,			// 0x1D	CRTL
		0x01,			// 0x1E	A
		0x01,			// 0x1F	S
			
		0x02,			// 0x20	D
		0x02,			// 0x21	F
		0x03,			// 0x22	G
		0x03,			// 0x23	H
		0x04,			// 0x24	J
		0x04,			// 0x25	K
		0x05,			// 0x26	L
		0x05,			// 0x27	COLON->COLON
		0x06,			// 0x28	APOS -> SEMI
		0x07,			// 0x29	TILD -> <-
		0x01,			// 0x2A	LSHIFT
		0x06,			// 0x2B	EQUALS
		0x01,			// 0x2C	Z
		0x02,			// 0x2D	X
		0x02,			// 0x2E	C
		0x03,			// 0x2F	V
	
		0x03,			// 0x30	B
		0x04,			// 0x31	N
		0x04,			// 0x32	M
		0x05,			// 0x33	COMA
		0x05,			// 0x34	PERIOD
		0x06,			// 0x35	SLASH
		0x06,			// 0x36	RSHIFT
		0x0f,			// 0x37	PRTSCR
		0x07,			// 0x38	ALT -> C=
		0x07,			// 0x39	SPACE
		0x07,			// 0x3A	CAPS
		0x00,			// 0x3B	F1 -> F1
		0x0f,			// 0x3C	F2
		0x00,			// 0x3D	F3 -> F3
		0x0f,			// 0x3E	F4
		0x00,			// 0x3F	F5 -> F5
			
		0x0f,			// 0x40	F6
		0x00,			// 0x41	F7 -> F7
		0x0f,			// 0x42	F8
		0x0f,			// 0x43	F9
		0x0f,			// 0x44	F10
		0x0f,			// 0x45	NUMLCK
		0x0f,			// 0x46	SCRLLCK
		0x0f,			// 0x47	HOME -> HOME
		0x0f,			// 0x48	UP
		0x0f,			// 0x49	PGEUP
		0x0f,			// 0x4A	GREY- -> MINUS
		0x0f,			// 0x4B	LEFT
		0x0f,			// 0x4C	CENTER
		0x00,			// 0x4D	RIGHT ->CRSERGT
		0x0f,			// 0x4E	GREY+ -> PLUS
		0x0f,			// 0x4F	END
			
		0x00,			// 0x50	DOWN ->CRSEDN
		0x0f,			// 0x51	PGEDN
		0x0f,			// 0x52	INS
		0x0f,			// 0x53	DEL -> EXP
		0x0f,			// 0x54	NOTHING
		0x0f,			// 0x55	NOTHING
		0x0f,			// 0x56	NOTHING
		0x0f,			// 0x57	F11
		0x0f,			// 0x58	F12
		0x06,			// 0x59	NOTHING->home
		0x0f,			// 0x5A	NOTHING
		0x0f,			// 0x5B	NOTHING
		0x0f,			// 0x5C	
		0x0f,			// 0x5D	
		0x0f,			// 0x5E	NOTHING
		0x0f,			// 0x5F	NOTHING
			
		0x0f,			// 0x60	NOTHING
		0x0f,			// 0x61	
		0x0f,			// 0x62	
		0x0f,			// 0x63	NOTHING
		0x0f,			// 0x64	NOTHING
		0x0f,			// 0x65	NOTHING
		0x0f,			// 0x66	
		0x0f,			// 0x67	
		0x0f,			// 0x68	NOTHING
		0x0f,			// 0x69	
		0x0f,			// 0x6A	
		0x0f,			// 0x6B	NOTHING
		0x0f,			// 0x6C	NOTHING
		0x0f,			// 0x6D	NOTHING
		0x0f,			// 0x6E	
		0x0f,			// 0x6F
			
		0x0f,			// 0x70	NOTHING
		0x0f,			// 0x71	
		0x0f,			// 0x72	
		0x0f,			// 0x73	NOTHING
		0x0f,			// 0x74	NOTHING
		0x0f,			// 0x75	NOTHING
		0x0f,			// 0x76	
		0x0f,			// 0x77	
		0x0f,			// 0x78	NOTHING
		0x0f,			// 0x79	
		0x0f,			// 0x7A	
		0x0f,			// 0x7B	NOTHING
		0x0f,			// 0x7C	NOTHING
		0x0f,			// 0x7D	NOTHING
		0x0f,			// 0x7E	
		0x0f			// 0x7F	
};

byte keyrow[]={
	0x0f,			// 0x00	NULL
	0x0f,			// 0x01	ESC
	0x00,			// 0x02	1
	0x03,			// 0x03	2
	0x00,			// 0x04	3
	0x03,			// 0x05	4
	0x00,			// 0x06	5
	0x03,			// 0x07	6
	0x00,			// 0x08	7
	0x03,			// 0x09	8
	0x00,			// 0x0A	9
	0x03,			// 0x0B	0
	0x00,			// 0x0C	MINUS
	0x03,			// 0x0D	EQUALS -> PLUS 
	0x00,			// 0x0E	BKSP   -> DELETE
	0x02,			// 0x0F	TAB    -> STOP
		
	0x06,			// 0x10	Q
	0x01,			// 0x11	W
	0x06,			// 0x12	E
	0x01,			// 0x13	R
	0x06,			// 0x14	T
	0x01,			// 0x15	Y
	0x06,			// 0x16	U
	0x01,			// 0x17	I
	0x06,			// 0x18	O
	0x01,			// 0x19	P
	0x06,			// 0x1A	[ -> @
	0x01,			// 0x1B	] -> *
	0x01,			// 0x1C	ENTER
	0x02,			// 0x1D	CRTL
	0x02,			// 0x1E	A
	0x05,			// 0x1F	S
		
	0x02,			// 0x20	D
	0x05,			// 0x21	F
	0x02,			// 0x22	G
	0x05,			// 0x23	H
	0x02,			// 0x24	J
	0x05,			// 0x25	K
	0x02,			// 0x26	L
	0x05,			// 0x27	COLON->COLON
	0x02,			// 0x28	APOS -> SEMI
	0x01,			// 0x29	TILD -> <-
	0x07,			// 0x2A	LSHIFT
	0x05,			// 0x2B	EQUALS
	0x04,			// 0x2C	Z
	0x07,			// 0x2D	X
	0x04,			// 0x2E	C
	0x07,			// 0x2F	V
		
	0x04,			// 0x30	B
	0x07,			// 0x31	N
	0x04,			// 0x32	M
	0x07,			// 0x33	COMA
	0x04,			// 0x34	PERIOD
	0x07,			// 0x35	/
	0x04,			// 0x36	RSHIFT
	0x01,			// 0x37	PRTSCR
	0x05,			// 0x38	ALT -> C=
	0x04,			// 0x39	SPACE
	0x07,			// 0x3A	CAPS
	0x04,			// 0x3B	F1 -> F1
	0x0f,			// 0x3C	F2
	0x05,			// 0x3D	F3 -> F3
	0x0f,			// 0x3E	F4
	0x06,			// 0x3F	F5 -> F5
		
	0x0f,			// 0x40	F6
	0x03,			// 0x41	F7 -> F7
	0x0f,			// 0x42	F8
	0x0f,			// 0x43	F9
	0x0f,			// 0x44	F10
	0x0f,			// 0x45	NUMLCK
	0x0f,			// 0x46	SCRLLCK
	0x03,			// 0x47	HOME -> HOME
	0x0f,			// 0x48	UP
	0x0f,			// 0x49	PGEUP
	0x0f,			// 0x4A	GREY- -> MINUS
	0x05,			// 0x4B	LEFT
	0x0f,			// 0x4C	CENTER
	0x02,			// 0x4D	RIGHT ->CRSERGT
	0x0f,			// 0x4E	GREY+ -> PLUS
	0x0f,			// 0x4F	END

	0x07,			// 0x50	DOWN ->CRSEDN
	0x0f,			// 0x51	PGEDN
	0x0f,			// 0x52	INS
	0x0f,			// 0x53	DEL -> EXP
	0x0f,			// 0x54	NOTHING
	0x0f,			// 0x55	NOTHING
	0x0f,			// 0x56	NOTHING
	0x0f,			// 0x57	F11
	0x0f,			// 0x58	F12
	0x03,			// 0x59	NOTHING ->home
	0x0f,			// 0x5A	NOTHING
	0x0f,			// 0x5B	NOTHING
	0x0f,			// 0x5C	
	0x0f,			// 0x5D	
	0x0f,			// 0x5E	NOTHING
	0x0f,			// 0x5F	NOTHING
		
	0x0f,			// 0x60	NOTHING
	0x0f,			// 0x61	
	0x0f,			// 0x62	
	0x0f,			// 0x63	NOTHING
	0x0f,			// 0x64	NOTHING
	0x0f,			// 0x65	NOTHING
	0x0f,			// 0x66	
	0x0f,			// 0x67	
	0x0f,			// 0x68	NOTHING
	0x0f,			// 0x69	
	0x0f,			// 0x6A	
	0x0f,			// 0x6B	NOTHING
	0x0f,			// 0x6C	NOTHING
	0x0f,			// 0x6D	NOTHING
	0x0f,			// 0x6E	
	0x0f,			// 0x6F
		
	0x0f,			// 0x70	NOTHING
	0x0f,			// 0x71	HOME KEYPAD ->HOME	
	0x0f,			// 0x72	
	0x0f,			// 0x73	NOTHING
	0x0f,			// 0x74	NOTHING
	0x0f,			// 0x75	NOTHING
	0x0f,			// 0x76	
	0x0f,			// 0x77	
	0x0f,			// 0x78	NOTHING
	0x0f,			// 0x79	
	0x0f,			// 0x7A	
	0x0f,			// 0x7B	NOTHING
	0x0f,			// 0x7C	NOTHING
	0x0f,			// 0x7D	NOTHING
	0x0f,			// 0x7E	
	0x0f			// 0x7F	
};

struct screen my_screen;
struct screen *main_screen;

struct guiitems mainmenuitems[]=
{
	{"    GAME    ",0,0},
	{"   JOYPORT  ",0,0},
	{"    CHIP    ",0,0},
	{"   SCREEN   ",0,0},
	{"   DEFAULT  ",0,0},
	{"    ABOUT   ",0,0},
	{"    HELP    ",0,0},
	{"    QUIT    ",0,0},
	{NULL,NULL,NULL}
};

struct guiitems gamemenuitems[]=
{
	{" LOAD T64 ",0,0},
	{" LOAD P00 ",0,0},
	{" LOAD C64 ",0,0},
	{NULL,NULL,NULL}
};

struct guiitems defaultmenuitems[]=
{
	{"                     ",0,0},
	{"   DEFAULT  SYSTEM   ",0,0},
	{"                     ",0,0},
	{NULL,NULL,NULL}
};

struct guiitems chipmenuitems[]=
{
	{" MNS MONAM     ",0,0},
	{" CPU CYCLES    ",0,0},
	{" CIA CYCLES    ",0,0},
	{" SPEEDO OFF    ",0,0},
	{" LOAD STATE    ",0,0},
	{" SAVE STATE    ",0,0},
	{NULL,NULL,NULL}
};

struct guiitems joymenuitems0[]=
{
	{" JOYPORT NONE ",0,0},
	{NULL,NULL,NULL}
};
struct guiitems joymenuitems1[]=
{
	{" JOYPORT ONE  ",0,0},
	{NULL,NULL,NULL}
};

struct guiitems joymenuitems2[]=
{
	{" JOYPORT TWO  ",0,0},
	{NULL,NULL,NULL}
};


struct guiitems colmenuitems1[]=
{
	{" 320X200  ",0,0},
	{NULL,NULL,NULL}
};

struct guiitems colmenuitems2[]=
{
	{" 360X256  ",0,0},
	{NULL,NULL,NULL}
};

struct guiitems colormenuitems[]=
{
	{"  GAMMA   ",0,0},
	{" PALETTE  ",0,0},
	{" 360X256  ",0,0},
	{" VBLANK   ",0,0},
	{" DEFAULT  ",0,0},
	{NULL,NULL,NULL}
};

struct guiitems aboutmenuitems[]=
{
	{"  MNS64 V2.5  ",0,0},
	{" (C)1997-2001 ",0,0},
	{"    CHOPPER   ",0,0},
	{NULL,NULL,NULL}
};

struct guiitems helpmenuitems[]=
{
	{"INGAME KEYS:       ",0,0},
	{"                   ",0,0},
	{"ESC QUIT EMU       ",0,0},
	{"F8  LOAD REQUESTER ",0,0},
	{"F9  DUMP SPRITES   ",0,0},
	{"F10 KILL GUI       ",0,0},
	{"F12 HARD RESET     ",0,0},
	{NULL,NULL,NULL}
};

struct gui helpmenu=
{
	0,						// next menu
	helpmenuitems,				//first item
	80,80,0,0,					//dimensions
	0,1,0,					//select entries rc
	0,0,0,					//gui1 gui2 tmp
	GUIBACK,GUIFORE,GUISELECT,		//bkcol fcol scol
	GUIRIGHT,GUILEFT,GUILEFT,GUIRIGHT	//rcol lcol tcol bcol
};

struct gui aboutmenu=
{
	&helpmenu,					// next menu
	aboutmenuitems,				//first item
	110,80,0,0,					//dimensions
	0,1,0,					//select entries rc
	0,0,0,					//gui1 gui2 tmp
	GUIBACK,GUIFORE,GUISELECT,		//bkcol fcol scol
	GUIRIGHT,GUILEFT,GUILEFT,GUIRIGHT	//rcol lcol tcol bcol
};

struct gui defaultmenu=
{
	&aboutmenu,					// next menu
	defaultmenuitems,				//first item
	80,80,0,0,					//dimensions
	0,2,0,					//select entries rc
	0,0,0,					//gui1 gui2 tmp
	GUIBACK,GUIFORE,GUISELECT,		//bkcol fcol scol
	GUIRIGHT,GUILEFT,GUILEFT,GUIRIGHT	//rcol lcol tcol bcol
};

struct gui colormenu=
{
	&defaultmenu,				// next menu
	colormenuitems,				//first item
	110,80,0,0,					//dimensions
	0,2,0,					//select entries rc
	0,0,0,					//gui1 gui2 tmp
	GUIBACK,GUIFORE,GUISELECT,		//bkcol fcol scol
	GUIRIGHT,GUILEFT,GUILEFT,GUIRIGHT	//rcol lcol tcol bcol
};

struct gui chipmenu=
{
	&colormenu,					// next menu
	chipmenuitems,				//first item
	100,80,0,0,					//dimensions
	0,3,0,					//select entries rc
	0,0,0,					//gui1 gui2 tmp
	GUIBACK,GUIFORE,GUISELECT,		//bkcol fcol scol
	GUIRIGHT,GUILEFT,GUILEFT,GUIRIGHT	//rcol lcol tcol bcol
};

struct gui joymenu=
{
	&chipmenu,					// next menu
	joymenuitems0,				//first item
	100,90,0,0,					//dimensions
	0,0,0,					//select entries rc
	0,0,0,					//gui1 gui2 tmp
	GUIBACK,GUIFORE,GUISELECT,		//bkcol fcol scol
	GUIRIGHT,GUILEFT,GUILEFT,GUIRIGHT	//rcol lcol tcol bcol
};

struct gui gamemenu=
{
	&joymenu,					// next menu
	gamemenuitems,				//first item
	110,80,0,0,					//dimensions
	0,2,0,					//select entries rc
	0,0,0,					//gui1 gui2 tmp
	GUIBACK,GUIFORE,GUISELECT,		//bkcol fcol scol
	GUIRIGHT,GUILEFT,GUILEFT,GUIRIGHT	//rcol lcol tcol bcol
};

struct gui mainmenu=
{
	&gamemenu,					// next menu
	mainmenuitems,				//first item
	110,60,0,0,					//dimensions
	0,7,0,					//select entries rc
	0,0,0,					//gui1 gui2 tmp
	GUIBACK,GUIFORE,GUISELECT,		//bkcol fcol scol
	GUIRIGHT,GUILEFT,GUILEFT,GUIRIGHT	//rcol lcol tcol bcol
};

/*---------------------------------------------------------*/
/* Start up functions                                      */
/*---------------------------------------------------------*/

void setup(byte mode,struct screen *the_screen)
{
	init_vga();
	install_guis(&mainmenu, the_screen);
	install_sid();
	printf("guis installed\n");
	delay(2000);
	the_screen->vblank=0xff;
	setvga(mode,the_screen);
	set_palette();
	InitVicTables(the_screen);
	io_init();
}

void HARD_RESET()
{
	byte index;
	cpu.irq=cpu.nmi=0;
	Reset6510();
	Reset6569();
	Reset6526();
	sb_reset();
	for(index=0;index<255;index++)
	{
		keyarr[index]=0xff;
		invkeyarr[index]=0xff;
	}
}

void finish()
{
	dword inst,rate;
	float per,t2;
	t2=elapsed_time()/18.2;
	per=((cpu.clk/t2)/PERIOD)*100;
  	rate=(cpu.clk/t2);
	inst=(cpu.inst/t2);
	io_close();
	unsetvga();
	printf("PC  TIME            s         %d\n",(dword)t2);
	printf("EMU CYCLES                    %d\n",cpu.clk);
	printf("EMU CYCLES/PC time  Hz        %d\n",rate);
	printf("REAL CBM64 Freq.    Hz        %d\n",PERIOD); 
	printf("PERCENTAGE                    %d\n",(dword)per);
	printf("INSTRUCTIONS/TIME             %d\n",inst);
	uninstall_guis(&mainmenu);
}

void sortkeys()
{
	byte column,row;
	byte temp=rawkey&0x7f;
	switch(rawkey)
	{
		case MAKE_F8:
		case MAKE_F9:
		case MAKE_F10:
		case MAKE_F11:
		case MAKE_F12:
			break;
		default:
			column=keycolumn[temp];
			row=keyrow[temp];
			if((row==0xf)||(column==0xf)) break;
			if(rawkey<0x80)
			{
				keyarr[column]&=~(1<<row);
				invkeyarr[row]&=~(1<<column);

			}
			else
			{
				keyarr[column]|=(1<<row);
				invkeyarr[row]|=(1<<column);
			}
			break;
	}
}


void loadformat64x()
{
	byte *txt;
	struct fileselector *zfile;
	
	if(dirprgmode==0)
	{
		zfile=dirprg("*.t64");
	}
	if(dirprgmode==1)
	{
		zfile=dirprg("*.p00");
	}
	if(dirprgmode==2)
	{
		zfile=dirprg("*.prg");
	}
	if(dirprgmode==4)
	{
		zfile=dirprg("*.st");
	}
	txt=zfile->filename;
	if((zfile->entries!=0)&&(txt))
	{
		txt=zfile->filename;
		if(dirprgmode==0)
		{
			t64load(txt);
		}
		if(dirprgmode==1)
		{
			p00load(txt);
		}
		if(dirprgmode==2)
		{
			c64load(txt);
		}
		if(dirprgmode==4)
		{
			s64load(txt);
		}
	}
}

void saveformat64st()
{
	byte *txt;
	struct fileselector *zfile;
	dirprgmode=4;
	zfile=dirprg("*.st");
	txt=zfile->filename;
	if((zfile->entries!=0)&&(txt)) s64save(txt);
}

/*---------------------------------------------------------*/
/* Various gui handling routines start here	           */
/*---------------------------------------------------------*/
void gamegui()
{
	
	if((gamemenu.gui1)&&(gamemenu.gui2))
	{
		while(KILLGUI!=0xff)
		{
			switch(gui_handle(&gamemenu))
			{
				case	0x00:
					dirprgmode=0;
					loadformat64x();
					break;
				case	0x1:
					dirprgmode=1;
					loadformat64x();
					break;
				case	0x2:
					dirprgmode=2;
					loadformat64x();
					break;
				case  0xf10:
					KILLGUI=0xff;
					break;
				case  0xdead:
					KILLGUI=KILLEMU=0xff;
					break;
				default:
					break;
			}
		}
	}
	KILLGUI=0x00;
	rawkey=0;
}

void joygui()
{
//	joymenu.entries=0;
	if(joystick)
	{
		if(joystick==2)
		{
			joymenu.first=joymenuitems2;
		}
		else
		{
			joymenu.first=joymenuitems1;
		}
	}
	else
	{
			joymenu.first=joymenuitems0;
	}
	if((joymenu.gui1)&&(joymenu.gui2))
	{
		while(KILLGUI!=0xff)
		{
			switch(gui_handle(&joymenu))
			{
				case 0x00:
					if(joystick)
					{
						if(joystick==2)
						{	
							joymenu.first=joymenuitems1;
							joystick=1;
						}
						else
						{
							joymenu.first=joymenuitems2;
							joystick=2;
						}
					}
					else
					{
						joymenu.first=joymenuitems0;
					}
					break;
				case  0xf10:
					KILLGUI=0xff;
					break;
				case  0xdead:
					KILLGUI=KILLEMU=0xff;
					break;
				default:
					break;
			}										
		}
	}
	KILLGUI=0x00;
	rawkey=0;
}

void monamgui()
{
	struct gui mgui;
	mgui.xcord=0;
	mgui.ycord=0;
	mgui.height=256;
	mgui.width=370;
	mgui.gui1=malloc(400*300);
	if((mgui.gui1))
	{
		main_screen->screen_bkg=mgui.gui1;
		save_screen(main_screen);
		monam();
		setvga(main_screen->mode,main_screen);
		draw_screen(main_screen);
		free(mgui.gui1);
	}
	main_screen->screen_bkg=0;
	rawkey=0;
}

void correcttimers()
{
	if(cia_base_cycles<60) cia_base_cycles=70;
	if(cia_base_cycles>70) cia_base_cycles=60;
	if(cpu_base_cycles<60) cpu_base_cycles=70;
	if(cpu_base_cycles>70) cpu_base_cycles=60;
}

void chipgui()
{
	struct guiitems *current;
	byte digit1;
	byte digit2;
	byte *ciatxt;
	byte *cputxt;
	correcttimers();
	current=chipmenu.first->next;
	cputxt=current->item;
	current=current->next;
	ciatxt=current->item;
	digit1=(byte)cpu_base_cycles%10;
	digit2=(byte)cpu_base_cycles/10;
	*(cputxt+12)=digit2+48;
	*(cputxt+13)=digit1+48;
	digit1=(byte)cia_base_cycles%10;
	digit2=(byte)cia_base_cycles/10;
	*(ciatxt+12)=digit2+48;
	*(ciatxt+13)=digit1+48;
	if((chipmenu.gui1)&&(chipmenu.gui2))
	{
		while(KILLGUI!=0xff)
		{
			switch(gui_handle(&chipmenu))
			{
				case	0x00:
					monamgui();
					break;
				case	0x1:
					cpu_base_cycles--;
					correcttimers();
					digit1=cpu_base_cycles%10;
					digit2=cpu_base_cycles/10;
					*(cputxt+12)=digit2+48;
					*(cputxt+13)=digit1+48;
					break;
				case	0x2:
					cia_base_cycles--;
					correcttimers();
					digit1=cia_base_cycles%10;
					digit2=cia_base_cycles/10;
					*(ciatxt+12)=digit2+48;
					*(ciatxt+13)=digit1+48;
					break;
				case  0x4:
					dirprgmode=4;
					loadformat64x();
					break;
				case  0x5:
					dirprgmode=4;
					saveformat64st();
					break;
				case  0xf10:
					KILLGUI=0xff;
					break;
				case  0xdead:
					KILLGUI=KILLEMU=0xff;
					break;
				default:
					break;
			}
		}
	}
	KILLGUI=0x00;
}

void colorcorrect()
{
	if(gamma>15.0) gamma=0.1;
	if(gfactor>64.0) gfactor=1.0;
	if(mypalette>3) mypalette=0;
	set_palette();
}

void colordefault()
{
	mypalette=0;
	gamma=5.0;
	gfactor=16.0;
	colorcorrect();
}

void sortvblank(struct screen *the_screen,struct gui *the_gui)
{
	struct guiitems *current;
	byte *vblanker;
	current=the_gui->first;
	current=current->next;
	current=current->next;
	current=current->next;
	vblanker=current->item;
	vblanker+=8;
	if(the_screen->vblank)
	{
		*vblanker=65+24;
	}
	else
	{
		*vblanker=32;
	}
}

void sortres(struct screen *the_screen,struct gui *the_gui)
{
	struct guiitems *current,*res1,*res2;
	current=the_gui->first;
	current=current->next;
	current=current->next;
	res1=colmenuitems1;
	res2=colmenuitems2;
	if(the_screen->mode)
	{
		current->item=res2->item;
	}
	else
	{
		current->item=res1->item;
	}
}

void colorgui(struct screen *the_screen)
{
	sortvblank(the_screen,&colormenu);
	sortres(the_screen,&colormenu);
	if((colormenu.gui1)&&(colormenu.gui2))
	{
		while(KILLGUI!=0xff)
		{
			switch(gui_handle(&colormenu))
			{
				case	0x00:
					gamma+=0.1;
					colorcorrect();
					break;
				case	0x1:
					mypalette++;
					colorcorrect();
					break;
				case	0x2:
					if(the_screen->mode)
					{
						the_screen->mode=0x00;
					}
					else
					{
						the_screen->mode=0x01;
					}
					sortres(the_screen,&colormenu);
					setvga(the_screen->mode,the_screen);
					delay(500);
					break;
				case	0x3:
					if(the_screen->vblank)
					{
						the_screen->vblank=0x00;
					}
					else
					{
						the_screen->vblank=0xff;
					}
					sortvblank(the_screen,&colormenu);
					delay(500);
					break;
				case	0x4:
					colordefault();
					break;
				case  0xf10:
					KILLGUI=0xff;
					break;
				case  0xdead:
					KILLGUI=KILLEMU=0xff;
					break;
				default:
					break;
			}
		}
	}
	KILLGUI=0x00;
}

/*---------------------------------------------------------*/
/* Handling for the main menu                              */
/*---------------------------------------------------------*/
void maingui(struct screen *the_screen)
{
	pause=0xff;					/*emu is paused   */
	rawkey=0;
	delay(2);
	if((mainmenu.gui1)&&(mainmenu.gui2))
	{
		while((KILLGUI!=0xff)&&(KILLEMU!=0xff))
		{
			switch(gui_handle(&mainmenu))
			{
				case 0x00:
					gamegui();
					break;
				case	0x1:
					joygui();
					break;
				case	0x2:
					chipgui();
					break;
				case	0x3:
					colorgui(the_screen);
					break;
				case	0x4:
					gui_message(&defaultmenu,0);
					colordefault();
					break;
				case	0x5:
					gui_message(&aboutmenu,0xff);
					break;
				case	0x6:
					gui_message(&helpmenu,0xff);
					break;
				case  0xf10:
					KILLGUI=0xff;
					break;
				case  0xdead:
					KILLGUI=KILLEMU=0xff;
					break;
				default:
					break;
			}
		}
	}
	KILLGUI=0;
	rawkey=0;
	delay(1);
	pause=0;
}


int main(void)
{
	dword poll_count;
	joystick=0;
	main_screen=&my_screen;
	ciatimer=cia_base_cycles;
	victimer=vic_base_lines;
	cputimer=cpu_base_cycles;
	
	if(detect_joystick())
	{
		printf("Joystick found\n");
		joystick=0x2;
	}
	else
	{
		printf("no Joystick found\n");
		joystick=0;
	}
	
	poll_count=0;
	setup(1,main_screen);
	cpu.clk=cpu.inst=0;

	oldticks=ticks=uclock_read();
	joy1=joy2=0xff;
	HARD_RESET();
	KILLEMU=0x00;
	while(!KILLEMU)
	{
		/* sort out keyboard */
		switch(rawkey)
		{
			case MAKE_ESC:
				KILLEMU=0xff;
				break;
			case MAKE_F12:
				pause_time(0xff);
				HARD_RESET();
				pause_time(0x00);
				break;
			case MAKE_F10:
				pause_time(0xff);
				maingui(main_screen);
				pause_time(0x00);
				break;
			case MAKE_F9:
				pause_time(0xff);
				DumpSprites();
				pause_time(0x00);
				break;
			case MAKE_F8:
				pause_time(0xff);
				if(dirprgmode>2) dirprgmode=4;
				loadformat64x();
				pause_time(0x00);
				break;
		}
	
		if((joystick)&&(poll_count==933))
		{
			joy1=0xff;
			joy2=0xff;
			poll_count=0;
			if(joystick==1)
			{
				joy1=~(poll_joystick());
			}
			else
			{
				joy2=~(poll_joystick());
			}

		}
		M6510();
		VicII();
		sortkeys();
		CIA1();
		poll_count++;
	}
	finish();
	exit(0);
}
