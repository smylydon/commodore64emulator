/*-----------------------------------------------------------------------------------------*/
// Gui.c 
// Orignal (c) MNS 1998
// revised (c) MNS 1999
// revised (C) MNS 2000
/*-----------------------------------------------------------------------------------------*/
#include "include/mytypes.h"
#include <dpmi.h>
#include <dos.h>
#include <go32.h>
#include <pc.h>
#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <sys/farptr.h>
#include <sys/segments.h>
#include <sys/movedata.h>
#include "include/keys.h"
#include "include/M6510.h"
#include "include/modes.h"

/* GUI DEFINES */
#define FSELWIDTH		20*8
#define FSELHEIGHT	20*8
#define FSELHBYTES	20


#define GUIBACK	0x0c
#define GUIFORE	0x0f
#define GUILEFT	0x01
#define GUIRIGHT	0x0b
#define GUISELECT 0x0e

/* gui structure prototypes */

struct guiitems
{
	byte *item;					//item name
	struct guiitems *previous;		//prev item
	struct guiitems *next;			//next item
};

struct gui
{
	struct gui *nxtgui;			//next gui
	struct guiitems *first;			//first item
	dword xcord,ycord,width,height;	//dimensions
	dword select,entries,rc;		//cursor
	byte *gui1,*gui2,*tmp;			//rendering
	byte bkcol,fcol,scol;			//gui colours
	byte rcol,lcol,tcol,bcol;		//border colours
};

struct filer
{
	byte name[40];
	dword attrib;
	struct filer *next;
	struct filer *previous;
};

struct fileselector
{
	struct filer *first;
	struct filer *last;
	struct filer *current;
	dword xcord,ycord,width,height;
	dword select,scroller,entries,rc;
	byte *fsel1,*fsel2,*fstmp;
	byte bkcol,fcol,scol;
	byte rcol,lcol,tcol,bcol;
	byte filename[40];

};

// prototypes for gui routines

/* gui library function prototypes */
void	install_guis(struct gui *G,struct screen *myscreen);
void	uninstall_guis(struct gui *G);
void	gui_base(struct gui *a_gui_struct);
void	gui_cleanup(struct gui *a_gui_struct);
void	gui_draw(struct gui *a_gui_struct);
dword	gui_handle(struct gui *mygui);
void	gui_init(struct gui *a_gui_struct);
dword	gui_init_ptrs(struct guiitems *gui_items);
void	gui_message(struct gui *G,dword mode);
void	gui_text(struct gui *a_gui_struct,byte *text,dword sel_col);
void	gui_render(struct bitmap *a_bitmap);
void	gui_render_bkg(struct bitmap *a_bitmap);
void	gui_grab_bkg(struct bitmap *a_bitmap_struct);
void	gui_to_bitmap(struct gui *G,struct bitmap *B);
	
/* fileselect prototypes */
void install_fileselector();
void uninstall_fileselector();
struct fileselector* dirprg(byte *path);
void fsltmpl(struct fileselector *F);
void fselecthandler(struct fileselector *F);
void text2fsel(struct fileselector *F,byte *text,dword back);
void wrtfsel(struct fileselector *F);

void grabfsel(struct bitmap *B);
void renderfsel(struct bitmap *B);
void renderfsel_bkg(struct bitmap *B);
void fsel_to_bitmap(struct fileselector *F,struct bitmap *B);

// structures for each gui menu
struct guiitems *current,*temp;
struct gui *currentgui,*tempgui;
struct screen *gui_screen;

extern dword mypalette;
extern dword dirprgmode;

extern byte rawkey;
extern byte pause;
extern byte KILLEMU;

dword KILLGUI=0;

//The fileselector
struct fileselector myfiles;
struct bitmap gui_bitmap;

