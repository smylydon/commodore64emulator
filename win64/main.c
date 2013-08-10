#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "tape.h"
#include "gfx.h"
#include "gui.h"
#define WIDTH  720
#define HEIGHT  400



void HardReset();
void fileMenu();
void optionsMenu();

struct Cords {
    unsigned int bufferx,buffery;
    unsigned char buffer[500];
    unsigned int cursorx,cursory;
    unsigned char cursor[15];
};

struct tasks {
    unsigned int KILLEMU;
    unsigned int GUIStatus;
    unsigned int PAUSE;
    unsigned int GUIKey;
    unsigned int trace;
    unsigned int menu;
    SDL_Joystick *joystickStatus;
    unsigned char joystick;
    unsigned char joyport;
};

unsigned int joyport,joyport1,joyport2,joybit;
unsigned int cpu_base_cycles,cia_base_cycles,vic_base_lines;
unsigned char PAUSE;

struct Cords monam;
struct tasks helper;
unsigned int poll_count;
unsigned char column,row,rawkey;
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

unsigned char keycolumn[0x80]={
                                  0x0f,   // 0x00 NULL
                                  0x0f,   // 0x01 ESC
                                  0x07,   // 0x02 1
                                  0x07,   // 0x03 2
                                  0x01,   // 0x04 3
                                  0x01,   // 0x05 4
                                  0x02,   // 0x06 5
                                  0x02,   // 0x07 6
                                  0x03,   // 0x08 7
                                  0x03,   // 0x09 8
                                  0x04,   // 0x0A 9
                                  0x04,   // 0x0B 0
                                  0x05,   // 0x0C MINUS
                                  0x05,   // 0x0D EQUALS -> PLUS
                                  0x00,   // 0x0E BKSP   -> DELETE
                                  0x07,   // 0x0F TAB    -> STOP
                                  
                                  0x07,   // 0x10 Q
                                  0x01,   // 0x11 W
                                  0x01,   // 0x12 E
                                  0x02,   // 0x13 R
                                  0x02,   // 0x14 T
                                  0x03,   // 0x15 Y
                                  0x03,   // 0x16 U
                                  0x04,   // 0x17 I
                                  0x04,   // 0x18 O
                                  0x05,   // 0x19 P
                                  0x05,   // 0x1A [ -> @
                                  0x06,   // 0x1B ] -> *
                                  0x00,   // 0x1C ENTER
                                  0x07,   // 0x1D CRTL
                                  0x01,   // 0x1E A
                                  0x01,   // 0x1F S
                                  
                                  0x02,   // 0x20 D
                                  0x02,   // 0x21 F
                                  0x03,   // 0x22 G
                                  0x03,   // 0x23 H
                                  0x04,   // 0x24 J
                                  0x04,   // 0x25 K
                                  0x05,   // 0x26 L
                                  0x05,   // 0x27 COLON->COLON
                                  0x06,   // 0x28 APOS -> SEMI
                                  0x07,   // 0x29 TILD -> <-
                                  0x01,   // 0x2A LSHIFT
                                  0x06,   // 0x2B EQUALS
                                  0x01,   // 0x2C Z
                                  0x02,   // 0x2D X
                                  0x02,   // 0x2E C
                                  0x03,   // 0x2F V
                                  
                                  0x03,   // 0x30 B
                                  0x04,   // 0x31 N
                                  0x04,   // 0x32 M
                                  0x05,   // 0x33 COMA
                                  0x05,   // 0x34 PERIOD
                                  0x06,   // 0x35 SLASH
                                  0x06,   // 0x36 RSHIFT
                                  0x0f,   // 0x37 PRTSCR
                                  0x07,   // 0x38 ALT -> C=
                                  0x07,   // 0x39 SPACE
                                  0x07,   // 0x3A CAPS
                                  0x00,   // 0x3B F1 -> F1
                                  0x0f,   // 0x3C F2
                                  0x00,   // 0x3D F3 -> F3
                                  0x0f,   // 0x3E F4
                                  0x00,   // 0x3F F5 -> F5
                                  
                                  0x0f,   // 0x40 F6
                                  0x00,   // 0x41 F7 -> F7
                                  0x0f,   // 0x42 F8
                                  0x0f,   // 0x43 F9
                                  0x0f,   // 0x44 F10
                                  0x0f,   // 0x45 NUMLCK
                                  0x0f,   // 0x46 SCRLLCK
                                  0x0f,   // 0x47 HOME -> HOME
                                  0x0f,   // 0x48 UP
                                  0x0f,   // 0x49 PGEUP
                                  0x0f,   // 0x4A GREY- -> MINUS
                                  0x0f,   // 0x4B LEFT
                                  0x0f,   // 0x4C CENTER
                                  0x00,   // 0x4D RIGHT ->CRSERGT
                                  0x0f,   // 0x4E GREY+ -> PLUS
                                  0x0f,   // 0x4F END
                                  
                                  0x00,   // 0x50 DOWN ->CRSEDN
                                  0x0f,   // 0x51 PGEDN
                                  0x0f,   // 0x52 INS
                                  0x0f,   // 0x53 DEL -> EXP
                                  0x0f,   // 0x54 NOTHING
                                  0x0f,   // 0x55 NOTHING
                                  0x0f,   // 0x56 NOTHING
                                  0x0f,   // 0x57 F11
                                  0x0f,   // 0x58 F12
                                  0x06,   // 0x59 NOTHING->home
                                  0x0f,   // 0x5A NOTHING
                                  0x0f,   // 0x5B NOTHING
                                  0x0f,   // 0x5C
                                  0x0f,   // 0x5D
                                  0x0f,   // 0x5E NOTHING
                                  0x0f,   // 0x5F NOTHING
                                  
                                  0x0f,   // 0x60 NOTHING
                                  0x0f,   // 0x61
                                  0x0f,   // 0x62
                                  0x0f,   // 0x63 NOTHING
                                  0x0f,   // 0x64 NOTHING
                                  0x0f,   // 0x65 NOTHING
                                  0x0f,   // 0x66
                                  0x0f,   // 0x67
                                  0x0f,   // 0x68 NOTHING
                                  0x0f,   // 0x69
                                  0x0f,   // 0x6A
                                  0x0f,   // 0x6B NOTHING
                                  0x0f,   // 0x6C NOTHING
                                  0x0f,   // 0x6D NOTHING
                                  0x0f,   // 0x6E
                                  0x0f,   // 0x6F
                                  
                                  0x0f,   // 0x70 NOTHING
                                  0x0f,   // 0x71
                                  0x0f,   // 0x72
                                  0x0f,   // 0x73 NOTHING
                                  0x0f,   // 0x74 NOTHING
                                  0x0f,   // 0x75 NOTHING
                                  0x0f,   // 0x76
                                  0x0f,   // 0x77
                                  0x0f,   // 0x78 NOTHING
                                  0x0f,   // 0x79
                                  0x0f,   // 0x7A
                                  0x0f,   // 0x7B NOTHING
                                  0x0f,   // 0x7C NOTHING
                                  0x0f,   // 0x7D NOTHING
                                  0x0f,   // 0x7E
                                  0x0f   // 0x7F
                              };
                              
