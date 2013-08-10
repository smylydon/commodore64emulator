
#include "AddressBus.h"
#include "CBM64.h"
#include "gfx.h"
#define ECM                  0x40
#define BMM                  0x20
#define DEN                  0x10
#define MCM                  0x10
#define COLORMASK             0x0f
#define DIM                  320*2
#define CSEL                 0x08
#define RSEL                 0x08

#define IRST                 0x01  //raster irq
#define IMDC                 0x02  //sprite-2-grafix collision irq
#define IMMC                 0x04  //sprite-2-sprite collision irq
#define ILP                  0x08  //light pen irq
#define IRQV                 0x80  //VIC irq enable
#define VicIIirq             0xff00  //VIC irq

#define FIRST_RASTER_LINE    0x10
#define LAST_RASTER_LINE     0x11f

#define DISPLAY_ROW25_START  0x33
#define DISPLAY_ROW25_END    0xfa
#define DISPLAY_ROW24_START  0x37
#define DISPLAY_ROW24_END    0xf6

#define DISPLAY_DMA_START    0x30
#define DISPLAY_DMA_END      0xf7
#define SPRITESX8            0x10  //bit 8 of sprites x cords
#define VICCONREG1           0x11  //vic control register 1 write latch
#define RASTER               0x12  //vic raster write latch
#define LITEPENX             0x13
#define LITEPENY             0x14
#define SPENABLE             0x15  //sprite enable
#define VICCONREG2           0x16  //0xc0 vic control register 2
#define SPEXPANY             0x17  //sprite y expansion
#define VICMEMPT             0x18  //0x01 vic memory pointer
#define VICFLAG              0x19  //0x70 vic interupt reg. writelatch
#define VICMASK              0x1a  //0xf0 vic interupt enable
#define SPRITEDP             0x1b  //sprite data priority
#define SPMULTICOLOR         0x1c  //sprite multicolor
#define SPEXPANX             0x1d  //sprite x expantion
#define SPSPCOLLIDE          0x1e  //sprite-sprite collision
#define SPGRFXCOLLIDE        0x1f  //sprite-data collision

#define BORDERCOLOR          0x20  //0x0f0 border color
#define VBC0                 0x21  //0x0f0 background color 0
#define VBC1                 0x22  //0x0f0 mcm1
#define VBC2                 0x23  //0x0f0 mcm2
#define VBC3                 0x24  //0x0f0 pen1
//spmulticolor1 equ 37  ;25 0x0f0 sprite multicolor 1
//spmulticolor2 equ 38  ;26 0x0f0 sprite multicolor 2

unsigned char VicIIChip[0x40];
unsigned int StandardColorTable[0xff];    //colour lookup table for standard sprites
unsigned int MultiColorTable[0xff];       //colour lookup table for muliticolor sprites
unsigned int MC[0x20];

unsigned char GfxLine[DIM];         //one scan line of grfx
unsigned char SpriteLine[DIM];      //one scan line of sprite
unsigned char sp2spcol[DIM];        //sprite to sprite collision
unsigned char sp2gfxcol[DIM];       //sprite to grfx collision
unsigned char VMLI[0x40];
unsigned char VMLC[0x40];
unsigned int raster;
unsigned int rasterlatch;
unsigned int cycles,aux1,aux2,aux3,aux4,image;
unsigned int bankid,matrixid,videomatrix;
unsigned int vicbank,screenbase;
unsigned int screenmode,bitmapbase,charbank,txtmode,scrmode;
unsigned int scrolly,scrollx;
unsigned int spriteptrs;
unsigned int VCCOUNT,VCBASE,RC;
unsigned int DelYstart=0x33;      // Y dma start
unsigned int DelYend=0xfa;        // Y dma end
unsigned int indexx,indexy,gfxchar,tester;

unsigned char col0,col1,col2,col3,bitmap;
unsigned char badline_flag,badline_enable_flag,idlestate_flag;
unsigned char in_screen_window,screen_enable_flag,in_dma_window;
unsigned char in_deltay_window,pixelrom;
unsigned char sp,sp2,display;

unsigned char *mnshline,*spcoll,*gcoll,*sprite;
unsigned int BitMapMatrix,BitMapColor,MCBASE;
unsigned char *vmliptr,*vmlcptr;
unsigned char *sprdum0,*sprdum1,*sprdum2;
unsigned char *sprena=VicIIChip+0x15;
unsigned char *sprexpx=VicIIChip+0x1d;
unsigned char *sprexpy=VicIIChip+0x17;
unsigned char *sprmcm=VicIIChip+0x1c;
unsigned char *sp2dat=VicIIChip+0x1f;
unsigned char *sp2sp=VicIIChip+0x1e;
unsigned char *spdatp=VicIIChip+0x1b;

/*-----------------------------------------------------------------------*/
/*			Load Vic from Savestate position				 */
/*-----------------------------------------------------------------------*/

