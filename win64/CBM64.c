#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <SDL.h>
#include "gfx.h"
#include "gui.h"
#include "AddressBus.h"
#include "M6510.h"
#include "M6569.h"
#include "M6526.h"
#include "M6581.h"
#include "tape.h"

#define cWhite 0xffffff
#define cBlack 0x000000
#define WIDTH  720
#define HEIGHT  400

void aStringToBuffer(char* aString);
void anIntToBuffer(unsigned int aNumber,unsigned char aMode,unsigned char aBase);
void Render(unsigned char *gfxchar,unsigned int vline,unsigned char bc);
void HardReset(void);
void fileMenu(void);
void optionsMenu(void);

struct Cords {
    unsigned int bufferx,buffery;
    char buffer[500];
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


unsigned char joyport,joyport1,joyport2,joybit;
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

unsigned int cbmcolor[] =
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



unsigned char keyarr[0xff];
unsigned char invkeyarr[0xff];

unsigned char GetKeyArr(unsigned char data) {
    return keyarr[data];
}

unsigned char GetInvKeyArr(unsigned char data) {
    return invkeyarr[data];
}

unsigned char GetJoy1(void) {
    return joyport1;
}

unsigned char GetJoy2(void) {
    return joyport2;
}


/*-------------------------------------------------------*/
/*----------------------- Text  -------------------------*/
/*-------------------------------------------------------*/

void aStringToBuffer(char* aString) {
    //This rountine prints a line of text in the buffer
    // string  - the string of character to place in the buffer

    char *s=monam.buffer;       //text buffer

    while(*s!=0) {
        s++;                   // find the end of the buffer
    }

    while(*aString!=0) {
        *s++=*aString++;        // add a character to the buffer
    }
    *s++=0;
}

void anIntToBuffer(unsigned int aNumber,unsigned char aMode,unsigned char aBase) {
    //Place a number in the buffer;
    //number - number to convert to ascii
    //mode   - byte=1, word=2, long word=0,
    //base   - Dec=0, Hex=1;

    char aString[0x10];
    aString[0]=0;
    switch(aBase) {
        case 0:
        mns_intToDecimal(aString,aNumber,aMode);
        break;
        case 1:
        mns_intToHex(aString,aNumber,aMode);
        break;
        default:
        mns_intToHex(aString,aNumber,aMode);
        break;
    }
    aStringToBuffer(aString);
}

void RenderBuffer(unsigned int x, unsigned int y) {
    // render text buffer to screen

    char line[40*2],c;
    char* s=monam.buffer;
    char* s2=s;
    unsigned int i=0;
    line[0]=0;
    mns_invertColors();
    mns_rectangle(x,y,300,200,1);
    mns_invertColors();
    while(*s!=0) {
        c=*s++;
        if(c=='\n') {
            line[i]=0;
            mns_textOut(x,y,line);
            y+=8;
            i=0;
            line[0]=0;
        }
        else {
            line[i]=c;
            i++;
        }
    }
    line[i]=0;
    mns_textOut(x,y,line);
    *s2=0;
    mns_swapScreens();
}

void Render(unsigned char* gfxchar,unsigned int vline,unsigned char bgncolor) {
    unsigned int x,color;
    unsigned int s;

    if((vline>=51)&&(vline<=251)) {
        color=cbmcolor[bgncolor];
        for(x=0;x<24;x++) {
            mns_plotColor(x,vline,color);
            mns_plotColor(x+344,vline,color);
        }
        for(x=24;x<24+320;x++) {
            s=*(gfxchar++)&0xff;
            mns_plotColor(x,vline,cbmcolor[s]);
            s++;
        }
    } else {
        if(vline<=310) {
            color=cbmcolor[bgncolor];
            for(x=0;x<368;x++) {
                mns_plotColor(x,vline,color);
            }
        }
    }

    if(vline==311) {
        mns_swapScreens();
    }
}

/*-------------------------------------------------------*/
/*----------------------- Menus -------------------------*/
/*-------------------------------------------------------*/

void mainMenu(void) {
    unsigned int done=0xff;
    while(done) {
        switch(mns_handleMenu(0)) {
            case 0xf00f:
                if(mainmenu.select==0) {
                    helper.menu=1;
                    fileMenu();
                }else {
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
}
void handleFiles(int msg){
    switch(msg) {
        case 0xeeee:
            helper.PAUSE=0xff;
            helper.GUIStatus=0;
            break;
        case 0xdead:
            helper.PAUSE=helper.KILLEMU=0x00;
            break;
    }
}

void fileMenu(void) {
    struct fileselector *P;
    P=(struct fileselector*) malloc(sizeof (struct fileselector));
    if(P==0){
        return;
    }
    unsigned int done=0xff;
    while(done) {
        switch(mns_handleMenu(1)) {
            case 0xf00f:
            switch(filemenu.select) {
                case 0:
                strcpy(P->path,"*.t64");
                mns_fileselecter(P);
                handleFiles(P->msg);
                if((P->msg==0xeee)||(P->msg==0xdead)){
                    mns_textOut(400,80,"HMMMM");
                    done=0;
                }else{
                    if(P->msg!=0xffff) {
                        t64load(P);
                    }
                    done=0;
                    helper.PAUSE=0xff;
                    helper.GUIStatus=0;
                }
                break;
                case 1:
                strcpy(P->path,"*.p00");
                mns_fileselecter(P);
                if(P->msg!=0xff) {
                    p00load(P);
                }
                done=0;
                helper.PAUSE=0xff;
                helper.GUIStatus=0;
                break;
                case 2:
                strcpy(P->path,"*.c64");
                mns_fileselecter(P);
                c64load(P);
                done=0;
                helper.PAUSE=0xff;
                helper.GUIStatus=0;
                break;
                case 3:
                strcpy(P->path,"*.D64");
                mns_fileselecter(P);
                break;
                case 4:
                strcpy(P->path,"*.st64");
                mns_fileselecter(P);
                handleFiles(P->msg);
                if((P->msg==0xeee)||(P->msg==0xdead)){
                    mns_textOut(400,80,"HMMMM");
                    done=0;
                }else{
                    if(P->msg!=0xffff) {
                        s64load(P);
                    }
                    done=0;
                    helper.PAUSE=0xff;
                    helper.GUIStatus=0;
                }
                break;
                case 6:
                strcpy(P->path,"*.st");
                mns_fileselecter(P);
                if(P->msg!=0xff) {
                    s64save(P);
                }

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
    free(P);
}

void optionsMenu(void) {
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
}

/*-------------------------------------------------------*/
/*------------------Emulator Mainloop--------------------*/
/*-------------------------------------------------------*/

void HardReset(void) {
    int index;
    BusResetBus();
    for(index=0;index<255;index++) {
        keyarr[index]=0xff;
        invkeyarr[index]=0xff;
    }
}

void rawkeyUp(unsigned char rawkey) {

    column=keycolumn[rawkey];
    row=keyrow[rawkey];
    if((row==0xf)||(column==0xf)) {
        return;
    }
    else {
        keyarr[column]|=(1<<row);
        invkeyarr[row]|=(1<<column);
    }
}

void rawkeyDown(unsigned char rawkey) {
    unsigned char column,row;
    column=keycolumn[rawkey];
    row=keyrow[rawkey];
    if((row==0xf)||(column==0xf)) {
        return;
    }
    else {
        keyarr[column]&=~(1<<row);
        invkeyarr[row]&=~(1<<column);
    }
}

#define JOYUP  0x01
#define JOYDN  0x02
#define JOYLT  0x04
#define JOYRT  0x08
#define JOYFA  0x10
#define JOYFB  0x10

void goEmulate(void) {
    SDL_Event event;
    //    SDL_keysym key;

    if(helper.PAUSE) {
        if((helper.joystickStatus)&&(poll_count==933)) {
            joyport1=0xff;
            joyport2=0xff;
            poll_count=0;
            if(helper.joyport) {
                joyport1=~(helper.joystick);
            }
            else {
                joyport2=~(helper.joystick);
            }
        }

        if (M6510trace()) {
            helper.PAUSE=0;
            helper.trace=M6510Tracer();
            if(helper.trace==0) {
                M6526Execute();
                M6569Execute();
            }
        }
        else {
            poll_count++;
            M6510Execute();
            M6526Execute();
            M6569Execute();
            M6581Execute();
        }
    }

    if(SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
            rawkey=event.key.keysym.scancode&0x7f;

            switch(event.key.keysym.sym) {
                case SDLK_F9:
                helper.GUIStatus=~helper.GUIStatus;
                break;
                case SDLK_F10:
                helper.PAUSE=~helper.PAUSE;
                break;
                case SDLK_F12:
                HardReset();
                break;
                default:
                if(helper.GUIStatus) {
                    helper.GUIKey=0;
                }
                else {
                    rawkeyDown(rawkey);
                    break;
                }
            }
            break;
            case SDL_KEYUP:
            rawkey=event.key.keysym.scancode&0x7f;
            // If escape is pressed, return (and thus, quit)
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                helper.KILLEMU=0x00;
            }else {
                if(helper.GUIStatus) {
                    helper.GUIKey=0;
                }else {
                    rawkeyUp(rawkey);
                }
            }
            break;
            case SDL_JOYAXISMOTION:  /* Handle Joystick Motion */
            helper.joystick=0;
            if ( ( event.jaxis.value < -3200 ) || (event.jaxis.value > 3200 ) ) {
                if( event.jaxis.axis == 0) {
                    /* Left-right movement code goes here */
                    if(event.jaxis.value<-3200) {
                        helper.joystick|=JOYLT;
                    }
                    else {
                        if(event.jaxis.value>3200) {
                            helper.joystick=JOYRT;
                        }
                    }
                }

                if( event.jaxis.axis == 1) {
                    /* Up-Down movement code goes here */
                    if(event.jaxis.value<-3200) {
                        helper.joystick|=JOYUP;
                    }
                    else {
                        if(event.jaxis.value>3200) {
                            helper.joystick=JOYDN;
                        }
                    }
                }
            }
            break;
            case SDL_JOYBUTTONDOWN:  /* Handle Joystick Button Presses */
            helper.joystick=0;
            if ( event.jbutton.button == 0 ) {
                /* code goes here */
                helper.joystick|=JOYFA;
            }
            if ( event.jbutton.button == 1 ) {
                /* code goes here */
                helper.joystick|=JOYFA;
            }
            break;
            case SDL_QUIT:
            helper.KILLEMU=0x00;
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    unsigned int flags1=SDL_INIT_VIDEO|SDL_INIT_JOYSTICK;
    unsigned int flags2=SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_INIT_JOYSTICK;

    mns_openScreen(flags1,flags2,WIDTH,HEIGHT); //initialise system
    mns_inializeMenus(&mainmenu);
    mns_setColors(cWhite,cBlack);

    monam.cursorx=0;
    monam.cursory=0;
    monam.bufferx=0;
    monam.buffery=0;
    monam.buffer[0]=0;
    monam.cursor[0]=0;

    poll_count=0;
    joyport1=joyport2=0xff;
    helper.PAUSE=helper.KILLEMU=0xff;
    helper.GUIStatus=0;
    helper.GUIKey=0;
    rawkey=0;
    helper.trace=0;
    helper.menu=0;
    helper.joyport=0;
    M6569Initialize();
    M6581Initialize();
    HardReset();
    SDL_JoystickEventState(SDL_ENABLE);
    helper.joystickStatus = SDL_JoystickOpen(0);
    // Main loop.
    while(helper.KILLEMU) {
        if(helper.GUIStatus) {
            mainMenu();
        }else {
            goEmulate();
        }
    }
    mns_closeScreen();
    SDL_Quit();
    return (0);
}