unsigned char keyrow[0x80]={
                               0x0f,   // 0x00 NULL
                               0x0f,   // 0x01 ESC
                               0x00,   // 0x02 1
                               0x03,   // 0x03 2
                               0x00,   // 0x04 3
                               0x03,   // 0x05 4
                               0x00,   // 0x06 5
                               0x03,   // 0x07 6
                               0x00,   // 0x08 7
                               0x03,   // 0x09 8
                               0x00,   // 0x0A 9
                               0x03,   // 0x0B 0
                               0x00,   // 0x0C MINUS
                               0x03,   // 0x0D EQUALS -> PLUS
                               0x00,   // 0x0E BKSP   -> DELETE
                               0x02,   // 0x0F TAB    -> STOP
                               
                               0x06,   // 0x10 Q
                               0x01,   // 0x11 W
                               0x06,   // 0x12 E
                               0x01,   // 0x13 R
                               0x06,   // 0x14 T
                               0x01,   // 0x15 Y
                               0x06,   // 0x16 U
                               0x01,   // 0x17 I
                               0x06,   // 0x18 O
                               0x01,   // 0x19 P
                               0x06,   // 0x1A [ -> @
                               0x01,   // 0x1B ] -> *
                               0x01,   // 0x1C ENTER
                               0x02,   // 0x1D CRTL
                               0x02,   // 0x1E A
                               0x05,   // 0x1F S
                               
                               0x02,   // 0x20 D
                               0x05,   // 0x21 F
                               0x02,   // 0x22 G
                               0x05,   // 0x23 H
                               0x02,   // 0x24 J
                               0x05,   // 0x25 K
                               0x02,   // 0x26 L
                               0x05,   // 0x27 COLON->COLON
                               0x02,   // 0x28 APOS -> SEMI
                               0x01,   // 0x29 TILD -> <-
                               0x07,   // 0x2A LSHIFT
                               0x05,   // 0x2B EQUALS
                               0x04,   // 0x2C Z
                               0x07,   // 0x2D X
                               0x04,   // 0x2E C
                               0x07,   // 0x2F V
                               
                               0x04,   // 0x30 B
                               0x07,   // 0x31 N
                               0x04,   // 0x32 M
                               0x07,   // 0x33 COMA
                               0x04,   // 0x34 PERIOD
                               0x07,   // 0x35 /
                               0x04,   // 0x36 RSHIFT
                               0x01,   // 0x37 PRTSCR
                               0x05,   // 0x38 ALT -> C=
                               0x04,   // 0x39 SPACE
                               0x07,   // 0x3A CAPS
                               0x04,   // 0x3B F1 -> F1
                               0x0f,   // 0x3C F2
                               0x05,   // 0x3D F3 -> F3
                               0x0f,   // 0x3E F4
                               0x06,   // 0x3F F5 -> F5
                               
                               0x0f,   // 0x40 F6
                               0x03,   // 0x41 F7 -> F7
                               0x0f,   // 0x42 F8
                               0x0f,   // 0x43 F9
                               0x0f,   // 0x44 F10
                               0x0f,   // 0x45 NUMLCK
                               0x0f,   // 0x46 SCRLLCK
                               0x03,   // 0x47 HOME -> HOME
                               0x0f,   // 0x48 UP
                               0x0f,   // 0x49 PGEUP
                               0x0f,   // 0x4A GREY- -> MINUS
                               0x05,   // 0x4B LEFT
                               0x0f,   // 0x4C CENTER
                               0x02,   // 0x4D RIGHT ->CRSERGT
                               0x0f,   // 0x4E GREY+ -> PLUS
                               0x0f,   // 0x4F END
                               
                               0x07,   // 0x50 DOWN ->CRSEDN
                               0x0f,   // 0x51 PGEDN
                               0x0f,   // 0x52 INS
                               0x0f,   // 0x53 DEL -> EXP
                               0x0f,   // 0x54 NOTHING
                               0x0f,   // 0x55 NOTHING
                               0x0f,   // 0x56 NOTHING
                               0x0f,   // 0x57 F11
                               0x0f,   // 0x58 F12
                               0x03,   // 0x59 NOTHING ->home
                               0x0f,   // 0x5A NOTHING
                               0x0f,   // 0x5B NOTHING
                               0x0f,   // 0x5C
                               0x0f,   // 0x5D
                               0x0f,   // 0x5E NOTHING
                               0x0f,   // 0x5F NOTHING
                               
                               0x0f,   // 0x60 NOTHING
                               0x0f,   // 0x61
                               0x0f,   // 0x62
                               0x0f,   // 0x63 NOTHING
                               0x0f,   // 0x64 NOTHING
                               0x0f,   // 0x65 NOTHING
                               0x0f,   // 0x66
                               0x0f,   // 0x67
                               0x0f,   // 0x68 NOTHING
                               0x0f,   // 0x69
                               0x0f,   // 0x6A
                               0x0f,   // 0x6B NOTHING
                               0x0f,   // 0x6C NOTHING
                               0x0f,   // 0x6D NOTHING
                               0x0f,   // 0x6E
                               0x0f,   // 0x6F
                               
                               0x0f,   // 0x70 NOTHING
                               0x0f,   // 0x71 HOME KEYPAD ->HOME
                               0x0f,   // 0x72
                               0x0f,   // 0x73 NOTHING
                               0x0f,   // 0x74 NOTHING
                               0x0f,   // 0x75 NOTHING
                               0x0f,   // 0x76
                               0x0f,   // 0x77
                               0x0f,   // 0x78 NOTHING
                               0x0f,   // 0x79
                               0x0f,   // 0x7A
                               0x0f,   // 0x7B NOTHING
                               0x0f,   // 0x7C NOTHING
                               0x0f,   // 0x7D NOTHING
                               0x0f,   // 0x7E
                               0x0f   // 0x7F
                           };
                           