/*
**
**	LoadVic restores Vic II emulation from 'save state'
**
*/
void M6569LoadVic(struct ioState *z) {
    unsigned int i;
    unsigned int *p;
    unsigned char *x;
    p=z->peekvic;
    x=z->pokevic;
    unsigned int fixcolor1,fixcolor2;
    for(i=0;i<0x40;i++) {
        VicIIChip[i]=*x++;
    }
    for(i=0;i<64;i++) {
        VMLI[i]=(unsigned char)*p++;
    }
    for(i=0;i<64;i++) {
        VMLC[i]=(unsigned char)*p++;
    }
    vicbank=*p++;
    videomatrix=*p++;
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
    //	SpritePointers=*p++;
    bitmapbase=*p++;
    bitmapbase=*p++;
    screenbase=*p++;
    charbank=*p++;
    //	Spriteptrs=*p++;
    display=(unsigned char)*p++;
    display=(unsigned char)*p++;
    badline_flag=(unsigned char)*p++;
    badline_enable_flag=(unsigned char)*p++;
    idlestate_flag=(unsigned char)*p++;
}


/*
**
**	SaveVic saves the state of Vic II emulation to HDD
**
*/
void M6569SaveVic(struct ioState *z) {
    unsigned int i;
    unsigned int *p;
    unsigned char *x;
    p=z->peekvic;
    x=z->pokevic;
    for(i=0;i<0x40;i++) {
        *x++=VicIIChip[i];
    }
    for(i=0;i<64;i++) {
        *p++=(unsigned int)VMLI[i];
    }
    for(i=0;i<64;i++) {
        *p++=(unsigned int)VMLC[i];
    }
    *p++=vicbank;
    *p++=videomatrix;
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
    //	*p++=SpritePointers;
    *p++=0;
    *p++=bitmapbase;
    *p++=screenbase;
    *p++=charbank;
    //	*p++=spriteptrs;
    *p++=0;
    *p++=(unsigned int)display;
    *p++=(unsigned int)badline_flag;
    *p++=(unsigned int)badline_enable_flag;
    *p++=(unsigned int)idlestate_flag;
}

void M6569Dump(void) {
    unsigned int rc,rr;
    aStringToBuffer("\nDisplay Mode   :");
    rr=VicIIChip[0x11]&0xff;
    rc=VicIIChip[0x16]&0xff;
    rc=((rc&0x10)|(rr&0x60))>>4;
    // ECM4 BMM2 MC
    if(rr&0x10) {
        switch(rc) {
            case 0x00:
            aStringToBuffer(" Standard Text \n");
            break;
            case 0x01:
            aStringToBuffer(" MultiColor Text \n");
            break;
            case 0x02:
            aStringToBuffer(" Standard BitMap \n");
            break;
            case 0x03:
            aStringToBuffer(" MultiColor BitMap \n");
            break;
            case 0x04:
            aStringToBuffer(" Extended Text \n");
            break;
            case 0x05:
            aStringToBuffer(" Invalid Text Mode \n");
            break;
            case 0x06:
            aStringToBuffer(" Invalid BitMap Mode One \n");
            break;
            case 0x07:
            aStringToBuffer(" Invalid BitMap Mode Two \n");
            break;
        }
    }
    else {
        aStringToBuffer(" Disabled Off \n");
    }
    aStringToBuffer("Raster Compare : ");
    anIntToBuffer(rasterlatch,2,1);
    aStringToBuffer("\n");
    aStringToBuffer("Raster Count   : ");
    anIntToBuffer(raster,2,1);
    aStringToBuffer("\n");

    rr=VicIIChip[VICMEMPT]&0xff;
    rc=(BusReadCiaB()^0x03)&0x03;
    aStringToBuffer("Video bank     :");
    if(rc==0)
        aStringToBuffer(" 0000-3fff\n");
    if(rc==1)
        aStringToBuffer(" 4000-7fff\n");
    if(rc==2)
        aStringToBuffer(" 8000-bfff\n");
    if(rc==3)
        aStringToBuffer(" c000-ffff\n");
    rc<<=0xe;
    aStringToBuffer("Video Matrix   : ");
    //   anIntToBuffer(((rr&0xf0)<<0x6)+rc,2,1);
    anIntToBuffer(videomatrix,2,1);
    aStringToBuffer("\n");
    aStringToBuffer("Character bank : ");
    anIntToBuffer(((rr&0xe)<<0xa)+rc,2,1);
    anIntToBuffer(charbank,2,1);
    aStringToBuffer("\n");
    aStringToBuffer("BitMap Matrix  : ");
    anIntToBuffer(((rr&0x8)<<10)+rc,2,1);
    aStringToBuffer("\n");
    aStringToBuffer("Bank ID        : ");
    anIntToBuffer(bankid,2,1);
    aStringToBuffer("\n");

    //    aStringToBuffer("SCREENBASE     : ");
    //    anIntToBuffer(screenbase,2,1);
    //    aStringToBuffer("\n");

    rc=(unsigned int) VicIIChip[0x19]&0x8f;
    rr=(unsigned int) VicIIChip[0x1a]&0xf;
    if(rr) {
        aStringToBuffer ("Interupts Enabled :");
        if(rr&1)
            aStringToBuffer(" Raster         ");
        if(rr&2)
            aStringToBuffer(" Sprite2Gfx     ");
        if(rr&4)
            aStringToBuffer(" Sprite2Sprite  ");
        if(rr&8)
            aStringToBuffer(" LitePen        ");
        aStringToBuffer("\n");
    }
    else {
        aStringToBuffer("Interupts Enabled : None    \n");
    }

    if(rc) {
        aStringToBuffer("Interupts Pending :");
        if(rc&1)
            aStringToBuffer(" Raster         ");
        if(rc&2)
            aStringToBuffer(" Sprite2Gfx     ");
        if(rc&4)
            aStringToBuffer(" Sprite2Sprite  ");
        if(rc&8)
            aStringToBuffer(" LitePen        ");
        aStringToBuffer("\n");
    }
    else {
        aStringToBuffer("Interupts Pending : None    \n");
    }
    aStringToBuffer("\n");
}

