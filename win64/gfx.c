#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <SDL.h>
#include "gui.h"
#include "AddressBus.h"
#define PUTPIXEL(x,y,c) \
((unsigned int*)mns_screen.screen->pixels) \
[(x) + (y) * (mns_screen.screen->pitch / 4)] = (c);
#define cWhite 0xffffff
#define cBlack 0x000000
#define cRed   0xff0000
#define cGreen 0x00ff00
#define cBlue  0x0000ff
#define cltGray  0xaaaaaa
#define cdkGray 0x555555

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;

struct Colors {
    UINT pen;
    UINT paper;
};

void mns_setColors(UINT color1,UINT color2);
void mns_getColors(struct Colors *colors);
void mns_invertColors();
void mns_clearScreen();
void mns_fillBuffer(UINT *aBuffer,UINT x,UINT y, UINT w,UINT h);
void mns_emptyBuffer(UINT *aBuffer,UINT x,UINT y, UINT w,UINT h);
void mns_plotPixel(UINT x, UINT y);
void mns_plotColor(UINT x, UINT y, UINT color);
UINT mns_getPixel(UINT x, UINT y);
void mns_horizontalLine(UINT x, UINT y,UINT dx);
void mns_verticalLine(UINT x,UINT y,UINT dy);
void mns_rectangle(UINT x1,UINT y1, UINT dx,UINT dy,UINT filler);
void mns_openScreen(UINT aFlag1, UINT aFlag2, UINT aWidth, UINT aHeight);
void mns_swapScreens();
void mns_closeScreen();
void mns_printLine(char *aString,unsigned int cordx,unsigned int cordy);
void mns_textOut(unsigned int x, unsigned int y,char* aString);
void mns_byteToASCII(char *aString,unsigned int aNumber);
void mns_intToDecimal(char *aString,unsigned int aNumber);
void mns_intToHex(char *aString,unsigned int number, unsigned char mode);
void mns_intToBuffer(unsigned int aNumber,unsigned char aMode
                     ,unsigned char aBase);

struct {
    SDL_Surface *screen;
    UINT screenPen,screenPaper;
    UINT screenWidth,screenHeight;
    UINT screenPage;
}
mns_screen;

SDL_Surface *myscreen;

/*-------------------------------------------------------*/
/*----------------------- colors ------------------------*/
/*-------------------------------------------------------*/

void mns_setColors(UINT color1, UINT color2) {
    mns_screen.screenPen=color1;
    mns_screen.screenPaper=color2;
};

void mns_getColors(struct Colors *colors) {
    colors->pen=mns_screen.screenPen;
    colors->paper=mns_screen.screenPaper;
};

void mns_invertColors() {
    unsigned int temp;
    temp=mns_screen.screenPaper;
    mns_screen.screenPaper=mns_screen.screenPen;
    mns_screen.screenPen=temp;
};

/*-------------------------------------------------------*/
/*--------------------- Primitives ----------------------*/
/*-------------------------------------------------------*/

void mns_fillBuffer(UINT *aBuffer,UINT x,UINT y, UINT w,UINT h) {
    unsigned int i,j;
    for(i=y;i<y+h;i++) {
        for(j=x;j<x+w;j++) {
            *aBuffer++=mns_getPixel(j,i);
        }
    }
}

void mns_emptyBuffer(UINT *aBuffer,UINT x,UINT y, UINT w,UINT h) {
    unsigned int i,j;
    for(i=y;i<y+h;i++) {
        for(j=x;j<x+w;j++) {
            mns_plotColor(j,i,*aBuffer++);
        }
    }
}

void mns_plotPixel(UINT x, UINT y) {
    PUTPIXEL(x,y, mns_screen.screenPen);
}

void mns_plotColor(UINT x, UINT y, UINT color) {
    PUTPIXEL(x,y, color);
}

unsigned int mns_getPixel(UINT x, UINT y) {
    unsigned int c=((unsigned int*)myscreen->pixels)[(x) + (y) * (myscreen->pitch / 4)] ;
    return(c);
}