unsigned int cbmcolor[0x10] =
    {
        0x00101010,    // Black
        0x00ffffff,    // White
        0x00e04040,    // Red
        0x0060ffff,    // Cyan
        0x00e060e0,    // Purple
        0x0040e040,    // Green
        0x004040e0,    // Blue
        0x00ffff40,    // Yellow
        0x00e0a040,    // Orange
        0x009c7448,    // Brown
        0x00ffa0a0,    // Lt.Red
        0x00545454,    // Dk.Gray
        0x00888888,    // Gray
        0x00a0ffa0,    // Lt.Green
        0x00a0a0ff,    // Lt.Blue
        0x00c0c0c0     // Lt.Gray
    };
    
struct mns_Items mainmenuitems[]= {
                                     {" File menu         ",0,0
                                     }
                                     , {" Emulator Options  ",0,0
                                       }, {" Monitor           ",0,0
                                          }, {" About             ",0,0
                                             }, {" Help Screen       ",0,0
                                                }, {"                   ",0,0
                                                   }, {" QUIT Emulation    ",0,0
                                                      }, {
                                         0,0,0
                                     }
                                 };
                                 
struct mns_Items filemenuitems[]= {
                                     {" Load T64      ",0,0
                                     }
                                     , {" Load P00      ",0,0
                                       }, {" Load C64      ",0,0
                                          }, {" Load D64      ",0,0
                                             }, {" Load STATE    ",0,0
                                                }, {" ------------- ",0,0
                                                   }, {" Save STATE    ",0,0
                                                      }, {
                                         0,0,0
                                     }
                                 };
                                 