void M6569Initialize(void) {
    unsigned int i,temp1,temp2;
    VCBASE=VCCOUNT=RC=0;
    vmliptr=VMLI;
    vmlcptr=VMLC;
    in_screen_window=0;
    in_dma_window=0;
    bankid=matrixid=videomatrix=0;
    vicbank=screenbase=0;
    screenmode=bitmapbase=0;
    charbank=txtmode=scrmode=0;

    scrolly=scrollx=0;
    spriteptrs=0;
    for(i=0;i<256;i++) {
        temp1=temp2=0;
        if(i&0x01) {
            temp1|=0x0003;
            temp2|=0x0005;
        }
        if(i&0x02) {
            temp1|=0x000c;
            temp2|=0x000a;
        }
        if(i&0x04) {
            temp1|=0x0030;
            temp2|=0x0050;
        }
        if(i&0x08) {
            temp1|=0x00c0;
            temp2|=0x00a0;
        }
        if(i&0x10) {
            temp1|=0x0300;
            temp2|=0x0500;
        }
        if(i&0x20) {
            temp1|=0x0c00;
            temp2|=0x0a00;
        }
        if(i&0x40) {
            temp1|=0x3000;
            temp2|=0x5000;
        }
        if(i&0x80) {
            temp1|=0xc000;
            temp2|=0xa000;
        }
        StandardColorTable[i]=temp1;
        MultiColorTable[i]=temp2;
    }
}

void M6569HardReset(void) {
    unsigned int i;
    raster=0;
    display=0;
    rasterlatch=0;
    VCBASE=VCCOUNT=RC=0;
    vmliptr=VMLI;
    vmlcptr=VMLC;
    in_screen_window=0;
    in_dma_window=0;
    for(i=0;i<0x40;i++) {
        VicIIChip[i]=0;
    }
}

void M6569SetCycles(unsigned int mycycles) {
    cycles=mycycles;
}

void M6569DoChanges(void) {
    aux1=VicIIChip[VICMEMPT]&0xff;
    matrixid=aux1;
    aux2=(BusReadCiaB()^0x03)&0x03;
    bankid=aux2;                      //which of 4 16k banks
    aux2=(aux2<<0x0e)&0xffff;         //aux2*16384
    vicbank=aux2;                     //which 16k bank
    aux1=(aux1&0xf0)<<0x06;           //aux1*64
    aux1=(aux1+aux2)&0xffff;          //add address of vic
    videomatrix=aux1;                 //screen
    screenbase=aux1+VCBASE;           //which row
    spriteptrs=(aux1+0x3f8)&0xffff;   //base of sprite pointers
    aux3=(matrixid&0x08)<<0x0a;       //which 8k half
    aux3=(aux3+(aux2+(VCBASE<<0x03)));
    bitmapbase=aux3&0xffff;
    matrixid&=0x0e;
    charbank=(matrixid<<0x0a)+aux2;
    aux1=VicIIChip[VICCONREG2]&0xff;
    aux2=VicIIChip[VICCONREG1]&0xff;
    screenmode=((aux1&0x10)|(aux2&0x60))>>4;
    scrollx=aux1&0x7;
    scrolly=aux2&0x7;
}

void M6569UpDateVicII(void) {
    unsigned int indexx;
    aux1=VicIIChip[VICMEMPT]&0xff;
    aux2=(BusReadCiaB()^0x3)&0x03;
    bankid=aux2;                      //which of 4 16k banks
    matrixid=aux1;
    aux2=(aux2<<0x0e)&0xffff;         //aux2*16384
    vicbank=aux2;                     //which 16k bank
    aux1=(aux1&0xf0)<<0x06;           //aux1*64
    aux1=(aux1+aux2)&0xffff;          //add address of vic
    videomatrix=aux1;                 //screen
    screenbase=aux1+VCBASE;           //which row
    spriteptrs=(aux1+0x3f8)&0xffff;   //base of sprite pointers
    aux3=(matrixid&0x08)<<0x0a;       //which 8k half
    aux3=(aux3+(aux2+(VCBASE<<0x03)));
    bitmapbase=aux3&0xffff;
    matrixid&=0x0e;
    charbank=(matrixid<<0x0a)+aux2;
    aux1=VicIIChip[VICCONREG2]&0xff;
    aux2=VicIIChip[VICCONREG1]&0xff;
    screenmode=((aux1&0x10)|(aux2&0x60))>>4;
    scrollx=aux1&0x7;
    scrolly=aux2&0x7;
    for(indexx=0;indexx<40;indexx++) {
        VMLI[indexx]=BusReadByte(screenbase+indexx);
        VMLC[indexx]=BusReadColorRam(VCCOUNT+indexx);
    }
}

