//includes
#include "keys.h"
#include "M6510.h"
#include "modes.h"
#include "gui.h"
#include "tape.h"
#include "joystick.h"

/**************** external functions ****************/	
extern void monam(void);

extern void VicII(void);
extern void DumpSprites(void);
extern void InitVicTables(struct screen *S);

extern struct fileselector* dirprg(byte *path);
extern void LoadVic(dword *p);
extern void SaveVic(dword *p);

extern void install_sid(void);
extern void uninstall_sid(void);
extern void sb_reset(void);

extern dword elapsed_time(void);
extern void pause_time(byte mode);
extern void key_delete(void);
extern void key_init(void);
extern dword uclock_read(void);
extern void io_init(void);
extern void io_close(void);
	
/***************** system functions *****************/
void finish(void);
void HARD_RESET(void);

/*********** Dos and interrupt functions ************/
void setmode(int mode);
void setpalette(void);
void setup(byte x, struct screen *the_screen);
void sortkeys(void);


/* file operations */
void loadformat64x(void);
void saveformat64st(void);


/****************** my gui handlers *****************/
void	maingui(struct screen *the_screen);
void	chipgui(void);
void	colorcorrect(void);
void	colordefault(void);
void	colorgui(struct screen *the_screen);
void	correcttimers(void);
void	gamegui(void);
void	joygui(void);
void	maingui(struct screen *the_screen);
void	monamgui(void);