struct mns_Items optionsmenuitems[]= {
                                        {" CPU Cycles     ",0,0
                                        }
                                        , {" Cia Cycles     ",0,0
                                          }, {" Joystick Port  ",0,0
                                             }, {" Emu Speed      ",0,0
                                                }, {
                                            0,0,0
                                        }
                                    };
                                 
struct mns_Menu optionsmenu= {
                            " OPTIONS ",            //title
                            0,                      //next menu
                            optionsmenuitems,0,     //first & last item
                            0,4,0,                   //select entries id
                            0,0,                    //xcoor, ycoord
                            0,0,                   //menuwith,height
                            0                       //titleheight
                        };
                        
struct mns_Menu filemenu= {
                         " FILE ",
                         &optionsmenu,               // next menu
                         filemenuitems,0,            //first item
                            0,4,0,                   //select entries id
                            0,0,                    //xcoor, ycoord
                            0,0,                   //menuwith,height
                            0                       //titleheight
                     };
                     
struct mns_Menu mainmenu= {
                         " Main ",
                         &filemenu,                 // next menu
                         mainmenuitems,0,            //first item
                            0,4,0,                   //select entries id
                            0,0,                    //xcoor, ycoord
                            0,0,                   //menuwith,height
                            0                       //titleheight
                     };
                     