unsigned char M6569ReadVIC(unsigned char address) {
    address&=0x3f;
    switch(address) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        return VicIIChip[address];
        case 0x16:
        return VicIIChip[address]|0xc0;
        case 0x17:
        return VicIIChip[address];
        case 0x18:
        return VicIIChip[address]|0x01;
        case 0x19:
        return VicIIChip[address]|0x70;
        case 0x1A:
        return VicIIChip[address]|0xf0;
        case 0x1B:
        case 0x1C:
        case 0x1D:
        return VicIIChip[address];
        break;
        case 0x1E://sprite to sprite collision register
        case 0x1F://sprite to background collision register
        //when read collision registers are cleared
        aux1=VicIIChip[address];
        VicIIChip[address]=0;
        return aux1;
        break;
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        return VicIIChip[address]|0xf0;
        break;
        case 0x2F:

        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        return 0xff;
        break;
    }
    return 0xff;
}

void M6569WriteVIC(unsigned char byte,unsigned char address) {
    address&=0x3f;
    switch(address) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:
        VicIIChip[address]=byte;
        break;
        case 0x11://upper bit of write latch
        aux2=VicIIChip[address];
        aux1=byte&0x80;
        VicIIChip[address]=(byte&0x7f)|(aux2&0x80);
        rasterlatch=(rasterlatch&0xff)|(aux1<<0x01);
        rasterlatch&=0x1ff;
        aux2&=0x78;
        aux1=byte&0x78;
        scrolly=byte&0x7;
        if(aux1!=aux2) {
            M6569DoChanges();
        }
        if(raster==rasterlatch) {
            aux1=VicIIChip[VICFLAG]&0xff;
            aux2=VicIIChip[VICMASK]&0xff;
            aux1|=IRST;          //flag a raster interupt.
            if(aux2&IRST)        //is raster interupt enabled
            {
                aux1|=IRQV;      //cause interupt
                BusSetIRQ(VicIIirq); //tell bus about it
            }
            VicIIChip[VICFLAG]=aux1;
        }
        break;
        case 0x12:// lower byte of write latch
        rasterlatch=(rasterlatch&0x100)|byte;
        if(raster==rasterlatch) {
            aux1=VicIIChip[VICFLAG]&0xff;
            aux2=VicIIChip[VICMASK]&0xff;
            aux1|=IRST;          //flag a raster interupt.
            if(aux2&IRST)        //is raster interupt enabled
            {
                aux1|=IRQV;      //cause interupt
                BusSetIRQ(VicIIirq); //tell bus about it
            }
            VicIIChip[VICFLAG]=aux1;
        }
        break;
        case 0x13:
        case 0x14:
        case 0x15:
        VicIIChip[address]=byte;
        break;
        case 0x16:
        aux1=VicIIChip[address]&0x38;
        VicIIChip[address]=byte;
        aux2=byte&0x38;
        scrollx=byte&0x7;
        if(aux1!=aux2) {
            M6569DoChanges();
        }
        break;
        case 0x17:
        VicIIChip[address]=byte;
        break;
        case 0x18:
        VicIIChip[address]=byte|0x01;
        break;
        case 0x19://vic flag
        BusClearIRQ(VicIIirq);
        byte=(~byte)&0xf;
        aux1=VicIIChip[VICFLAG]&byte;
        byte=aux1;
        aux1&=VicIIChip[VICMASK];
        if(aux1!=0) {
            byte|=0x80;
        }
        VicIIChip[VICFLAG]=byte|0x70;
        break;
        case 0x1A://vic mask
        byte&=0xf;
        VicIIChip[address]=byte|0xf0;
        aux1=VicIIChip[VICFLAG]&0xff;
        byte&=aux1;
        BusClearIRQ(VicIIirq);
        if(byte!=0) {
            aux1|=0x80;
            VicIIChip[VICFLAG]=aux1;
        }
        else {
            aux1&=0x7f;
            VicIIChip[VICFLAG]=aux1;
        }
        break;
        case 0x1B:
        case 0x1C:
        case 0x1D:
        VicIIChip[address]=byte;
        break;
        case 0x1E: //writing to the collision registers
        case 0x1F: // has no effect;
        break;
        case 0x20://colour registers
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        VicIIChip[address]=byte|0xf0;
        break;
        case 0x2F:

        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        break;
    }
}

/*-----------------------------------------------------------------------*/
/*   Draw C64 sprites to screen                      */
/*-----------------------------------------------------------------------*/

