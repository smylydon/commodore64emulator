
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

struct fileselector{
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

/* gui library function prototypes */
extern void install_guis(struct gui *the_gui, struct screen *the_screen);
extern void uninstall_guis(struct gui *the_gui);
extern dword gui_handle(struct gui *my_gui);
extern void	gui_message(struct gui *the_gui,dword mode);