/*-------------------------------------------------------*/
/*----------------------- Menus -------------------------*/
/*-------------------------------------------------------*/

void mainMenu() {
    unsigned int done=0xff;
    while(done) {
        switch(mns_handleMenu(0)) {
            case 0xf00f:
            if(mainmenu.select==0) {
                helper.menu=1;
                fileMenu();
                
            }
            else {
                if(mainmenu.select==1) {
                    helper.menu=2;
                    optionsMenu();
                }
            }
            if(helper.KILLEMU==0) {
                done=0;
            }
            break;
            case 0xeeee:
            done=0;
            helper.PAUSE=0xff;
            helper.GUIStatus=0;
            break;
            case 0xdead:
            done=0;
            helper.PAUSE=helper.KILLEMU=0x00;
            break;
        }
    }
};

void fileMenu() {
    struct fileselector P;
    unsigned int done=0xff;
    while(done) {
        switch(mns_handleMenu(1)) {
            case 0xf00f:
            switch(filemenu.select) {
                case 0:
                strcpy(P.path,"*.t64");
                mns_fileselecter(&P);
                if(P.msg!=0xff) {
                    t64load(P.filename);
                }
                mns_textOut(400,50,P.filename);
                done=0;
                helper.PAUSE=0xff;
                helper.GUIStatus=0;
                break;
                case 1:
                strcpy(P.path,"*.p00");
                mns_fileselecter(&P);
                if(P.msg!=0xff) {
                    p00load(P.filename);
                }
                done=0;
                helper.PAUSE=0xff;
                helper.GUIStatus=0;
                break;
                case 2:
                strcpy(P.path,"*.c64");
                mns_fileselecter(&P);
                c64load(P.filename);
                done=0;
                helper.PAUSE=0xff;
                helper.GUIStatus=0;
                break;
                case 3:
                strcpy(P.path,"*.D64");
                mns_fileselecter(&P);
                break;
                case 4:
                strcpy(P.path,"*.st");
                mns_fileselecter(&P);
                done=0;
                helper.PAUSE=0xff;
                helper.GUIStatus=0;
                break;
            }
            break;
            case 0xeeee:
            done=0;
            helper.PAUSE=0xff;
            helper.GUIStatus=0;
            break;
            case 0xdead:
            done=0;
            helper.PAUSE=helper.KILLEMU=0x00;
            break;
        }
    }
    
};

void optionsMenu() {
    unsigned int done=0xff;
    while(done) {
        switch(mns_handleMenu(2)) {
            case 0xf00f:
            if(mainmenu.select==0) {
                helper.menu=1;
            }
            break;
            case 0xeeee:
            done=0;
            helper.PAUSE=0xff;
            helper.GUIStatus=0;
            break;
            case 0xdead:
            done=0;
            helper.PAUSE=helper.KILLEMU=0x00;
            break;
        }
    }
};

/*-------------------------------------------------------*/
/*------------------Emulator Mainloop--------------------*/
/*-------------------------------------------------------*/


int main(int argc, char *argv[]) {
	unsigned int flags1=SDL_INIT_VIDEO|SDL_INIT_JOYSTICK;	
	unsigned int flags2=SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_INIT_JOYSTICK;

    mns_openScreen(flags1,flags2,WIDTH,HEIGHT); //initialise system
    mns_inializeMenus(&mainmenu);    
    mns_setColors(cWhite,cBlack);
    
    poll_count=0;
    joyport1=joyport2=0xff;
    helper.PAUSE=helper.KILLEMU=0xff;
    helper.GUIStatus=0;
    helper.GUIKey=0;
    rawkey=0;
    helper.trace=0;
    helper.menu=0;
    helper.joyport=0;
    // Main loop.
    while(helper.KILLEMU) {
        mainMenu();
    }
    mns_closeScreen();
    exit(0);
};