/*
**
** Draws a mono coloured sprite with NO horizontal expansion
**
*/
void DrawPlaneSprite(unsigned int address) {
    unsigned char x,width,bitpattern;
    unsigned char check=*spdatp&sp2;
    sprdum0=gcoll;                  /* graphics collision   */
    sprdum1=spcoll;                 /* sprite collision   */
    sprdum2=mnshline;               /* horizontal line   */
    for(width=0;width<3;width++) {
        x=0x80;                     /* test bit x in sprite bitpattern */
        /* first byte of sprite data  */
        bitpattern=BusReadRam(address++);
        while(x) {
            /* is the text bit x set  */
            if(bitpattern&x) {
                /* is there a sprite pixel already painted here line */
                if(*sprdum1==0) {
                    /* has sprite sp got priority or no gfx pixel present */
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col3;
                    }
                }
                else {
                    /* indicate sprite sp collide with another sprite */
                    *sp2sp|=*sprdum1|sp2;
                }


                /* is there gfx present here     */
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                /* mark sprite sp has pixel at this point   */
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
** Draws a mono coloured sprite with horizontal expansion
**
*/
void DrawPlaneSpriteX2(unsigned int address) {
    unsigned int x,width,bitpattern;
    unsigned char check=*spdatp&sp2;
    sprdum0=gcoll;
    sprdum1=spcoll;
    sprdum2=mnshline;
    for(width=0;width<3;width++) {
        x=0x8000;
        /* first byte of sprite data  */
        bitpattern=StandardColorTable[BusReadRam(address++)];
        while(x) {
            if(bitpattern&x) {
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col3;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
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
** Draws a multi colour sprite with NO horizontal expansion
**
*/
void DrawMulticolorSprite(unsigned int address) {
    unsigned char y,bitpattern;
    unsigned int testbitx,x,p;
    unsigned char check=*spdatp&sp2;
    sprdum0=gcoll;
    sprdum1=spcoll;
    sprdum2=mnshline;
    for(y=0;y<3;y++) {
        testbitx=0xc0;
        p=6;
        /* first byte of sprite data  */
        bitpattern=BusReadRam(address++);
        while(testbitx) {
            x=(bitpattern&testbitx)>>p;
            switch(x) {
                case 0x3:
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col2;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                sprdum0++;
                sprdum1++;
                sprdum2++;
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col2;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                break;
                case 0x2:
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col3;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                sprdum0++;
                sprdum1++;
                sprdum2++;
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col3;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                break;
                case 0x1:
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col1;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                sprdum0++;
                sprdum1++;
                sprdum2++;
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col1;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
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
** Draws a multi colour sprite with horizontal expansion
**
*/
void DrawMulticolorSpriteX2(unsigned int address) {
    unsigned char y;
    unsigned int testbitx,x,p,bitpattern;
    unsigned char check=*spdatp&sp2;
    sprdum0=gcoll;
    sprdum1=spcoll;
    sprdum2=mnshline;
    for(y=0;y<3;y++) {
        testbitx=0xc000;
        p=14;
        /* first byte of sprite data  */
        bitpattern=MultiColorTable[BusReadRam(address++)];
        while(testbitx) {
            x=(bitpattern&testbitx)>>p;
            switch(x) {
                case 0x3:
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col2;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                sprdum0++;
                sprdum1++;
                sprdum2++;
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col2;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                break;
                case 0x2:
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col3;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                sprdum0++;
                sprdum1++;
                sprdum2++;
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col3;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                break;
                case 0x1:
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col1;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
                    *sp2dat|=sp2;
                }
                *sprdum1|=sp2;
                sprdum0++;
                sprdum1++;
                sprdum2++;
                if(*sprdum1==0) {
                    if((check==0)||(*sprdum0==0)) {
                        *sprdum2=col1;
                    }
                }
                else {
                    *sp2sp|=*sprdum1|sp2;
                }
                if(*sprdum0==1) {
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
** DrawSprites:
**
*/

void DrawSprites(void) {
    unsigned char t;
    unsigned int py,px,spriteAddress;
    sp2=1;
    sp=0;
    image=0;
    MCBASE=vicbank+spriteptrs;

    for(t=0;t<8;t++) {
        py=VicIIChip[1+(t*2)]&0xff;
        px=VicIIChip[0+(t*2)]&0xff;
        if((*sprena&sp2)&&(MC[sp]<63)) {
            col3=VicIIChip[0x27+sp]&0xf;
            spriteAddress=BusReadRam(MCBASE+MC[sp])<<6;
            mnshline=GfxLine+px;
            spcoll=sp2spcol+px;
            gcoll=sp2gfxcol+px;
            image+=2;

            //Multicolor or monocolor sprites?
            if(*sprmcm&sp2) {
                //double width or single width sprites?
                if(*sprexpx&sp2) {
                    DrawMulticolorSpriteX2(spriteAddress);
                }
                else {
                    DrawMulticolorSprite(spriteAddress);
                }
            }
            else {
                //monocolored sprites...
                //double width monocolored sprites or normal width
                if(*sprexpx&sp2) {
                    DrawPlaneSpriteX2(spriteAddress);
                }
                else {
                    DrawPlaneSprite(spriteAddress);
                }
            }
            //double height sprite?
            if(*sprexpy&sp2) {
                if(MC[sp+8]&0x01) {
                    MC[sp]+=3;
                }
                MC[sp+8]+=1;
            }
            else {
                MC[sp]+=3;
                MC[sp+8]+=1;
            }
        }
        if(py==raster) {
            MC[sp]=MC[sp+8]=0;
        }
        sp2<<=1;
        sp++;
        sp&=0x7;
        MCBASE++;
    }
    sp=VicIIChip[VICMASK];
    if(*sp2sp) {
        VicIIChip[VICFLAG]|=IMMC;       //sprite to sprite collision occurred
        //is this type of interrupt enabled
        if(sp&IMMC) {
            VicIIChip[VICFLAG]|=IRQV;    //signal interrupt
            BusSetIRQ(VicIIirq);        //tell bus about it
        }
    }
    if(*sp2dat) {
        VicIIChip[VICFLAG]|=IMDC;        //sprite to data collision occurred
        if(sp&IMDC) {
            VicIIChip[VICFLAG]|=IRQV;    //signal interrupt
            BusSetIRQ(VicIIirq);        //tell bus about it
        }
    }
    if(raster>=0xff) {
        /* if beem mnsvline is at bottom of paper, so turn off sprites */
        for(t=0;t<8;t++) {
            MC[t]=63;
        }
    }
}



/*-------------------------------------------------------*/
/*---------------------Screen Modes----------------------*/
/*-------------------------------------------------------*/

/*--------------------- Text Modes ----------------------*/

void StandardTxtMode(void) {
    gcoll=sp2gfxcol;
    mnshline=GfxLine+24;
    col0=VicIIChip[VBC0]&0x0f;
    /* paint background paper color */
    for(indexx=0;indexx<320;indexx++) {
        *mnshline++=col0;
    };
    /* paint a line of pixels */
    mnshline=GfxLine+scrollx+24;
    for(indexx=0;indexx<40;indexx++) {
        aux1=(VMLI[indexx]&0xff)<<3; //character
        col1=VMLC[indexx]&0x0f;        //color ram
        if(pixelrom) {
            aux2=BusReadChar(aux1+gfxchar+RC)&0xff;
        }
        else {
            aux2=BusReadByte(aux1+gfxchar+RC)&0xff;
        }
        if(aux2) {
            aux3=0x80;
            for(indexy=0;indexy<8;indexy++) {
                if(aux2&aux3) {
                    *gcoll=1;
                    *mnshline=col1;
                }
                gcoll++;
                mnshline++;
                aux3>>=1;
            }
        }
        else {
            gcoll+=8;
            mnshline+=8;
        }
    }
};

void ExtendedTxtMode(void) {
    gcoll=sp2gfxcol;
    mnshline=GfxLine+scrollx+24;
    for(indexx=0;indexx<40;indexx++) {
        aux1=VMLI[indexx]&0xff; //character
        col0=VMLC[indexx]&0x0f;        //color ram
        aux2=((aux1>>6)&0x3) ;    //color ram
        col1=VicIIChip[VBC0+aux2]&0xf;
        aux1=(aux1&0x3f)<<0x3;    //only first 64 characters
        if(pixelrom) {
            aux2=BusReadChar(aux1+gfxchar+RC)&0xff;
        }
        else {
            aux2=BusReadByte(aux1+gfxchar+RC)&0xff;
        };
        if(aux2) {
            aux3=0x80;
            for(indexy=0;indexy<8;indexy++) {
                if(aux2&aux3) {
                    *gcoll++=1;
                    *mnshline++=col1;
                }
                else {
                    *gcoll++=0;
                    *mnshline++=col0;
                }
                aux3>>=1;
            }
        }
        else {
            gcoll+=8;
            for(aux3=0;aux3<8;aux3++) {
                *mnshline++=col0;
            }
        }
    }
};

void MultiColorTxtMode(void) {
    gcoll=sp2gfxcol;
    mnshline=GfxLine+24;
    col0=VicIIChip[VBC0]&0x0f;
    col1=VicIIChip[VBC1]&0x0f;
    col2=VicIIChip[VBC2]&0x0f;
    mns_textOut(400,50,"MCM Text");
    /* paint background paper color */
    for(indexx=0;indexx<320;indexx++) {
        *mnshline++=col0;
    };

    /* paint a line of pixels */
    mnshline=GfxLine+scrollx+24;
    for(indexx=0;indexx<40;indexx++) {
        aux1=(VMLI[indexx]&0xff)<<3; //character
        col3=VMLC[indexx]&0x0f;        //color ram
        if(pixelrom) {
            aux2=BusReadChar(aux1+gfxchar+RC)&0xff;
        }
        else {
            aux2=BusReadByte(aux1+gfxchar+RC)&0xff;
        }
        if(col3>7) {
            col3>>=4;
            for(aux3=0;aux3<4;aux3++) {
                if(aux2&0x80) {
                    if(aux2&0x40) {
                        // bits are 11xxxxxx
                        *gcoll++=1;
                        *gcoll++=1;
                        *mnshline++=col3;
                        *mnshline++=col3;
                    }
                    else {
                        // bits are 10xxxxxx
                        *gcoll++=1;
                        *gcoll++=1;
                        *mnshline++=col2;
                        *mnshline++=col2;
                    };
                }
                else {
                    if(aux2&0x40) {
                        // bits are 01xxxxxx
                        gcoll+=2;
                        *mnshline++=col3;
                        *mnshline++=col3;
                    }
                    else {
                        // bits are 00xxxxxx
                        gcoll+=2;
                        mnshline+=2;
                    }
                }
                aux2<<=2;
            }//end for

        }
        else {
            gcoll+=8;
            mnshline+=8;
        }
    }
};

/*---------------------Bitmap Modes----------------------*/

void StandardBitMapMode(void) {
    unsigned int bitmapcounter;
    gcoll=sp2gfxcol;
    mnshline=GfxLine+scrollx+24;
    bitmapcounter=bitmapbase+(VCCOUNT<<0x3)+RC;
    /* paint a line of pixels */
    for(indexx=0;indexx<40;indexx++) {
        col0=(VMLI[indexx]&0xff); //used for colors
        col1=(col0>>0x4);
        col0&=0xf;
        aux3=0x80;
        aux2=BusReadRam(bitmapcounter)&0xff;
        for(indexy=0;indexy<8;indexy++) {
            if(aux2&aux3) {
                *gcoll=1;
                *mnshline=col1;
            }
            else {
                *gcoll+=0;
                *mnshline+=col0;
            }
            aux3>>=1;
        }
        bitmapcounter+=8;
    }
}

void MultiColorBitMapMode(void) {
    unsigned int bitmapcounter;
    gcoll=sp2gfxcol;
    col0=VicIIChip[VBC0]&0x0f;
    mnshline=GfxLine+24;
    mns_textOut(400,50,"MCM BMap");
    /* paint background paper color */
    for(indexx=0;indexx<320;indexx++) {
        *mnshline++=col0;
    };
    /* paint a line of pixels */
    mnshline=GfxLine+scrollx+24;
    bitmapcounter=bitmapbase+(VCCOUNT<<0x3)+RC;
    for(indexx=0;indexx<40;indexx++) {
        col1=(VMLI[indexx]&0xff); //used for colors
        col3=VMLC[indexx]&0x0f;        //color ram
        col2=(col0>>0x4);
        col1&=0xf;
        aux3=0x80;
        aux2=BusReadRam(bitmapcounter)&0xff;
        if(aux2) {
            for(aux3=0;aux3<4;aux3++) {
                if(aux2&0x80) {
                    if(aux2&0x40) {
                        // bits are 11xxxxxx
                        *gcoll++=1;
                        *gcoll++=1;
                        *mnshline++=col3;
                        *mnshline++=col3;
                    }
                    else {
                        // bits are 10xxxxxx
                        *gcoll++=1;
                        *gcoll++=1;
                        *mnshline++=col2;
                        *mnshline++=col2;
                    };
                }
                else {
                    if(aux2&0x40) {
                        // bits are 01xxxxxx
                        gcoll+=2;
                        *mnshline++=col1;
                        *mnshline++=col1;
                    }
                    else {
                        // bits are 00xxxxxx
                        gcoll+=2;
                        mnshline+=2;
                    }
                }
                aux2<<=2;
            }
        }
        else {
            gcoll+=8;
            mnshline+=8;
        }
        bitmapcounter+=8;
    }
}

/*---------------------Invalid Modes---------------------*/

void InvalidTxtMode(void) {
    gcoll=sp2gfxcol;
    mnshline=GfxLine+24;

    /* paint background paper color */
    for(indexx=0;indexx<320;indexx++) {
        *mnshline++=0;
    };

    /* paint a line of pixels */
    mnshline=GfxLine+scrollx+24;
    for(indexx=0;indexx<40;indexx++) {
        aux1=(VMLI[indexx]&0xff); //character
        aux1=(aux1&0x3f)<<0x3;    //only first 64 characters
        if(pixelrom) {
            aux2=BusReadChar(aux1+gfxchar+RC)&0xff;
        }
        else {
            aux2=BusReadByte(aux1+gfxchar+RC)&0xff;
        }
        if(aux2) {
            for(aux3=0;aux3<4;aux3++) {
                if(aux2&0x80) {
                    if(aux2&0x40) {
                        // bits are 11xxxxxx
                        *gcoll++=1;
                        *gcoll++=1;
                    }
                    else {
                        // bits are 10xxxxxx
                        *gcoll++=1;
                        *gcoll++=1;
                    };
                }
                else {
                    if(aux2&0x40) {
                        // bits are 01xxxxxx
                        gcoll+=2;
                    }
                    else {
                        // bits are 00xxxxxx
                        gcoll+=2;
                    }
                }
                aux2<<=2;
            }
        }
        else {
            gcoll+=8;
        }
    }
}

/*---------------------- Computation --------------------*/

void DrawC64DisplayMode(void) {
    if((raster>=DelYstart)&&(raster<=DelYend)&&display) {
        in_deltay_window=0xff;
        if(!idlestate_flag) {
            switch(screenmode) {
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
                StandardTxtMode();
                break;
                case 0x06:
                StandardTxtMode();
                break;
                case 0x07:
                StandardTxtMode();
                break;
            }
        }
        else {
            StandardTxtMode();
        }
    }
}

void ComputeBadline(void) {
    badline_flag=0;
    VCCOUNT=VCBASE;
    in_screen_window=0xff;
    scrolly=txtmode&0x7;

    /* is the line within dma start and end? */
    if((raster>=DISPLAY_DMA_START)&&(raster<=DISPLAY_DMA_END)) {
        in_dma_window=0xff;

        /* is this a badline? */
        if((scrolly==(raster&0x07))&&(badline_enable_flag)) {
            RC=0;
            BusCPUyesBadline();
            display=screen_enable_flag;
            idlestate_flag=0x00;
            badline_flag=0xff;
            M6569UpDateVicII();
        }
    }
}

void M6569Execute(void) {
    aux1=VicIIChip[VICCONREG1]&0xff;       //vic control reg 1
    aux2=aux1&0x7f;                   //copy read latch
    aux3=VicIIChip[VICFLAG]&0xff;          //interupt flag
    aux4=VicIIChip[VICMASK]&0xff;          //interupt mask
    raster++;
    if(raster>311) {
        raster=0;
    }
    VicIIChip[RASTER]=(unsigned char)raster&0xff;
    aux1=(aux2|((raster>>1)&0x80));
    VicIIChip[VICCONREG1]=(unsigned char)aux1;       //vic control reg 1
    //check for raster interupt
    if(raster==rasterlatch) {
        aux3|=IRST;          //flag a raster interupt.
        if(aux4&IRST)        //is raster interupt enabled
        {
            aux3|=IRQV;      //cause interupt
            BusSetIRQ(VicIIirq); //tell bus about it
        }
        VicIIChip[VICFLAG]=aux3;
    }
    in_dma_window=0;
    in_deltay_window=0;
    txtmode=aux1;

    aux3=aux1&DEN;
    if(raster==DISPLAY_DMA_START) {
        badline_enable_flag=aux3;
    }
    screen_enable_flag=aux3;
    scrmode=VicIIChip[VICCONREG2]&0xff;       //vic control reg 2
    if((raster>=0x33)&&(raster<=0xfb)) {
        gcoll=sp2gfxcol;
        for(indexx=0;indexx<0x180;indexx++) {
            *gcoll++=0;
        }
    }

    BusCPUnoBadline();
    if((raster>=FIRST_RASTER_LINE)&&(raster<=LAST_RASTER_LINE)) {
        ComputeBadline();
        /* is display turned on? */

        if((raster>=DISPLAY_ROW25_START)&&(raster<=DISPLAY_ROW25_END)) {
            mnshline=GfxLine+scrollx;
            gcoll=sp2gfxcol+scrollx;       // start of gfx collision buffer
            gfxchar=charbank;              // start of character pointer
            pixelrom=0x00;
            if((bankid==0)||(bankid==2)) {
                pixelrom=0xff;
                if(matrixid==6) {
                    gfxchar=0x00;
                }
                if(matrixid==4) {
                    gfxchar=0x800;
                }
            }

            //calculate number of rows
            if(txtmode&RSEL) {
                DelYstart=DISPLAY_ROW25_START;
                DelYend=DISPLAY_ROW25_END;
            }
            else {
                DelYstart=DISPLAY_ROW24_START;
                DelYend=DISPLAY_ROW24_END;
            }

            DrawC64DisplayMode();
            //   DrawSprites();
            mnshline=GfxLine+24;
            if((!in_deltay_window)||(!(txtmode&DEN))) {
                col0=VicIIChip[BORDERCOLOR]&0x0f;
                mnshline=GfxLine+24;
                for(indexx=0;indexx<320;indexx++) {
                    *mnshline++=col0;
                }
            }

            //how many columns
            if(!(scrmode&CSEL)) {
                // left & right by 8 pixels for scrolling
                col0=VicIIChip[BORDERCOLOR]&0x0f;
                for(indexx=0;indexx<4;indexx++) {
                    GfxLine[24+indexx]=col0;
                    GfxLine[(24+313)+indexx]=col0;
                }
            }
        }
        else {
            if(raster<=0xff) {
                mnshline=GfxLine+24;
                DrawSprites();
            }
        }

        if(RC==0x7) {
            idlestate_flag=0xff;
        }
        else {
            RC++;
            RC&=0x7;
        }

        if(badline_flag) {
            idlestate_flag=0x00;
            VCBASE+=40;
        }

        badline_flag=0;

        if(raster==0xfb) {
            VCBASE=VCCOUNT=0;
            RC=0;
            idlestate_flag=0;
            badline_flag=0;
            badline_enable_flag=0;
            display=0;
            gcoll=sp2gfxcol;
            /* clear collision buffer */
            for(indexx=0;indexx<0x180;indexx++) {
                *gcoll++=0;
            }
            //           ScreenDump();
        }
    }
    /* Render C64 raster line to PC raster line */
    Render(GfxLine+24,raster,VicIIChip[BORDERCOLOR]&0xf);
}