byte thefont[]={
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x01, 0x04, 0x01, 0x01, 0x04, 0x01, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x01, 0x04, 0x01, 0x01, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x01, 0x01, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x01, 0x01, 0x04, 0x01, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x01, 0x01, 0x04, 0x01, 0x01, 0x01,
	 0x00, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x01, 0x01, 0x01, 0x04, 0x01, 0x01, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x01, 0x00, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x00, 0x01, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x00, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x01, 0x04, 0x04, 0x01, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x01, 0x04, 0x04, 0x01, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x01, 0x04, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x00, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x00, 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x00, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x01, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x04, 0x04, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01,
	 0x00, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x00,
	 0x01, 0x01, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01,
	 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01,
	 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
/*-----------------------------------------------------------------------------------------*/
void my2upper(byte *str)
{
	char *p;
	p=str;
	while(*p)
	{
		*p=toupper(*p);
		p++;
	}
}

void fsel_to_bitmap(struct fileselector *F,struct bitmap *B)
{
	B->xcord=F->xcord+20;
	B->ycord=F->ycord+28;
	B->width=F->width;
	B->height=F->height;
	B->bitmap_ptr=F->fsel1;
	B->bitmap_bkg=F->fsel2;
	if(gui_screen->mode==0)
	{
		B->xinc=B->width;
	}
	else
	{
		B->xinc=B->width>>2;
	}
}

void text2fsel(struct fileselector *F,byte *text,dword back)
{
	byte txt[40];
	dword ch;
	dword rc=F->rc;
	byte *fselptr=F->fstmp;
	byte bit,x;
	while(*text)
	{  
		strcpy(txt,text);
		my2upper(txt);
		ch=((*txt)-0x21)&0xff;
		if(ch>57) ch=0;
		ch=(ch<<6)+(rc<<3);
		text++;
		for(x=0;x<8;x++)
		{
			bit=thefont[ch++];
			if(bit==4)
			{
				*fselptr++=1;
			}
			else
			{
				if(bit==1)
				{
					*fselptr++=0;
				}
				else
				{
					*fselptr++=back;
				}
			}
		} 
	}
}

void wrtfsel(struct fileselector *F)
{
	byte *txt;
	dword x=0;
	dword count=11;
	dword selection=0;
	struct filer *first;
	first=F->last;
	F->fstmp=(F->fsel1)+(11*FSELWIDTH)+3;
	while((count<(F->height-16))&&(first))
	{
		txt=first->name;
		for(x=0;x<8;x++)
		{
			F->rc=x;
			if(F->select==selection)
			{
				text2fsel(F,txt,GUISELECT);
			}
			else
			{
				text2fsel(F,txt,0);
			}
			count++;
			F->fstmp=(F->fstmp)+FSELWIDTH;
		}
		first=first->next;
		selection++;
	}
}

void fsltmpl(struct fileselector *F)
{
	dword x;
	F->fstmp=F->fsel1;
	memset(F->fstmp,0,(F->width*F->height)-1);
	memset(F->fstmp,F->bkcol,F->width*10);
	memset(F->fstmp,F->lcol,F->width);
	for(x=1;x<F->height;x++)
	{
		*F->fstmp=F->lcol;
		*(F->fstmp+(F->width-1))=F->rcol;
		F->fstmp+=F->width;
	}
	memset(F->fstmp,F->rcol,F->width);
}

void renderfsel(struct bitmap *B)
{
	draw_bitmap(B,gui_screen);
}

void renderfsel_bkg(struct bitmap *B)
{
	draw_bitmap_bkg(B,gui_screen);
}

void grabfsel(struct bitmap *B)
{
	save_bitmap_bkg(B,gui_screen);
}


void install_fileselector()
{
	myfiles.xcord=80;
	myfiles.ycord=25;
	myfiles.select=0;
	myfiles.height=FSELHEIGHT;
	myfiles.width=FSELWIDTH;
	myfiles.fsel1=malloc(myfiles.height*myfiles.width*sizeof(byte));
	myfiles.fsel2=malloc(myfiles.height*myfiles.width*sizeof(byte));
	myfiles.bkcol=GUIBACK;
	myfiles.fcol=GUIFORE;
	myfiles.scol=GUISELECT;
	myfiles.rcol=GUIRIGHT;
	myfiles.lcol=GUILEFT;
	myfiles.tcol=GUILEFT;
	myfiles.bcol=GUIRIGHT;
}

void uninstall_fileselector()
{
	if(myfiles.fsel1) free(myfiles.fsel1);
	if(myfiles.fsel2) free(myfiles.fsel2);
}

void fselecthandler(struct fileselector *F)
{
	dword KILLFSEL=0;

	F->select=0;
	F->scroller=0;
	while(!KILLFSEL)
	{
		if(rawkey==MAKE_ESC)
		{
			KILLGUI=KILLEMU=KILLFSEL=0xff;
			F->entries=0;
		}
		if(rawkey==MAKE_F10)
		{
			KILLFSEL=0xff;
			F->entries=0;
		}
		if(rawkey==MAKE_ENTER)
		{
			strcpy(F->filename,F->current->name);
			KILLGUI=KILLFSEL=0xff;
			rawkey=0;
			delay(1);
		}
		if(rawkey==MAKE_UP)
		{
			fsltmpl(F);
			if(F->select==0)
			{
				F->scroller=0;
				if(F->current->previous)
				{
					F->current=F->current->previous;
					if(F->last->previous) F->last=F->last->previous;
				}
			}
			else
			{

				if(F->current->previous)
				{
					F->current=F->current->previous;
					if(F->last->previous)
					{
						if((F->select==0)&&(F->current==F->last)) 
						{
							F->last=F->last->previous;
						}
					}
				}
				F->scroller--;
				F->select--;
			}
			wrtfsel(F);
			renderfsel(&gui_bitmap);
			rawkey=0;
			delay(1);
		}
		if(rawkey==MAKE_DOWN)
		{
			fsltmpl(F);
			if((F->scroller)>(F->entries-2))
			{

				F->scroller=F->entries-2;
				if(F->current->next)
				{
					F->select++;
					F->current=F->current->next;
					F->last=F->last->next;
				}
			}
			else
			{

				if(F->current->next)
				{
					F->select++;
					F->scroller++;
					F->current=F->current->next;
					if(F->select>16) F->last=F->last->next;
				}
			}
			if(F->select>=16) F->select=16;
			wrtfsel(F);
			renderfsel(&gui_bitmap);
			rawkey=0;
			delay(1);
		}
	}
	F->select=0;
	F->scroller=0;
}

struct fileselector* dirprg(byte *path)
{
	struct ffblk blk;
	struct filer *currentfiler,*firstfiler,*tempfiler;
	struct fileselector *fsel=&myfiles;
   	int done;
	dword error=0;
	dword numfiles=0;
	fsel->entries=0;
	currentfiler=firstfiler=tempfiler=0;
	rawkey=0;
	delay(3);
	if((fsel->fsel1)&&(fsel->fsel2))
	{	
	   	done = findfirst(path,&blk,0);
		fsel_to_bitmap(fsel,&gui_bitmap);
		while ((!done)&&(error!=0xff))
	   	{
			currentfiler=(struct filer *) malloc(sizeof(struct filer));
			if(firstfiler==0)
			{
				firstfiler=currentfiler;
				firstfiler->previous=0;
			}
			if(currentfiler)
			{
				strcpy(currentfiler->name,blk.ff_name);
				currentfiler->attrib=blk.ff_attrib;
				currentfiler->previous=tempfiler;
				if(tempfiler) tempfiler->next=currentfiler;
				tempfiler=currentfiler;
				numfiles++;
				done = findnext(&blk);
			}
			else
			{
				error=0xff;
				fsel->entries=0;
			}
		}
		currentfiler->next=0;
		if(error!=0xff)
		{
			fsel->current=firstfiler;
			fsel->last=firstfiler;
			fsel->entries=numfiles;
			grabfsel(&gui_bitmap);
			fsltmpl(fsel);
			wrtfsel(fsel);
			renderfsel(&gui_bitmap);
			fselecthandler(fsel);
			renderfsel_bkg(&gui_bitmap);
		}
		currentfiler=firstfiler;
		while(currentfiler)
		{
			tempfiler=currentfiler->next;
			free(currentfiler);
			currentfiler=tempfiler;
		}
	}
	rawkey=0;
	delay(1);
	return(&myfiles);	
}

/*---------------------------------------------------------*/
/* Initialise linked list of menu items                    */
/*---------------------------------------------------------*/
dword gui_init_ptrs(struct guiitems *items)
{
	dword x=0;
	current=items++;
	temp=items;
	current->previous=0;
	/* Initialising linked list */
	while(temp->item)
	{
		temp->previous=current;
		current->next=temp;
		current=temp;
		temp=items++;
		x++;
	}
	current->next=0;
	return(x);
}

/*---------------------------------------------------------*/
/* Calculate width,height & call gui_init_pointers         */
/*---------------------------------------------------------*/
void gui_init(struct gui *G)
{
	dword x=0;
	byte	*y=G->first->item;
	G->entries=(gui_init_ptrs(G->first))-1;
	while(*y++)
	{
		x++;
	}
	G->width=(x<<3)+6;
	G->height=((G->entries+1)<<3)+7;
	if(G->height<=8) G->height=15;
	G->gui1=malloc(G->height*G->width*sizeof(byte));
	G->gui2=malloc(G->height*G->width*sizeof(byte));
	G->rc=0;
	G->tmp=0;
	if(G->entries==0) G->select=0x00;
}

/*---------------------------------------------------------*/
/* Render a gui on to the screen			           */
/*---------------------------------------------------------*/
void gui_render(struct bitmap *B)
{
	draw_bitmap(B,gui_screen);
}
/*---------------------------------------------------------*/
/* Render a gui on to the screen			           */
/*---------------------------------------------------------*/
void gui_render_bkg(struct bitmap *B)
{
	draw_bitmap_bkg(B,gui_screen);
}
/*---------------------------------------------------------*/
/* Grab the current background for later replacement       */
/*---------------------------------------------------------*/
void gui_grab_bkg(struct bitmap *B)
{
	save_bitmap_bkg(B,gui_screen);	
}

/*---------------------------------------------------------*/
/* Draw the gui background                                 */
/*---------------------------------------------------------*/
void gui_base(struct gui *G)
{
	dword x;
	byte *guiptr=G->gui1;
	memset(guiptr,G->bkcol,((G->height)*(G->width))-1);
	memset(guiptr,G->lcol,G->width);
	guiptr+=G->width;
	for(x=1;x<G->height-1;x++)
	{
		*guiptr=G->lcol;
		*(guiptr+(G->width-1))=G->rcol;
		guiptr+=G->width;
	}
	memset(guiptr,G->rcol,G->width);
}

/*---------------------------------------------------------*/
/* These routines write text on the gui		           */
/*---------------------------------------------------------*/
void gui_text(struct gui *G,byte *text,dword back)
{
	dword ch;
	dword rc=G->rc;
	byte *guiptr=G->tmp;
	byte bit,x;
	while(*text)
	{  
		ch=((*text)-0x21)&0xff;
		if(ch>57) ch=0;
		ch=(ch<<6)+(rc<<3);
		text++;
		for(x=0;x<8;x++)
		{
			bit=thefont[ch++];
			if(bit==4)
			{
				*guiptr++=1;
			}
			else
			{
				if(bit==1)
				{
					*guiptr++=0;
				}
				else
				{
					*guiptr++=back;
				} 
			}
		} 
	}
}

void gui_draw(struct gui *G)
{
	dword x=0;
	dword entry=0;
	dword cy=4;
	dword cx=3;
	byte *text;
	gui_base(G);
	current=G->first;
	G->tmp=(G->gui1)+(cy*G->width)+cx;
	while((current)&&(cy<G->height))
	{
		text=current->item;
		for(x=0;x<8;x++)
		{
			G->rc=x;
			if(G->select==entry)
			{
				gui_text(G,text,G->scol);
			}
			else
			{
				gui_text(G,text,G->bkcol);
			}
			cy++;
			G->tmp=(G->tmp)+(G->width);
		}
		current=current->next;
		entry++;
	}
	gui_render(&gui_bitmap);
}

void gui_to_bitmap(struct gui *G,struct bitmap *B)
{
	B->xcord=G->xcord+20;
	B->ycord=G->ycord+28;
	B->width=G->width;
	B->height=G->height;
	B->bitmap_ptr=G->gui1;
	B->bitmap_bkg=G->gui2;
	if(gui_screen->mode)
	{
		B->xinc=B->width>>2;
	}
	else
	{
		B->xinc=B->width;
	}
}

dword gui_handle(struct gui *mygui)
{
	dword value=0xff;
	rawkey=0;
	gui_to_bitmap(mygui, &gui_bitmap);
	gui_grab_bkg(&gui_bitmap);
	gui_draw(mygui);
	while(KILLGUI!=0xff)
	{
		switch(rawkey)
		{
			case MAKE_ESC:
				KILLGUI=KILLEMU=0xff;
				value=0xdead;
				break;
			case MAKE_F10:
				KILLGUI=0xff;
				value=0xf10;
				break;
			case MAKE_UP:
				if(mygui->select!=0xff)
				{
					if((mygui->entries)>0)
					{
						mygui->select=(mygui->select--)&0x0f;
						if(mygui->select>mygui->entries)
						{
							mygui->select=mygui->entries;
						}
					}
					else
					{
						mygui->select=0x00;
					}
				}
				else
				{
					mygui->select=0x00;
				}
				rawkey=0;
				gui_draw(mygui);
				break;
			case MAKE_DOWN:
				if(mygui->select!=0xff)
				{
					if((mygui->entries)>0)
					{
						mygui->select=(mygui->select++)&0xf;
						if(mygui->select>mygui->entries)
						{
							mygui->select=0;
						}
					}
					else
					{
						mygui->select=0x00;
					}
				}
				else
				{
					mygui->select=0x00;
				}
				rawkey=0;
				gui_draw(mygui);
				break;
			case MAKE_ENTER:
				value=mygui->select;
				KILLGUI=0xff;
				break;
			default:
				break;
		}
	}
	gui_render_bkg(&gui_bitmap);
	rawkey=0;
	KILLGUI=0;
	return(value);
}

void gui_message(struct gui *G,dword mode)
{
	G->select=0xff;
	gui_init(G);
	rawkey=0;
	if((G->gui1)&&(G->gui2))
	{
		gui_to_bitmap(G, &gui_bitmap);
		gui_grab_bkg(&gui_bitmap);
		gui_draw(G);
		gui_render(&gui_bitmap);
		if(mode==0)
		{
			delay(1000);
		}
		else
		{
			while(rawkey!=MAKE_F10);
		}
		gui_render_bkg(&gui_bitmap);
	}
	rawkey=0;
	delay(1);
}
/*---------------------------------------------------------*/
/* Free all reserved memory used by the current gui        */
/*---------------------------------------------------------*/
void gui_cleanup(struct gui *F)
{
	if(F->gui1) free(F->gui1);
	if(F->gui2) free(F->gui2);
}
/*----------------------------------------------------------*/
/* start up gui's								*/
/*----------------------------------------------------------*/
void install_guis(struct gui *G,struct screen *myscreen)
{
	while(G->nxtgui)
	{
		gui_init(G);
		G=G->nxtgui;
	}
	gui_init(G);
	gui_screen=myscreen;
	install_fileselector();
}

void uninstall_guis(struct gui *G)
{
	uninstall_fileselector();
	while(G->nxtgui)
	{
		gui_cleanup(G);
		G=G->nxtgui;
	}
	gui_cleanup(G);
}