void mns_horizontalLine(UINT x, UINT y,UINT dx) {
    unsigned int x1;
    for(x1=x;x1<(x+dx);x1++) {
        PUTPIXEL(x1,y, mns_screen.screenPen);
    };
}

void mns_verticalLine(UINT x,UINT y,UINT dy) {
    unsigned int y1;
    for(y1=y;y1<(y+dy);y1++) {
        PUTPIXEL(x,y1,mns_screen.screenPen);
    }
}

void mns_rectangle(UINT x1,UINT y1, UINT dx,UINT dy,UINT filler) {
    UINT x2,y2;
    if(filler) {
        for(y2=0;y2<=dy;y2++) {
            for(x2=0;x2<=dx;x2++) {
                PUTPIXEL(x1+x2,y1+y2,mns_screen.screenPen);
            }
        }
    }
    else {
        mns_verticalLine(x1,y1,dy);
        mns_verticalLine(x1+dx,y1,dy);
        mns_horizontalLine(x1,y1,dx);
        mns_horizontalLine(x1,y1+dy,dx);
    }
}

/*-------------------------------------------------------*/
/*--------------------- Primitives ----------------------*/
/*-------------------------------------------------------*/

void mns_clearScreen() {
    struct Colors color;
    mns_getColors(&color);
    mns_setColors(color.paper,color.paper);
    mns_rectangle(0,0,mns_screen.screenWidth,mns_screen.screenHeight,1);
    mns_setColors(color.pen,color.paper);
};

void mns_closeScreen() {
    mns_uninstallMenuBar();
}

void mns_openScreen(UINT aFlag1, UINT aFlag2, UINT aWidth, UINT aHeight) {
    // Initialize SDL's subsystems - in this case, only video.
    if ( SDL_Init(aFlag1) < 0 ) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    mns_screen.screenWidth=aWidth;
    mns_screen.screenHeight=aHeight;

    // Register SDL_Quit to be called at exit; makes sure things are
    // cleaned up when we quit.
    atexit(SDL_Quit);
    aFlag1|=SDL_INIT_JOYSTICK;
    aFlag2|=SDL_INIT_JOYSTICK;


    // Attempt to create a 640x480 window with 32bit pixels.
    mns_screen.screen = SDL_SetVideoMode(aWidth, aHeight, 32, aFlag2);

    // If we fail, return error.
    if ( mns_screen.screen == NULL ) {
        fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
    }
    myscreen=mns_screen.screen;
    mns_screen.screenPage=0;
    mns_setColors(cWhite,cBlack);
    mns_installMenuBar(aWidth,aHeight);
};

void mns_swapScreens() {
    if (SDL_MUSTLOCK(mns_screen.screen)) {
        if (SDL_LockSurface(mns_screen.screen) < 0)
            return;
    }

    if (SDL_MUSTLOCK(mns_screen.screen))
        SDL_UnlockSurface(mns_screen.screen);
    SDL_Flip(mns_screen.screen);
    mns_screen.screenPage=~mns_screen.screenPage;
}


/*-------------------------------------------------------*/
/*--------------------- Text ----------------------*/
/*-------------------------------------------------------*/

void mns_printLine(char *aString,unsigned int cordx,unsigned int cordy) {
    unsigned char byte,bitwise;
    unsigned int aux1,aux2,x,y,color,color1,color2;
    color1=mns_screen.screenPen;
    color2=mns_screen.screenPaper;
    aux1=0;
    while(*aString) {
        aux2=*aString++;
        //  aux2='4';
        if((aux2>=' ')&&(aux2<='?')) {
            aux2*=8;
        }
        else {
            if((aux2>='A')&&(aux2<='Z')) {
                aux2=(aux2-64)*8;
            }
            else {
                aux2=(aux2-96)*8;
            }
        }
        for(y=0;y<8;y++) {
            byte=BusReadChar(aux2+y);
            bitwise=0x40;
            for(x=0;x<8;x++) {
                color=color2;
                if(byte&bitwise) {
                    color=color1;
                }
                PUTPIXEL(cordx+x,cordy+y,color);
                bitwise>>=1;
            }
        }
        cordx+=8;
        aux1++;
    }
};

