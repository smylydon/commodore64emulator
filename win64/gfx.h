#define cWhite 0xffffff
#define cBlack 0x000000
#define cRed   0xff0000
#define cGreen 0x00ff00
#define cBlue  0x0000ff
#define cltGray  0xaaaaaa
#define cdkGray 0x555555

struct mns_colors {
    unsigned int pen;
    unsigned int paper;
};

extern void mns_setColors(unsigned int color1,unsigned int color2);
extern void mns_getColors(struct mns_colors *colors);
extern void mns_invertColors(void);
extern void mns_clearScreen(void);
extern void mns_fillBuffer(unsigned int *aBuffer,unsigned int x,unsigned int y, unsigned int w,unsigned int h);
extern void mns_emptyBuffer(unsigned int *aBuffer,unsigned int x,unsigned int y, unsigned int w,unsigned int h);
extern void mns_plotPixel(unsigned int x, unsigned int y);
extern void mns_plotColor(unsigned int x, unsigned int y, unsigned int color);
extern unsigned int mns_getPixel(unsigned int x, unsigned int y);
extern void mns_horizontalLine(unsigned int x, unsigned int y,unsigned int dx);
extern void mns_verticalLine(unsigned int x,unsigned int y,unsigned int dy);
extern void mns_rectangle(unsigned int x1,unsigned int y1, unsigned int dx,unsigned int dy,unsigned int filler);
extern void mns_openScreen(unsigned int aFlag1, unsigned int aFlag2, unsigned int aWidth, unsigned int aHeight);
extern void mns_swapScreens(void);
extern void mns_closeScreen(void);
extern void mns_printLine(char *aString,unsigned int cordx,unsigned int cordy);
extern void mns_textOut(unsigned int x, unsigned int y,char* aString);
extern void mns_byteToASCII(char *aString,unsigned int aNumber);
extern void mns_intToDecimal(char *aString,unsigned int aNumber,unsigned char aMode);
extern void mns_intToHex(char *aString,unsigned int number, unsigned char mode);
extern void mns_intToBuffer(unsigned int aNumber,unsigned char aMode
                   ,unsigned char aBase);
extern void mns_numberOut(unsigned int x, unsigned int y,unsigned int aNumber);