void mns_textOut(unsigned int x, unsigned int y,char* aString) {
    char line[40*2],c;
    char* s=aString;
    unsigned int i=0;
    line[0]=0;
    while(*s!=0) {
        c=*s++;
        if(c=='\n') {
            line[i]=0;
            mns_printLine(line,x,y);
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
    mns_printLine(line,x,y);
};

/*-------------------------------------------------------*/
/*-------------------- Conversion -----------------------*/
/*-------------------------------------------------------*/

void mns_byteToASCII(char *aString,unsigned int aNumber) {
    // This subroutine converts a number into a decimal to ASCII

    // table of decimal numberals
    char st[]=
        {'0','1','2','3','4','5','6','7','8','9'
        };
    unsigned int numeral=0;
    unsigned int base10=1000000000; //a billion
    char *s;

    s=aString;
    for(numeral=0;numeral<9;numeral++) {
        *s++=st[0];
    }
    *s=0;

    s=aString;
    if(aNumber==0){
        return;
    }
    while(base10>=10) {
        numeral=aNumber/base10;
        aNumber=aNumber-(numeral*base10);
        base10=base10/10;
        *s++=st[numeral];
    }
    *s++=st[aNumber];     //units
    *s=0;
};

void mns_intToDecimal(char *aString,unsigned int aNumber) {
    //aString- a string to return text in
    //number - number to convert to ascii
    //mode   - byte=1, word=2, long word=0,
    //Base   - number base to convert to


    char tp[0x10];

        mns_byteToASCII(tp,aNumber);
        *aString++=tp[0];
        *aString++=tp[1];
        *aString++=tp[2];
        *aString++=tp[3];

        *aString++=tp[4];
        *aString++=tp[5];
        *aString++=tp[6];
        *aString++=tp[7];
        *aString++=tp[8];
        *aString++=tp[9];
    *aString=0;
}

void mns_intToHex(char *aString,unsigned int number, unsigned char mode) {
    //aString- a string to return text in
    //number - number to convert to ascii
    //mode   - byte=1, word=2, long word=0,


    char st[0x10]={'0','1','2','3','4','5','6','7','8','9',
                   'A','B','C','D','E','F'}; // ascii
    char tp[0x10]; // temporary array
    unsigned int aux1,aux2; // scratch
    aux1=number;
    aux2=0;
    //copy each nybble right to left into temp array.
    for(aux2=0;aux2<8;aux2++) {
        tp[aux2]=st[aux1&0xf];
        aux1>>=4;
    }

    //convert nybble into hex and copy left to right into aString
    for(aux2=0;aux2<8;aux2++) {
        switch(mode) {
            case 1:
            // only copy last byte into string
            if(aux2>5) {
                *aString++=tp[7-aux2];
            }

            break;
            case 2:
            //only copy last last word into string
            if(aux2>3) {
                *aString++=tp[7-aux2];
            }
            break;
            default:
            // copy both words into string
            *aString++=tp[7-aux2];
            break;
        }
    }
    *aString=0;
}


void mns_numberOut(unsigned int x, unsigned int y,unsigned int aNumber) {
    //Place a number in the buffer;
    //number - number to convert to ascii
    //mode   - byte=1, word=2, long word=0,
    //base   - Dec=0, Hex=1;
    unsigned int aBase=0;
    unsigned int aMode=1;
    char aString[0x10];
    aString[0]=0;
    switch(aBase) {
        case 0:
        mns_intToDecimal(aString,aNumber);
        break;
        case 1:
        mns_intToHex(aString,aNumber,aMode);
        break;
        default:
        mns_intToHex(aString,aNumber,aMode);
        break;
    }
    mns_textOut(x,y,aString);
};
