/*
 * VicII.java
 *
 * Created on February 2, 2006, 8:53 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

/**
 *
 * @author Owner
 */
import java.awt.*;

public class VicII {
private static final int ECM=                  0x40;
private static final int BMM=                  0x20;
private static final int DEN=                  0x10;
private static final int MCM=                  0x10;
private static final int COLORMASK=            0x0f;
private static final int DIM=                  320*2;
private static final int CSEL=                 0x08;
private static final int RSEL=                 0x08;

private static final int IRST=                 0x01;  //raster irq
private static final int IMDC=                 0x02;  //sprite-2-grafix collision irq
private static final int IMMC=                 0x04;  //sprite-2-sprite collision irq
private static final int ILP=                  0x08;  //light pen irq
private static final int IRQV=                 0x80;  //VIC irq enable
private static final int VicIIirq=             0xff00;  //VIC irq

private static final int FIRST_RASTER_LINE=    0x10;
private static final int LAST_RASTER_LINE=     0x11f;

private static final int DISPLAY_ROW25_START=  0x33;
private static final int DISPLAY_ROW25_END=    0xfa;
private static final int DISPLAY_ROW24_START=  0x37;
private static final int DISPLAY_ROW24_END=    0xf6;

private static final int DISPLAY_DMA_START=    0x30;
private static final int DISPLAY_DMA_END=      0xf7;
private static final int SPRITESX8=            0x10;  //bit 8 of sprites x cords
private static final int VICCONREG1=           0x11;  //vic control register 1 write latch
private static final int RASTER=               0x12;  //vic raster write latch
private static final int LITEPENX=             0x13;
private static final int LITEPENY=             0x14;
private static final int SPENABLE=             0x15; //sprite enable
private static final int VICCONREG2=           0x16;  //0xc0 vic control register 2
private static final int SPEXPANY=             0x17;  //sprite y expansion
private static final int VICMEMPT=             0x18;  //0x01 vic memory pointer
private static final int VICFLAG=              0x19;  //0x70 vic interupt reg. writelatch
private static final int VICMASK=              0x1a;  //0xf0 vic interupt enable
private static final int SPRITEDP=             0x1b;  //sprite data priority
private static final int SPMULTICOLOR=         0x1c;  //sprite multicolor
private static final int SPEXPANX=             0x1d;  //sprite x expantion
private static final int SPSPCOLLIDE=          0x1e;  //sprite-sprite collision
private static final int SPGRFXCOLLIDE=        0x1f;  //sprite-data collision

private static final int BORDERCOLOR=          0x20;  //0x0f0 border color
private static final int VBC0=                 0x21;  //0x0f0 background color 0
private static final int VBC1=                 0x22;  //0x0f0 mcm1
private static final int VBC2=                 0x23;  //0x0f0 mcm2
private static final int VBC3=                 0x24;  //0x0f0 pen1
//spmulticolor1 equ 37  ;25 0x0f0 sprite multicolor 1
//spmulticolor2 equ 38  ;26 0x0f0 sprite multicolor 2    
    
    private AddressBus Bus;
    private int[] VicIIChip;
    private int i,temp1,temp2;
    
    private int[] StandardColorTable;    //colour lookup table for standard sprites
    private int[] MultiColorTable;       //colour lookup table for muliticolor sprites
    private int[] MC;

    private int[] GfxLine;         //one scan line of grfx
    private int[] SpriteLine;      //one scan line of sprite
    private int[] sp2spcol;        //sprite to sprite collision
    private int[] sp2gfxcol;       //sprite to grfx collision
    private int[] VMLI;
    private int[] VMLC;
    private Color[] coltable;
    private int raster;
    private int rasterlatch;
    private int cycles,aux1,aux2,aux3,aux4;
    private int bankid,matrixid,videomatrix;
    private int vicbank,screenbase;
    private int screenmode,bitmapbase,charbank,txtmode,scrmode;
    private int scrolly,scrollx;
    private int spriteptrs;
    private int VCCOUNT,VCBASE,RC;
    private int DelYstart;      // Y dma start
    private int DelYend;        // Y dma end
    private int indexx,indexy,gfxchar,tester;

    private int col0,col1,col2,col3;
    private boolean display;
    private boolean badline_flag,badline_enable_flag;
    private boolean idlestate_flag,in_screen_window;
    private boolean screen_enable_flag,in_dma_window;
    private boolean in_deltay_window,pixelrom; 
    private Graphics g;
    public boolean isdone;
    private int xcoord,ycoord;
    
    /** Creates a new instance of VicII */
    public VicII(AddressBus b,Graphics p) {
        Bus=b;
        g=p;
        isdone=false;
        VicIIChip=new int[0x40];
        GfxLine=new int[320*2];         //one scan line of grfx
        SpriteLine=new int[320*2];
        sp2spcol=new int[320*2];        //sprite to sprite collision
        sp2gfxcol=new int[320*2]; 
        coltable=new Color[0x10];
        StandardColorTable=new int[0x100];
        MultiColorTable=new int[0x100];
        MC=new int[0x20];
        VMLI=new int[0x40];
        VMLC=new int[0x40];
        initialize();
    };
    
     public void initialize(){
        VCBASE=VCCOUNT=RC=0;
        in_screen_window=false;
        in_dma_window=false;
        bankid=matrixid=videomatrix=0;
        vicbank=screenbase=0;
        screenmode=bitmapbase=0;
        charbank=txtmode=scrmode=0;;
        scrolly=scrollx=0;
        spriteptrs=0; 
        coltable[0]=Color.black;
        coltable[1]=Color.white;
        coltable[2]=Color.red;
        coltable[3]=Color.cyan;
        
        coltable[4]=Color.magenta;
        coltable[5]=Color.green;
        coltable[6]=Color.blue;
        coltable[7]=Color.yellow;        
        
        
        for(i=0;i<256;i++){
            temp1=temp2=0;
            if((i&0x01)!=0){
                temp1|=0x0003;
                temp2|=0x0005;
            }
            
            if((i&0x02)!=0){
                temp1|=0x000c;
                temp2|=0x000a;
            }
            
            if((i&0x04)!=0){
                temp1|=0x0030;
                temp2|=0x0050;
            }
            
            if((i&0x08)!=0){
                temp1|=0x00c0;
                temp2|=0x00a0;
            }
            
            if((i&0x10)!=0){
                temp1|=0x0300;
                temp2|=0x0500;
            }
            
            if((i&0x20)!=0)
            {
                temp1|=0x0c00;
                temp2|=0x0a00;
            }
            
            if((i&0x40)!=0){
                temp1|=0x3000;
                temp2|=0x5000;
            }
            
            if((i&0x80)!=0){
                temp1|=0xc000;
                temp2|=0xa000;
            }
            StandardColorTable[i]=temp1;
            MultiColorTable[i]=temp2;
        }        
    };
    
    public void setCycles(int newCycles){
        cycles=newCycles;
    };
    public void setCoords(int x,int y){
        xcoord=x;
        ycoord=y;
    }
    
    void aStringToBuffer(String s){
        g.drawString(s,xcoord,ycoord);
    };
    
     void anIntToBuffer(int num,int x,int y){
        String s;
        s=Integer.toHexString(num);
        g.drawString(s,xcoord,ycoord);
    };
    
    void DumpVic(Graphics p){
        int rc,rr;
        aStringToBuffer("Display Mode   :");
        rr=VicIIChip[0x11]&0xff;
        rc=VicIIChip[0x16]&0xff;
        rc=((rc&0x10)|(rr&0x60))>>4;
        // ECM4 BMM2 MC
        if((rr&0x10)!=0){
            switch(rc){
                case 0x00:
                    aStringToBuffer(" Standard Text ");
                    break;
                case 0x01:
                    aStringToBuffer(" MultiColor Text ");
                    break;
                case 0x02:
                    aStringToBuffer(" Standard BitMap ");
                    break;
                case 0x03:
                    aStringToBuffer(" MultiColor BitMap ");
                    break;
                case 0x04:
                    aStringToBuffer(" Extended Text ");
                    break;
                case 0x05:
                    aStringToBuffer(" Invalid Text Mode ");
                    break;
                case 0x06:
                    aStringToBuffer(" Invalid BitMap Mode One ");
                    break;
                case 0x07:
                    aStringToBuffer(" Invalid BitMap Mode Two ");
                    break;
            }
        }
        else{
            aStringToBuffer(" Disabled Off ");
        }
        aStringToBuffer("Raster Compare : ");
        anIntToBuffer(rasterlatch,2,1);
        aStringToBuffer("\n");
        aStringToBuffer("Raster Count   : ");
        anIntToBuffer(raster,2,1);
        aStringToBuffer("\n");
    
        rr=VicIIChip[VICMEMPT]&0xff;
        rc=(Bus.readCiaB()^0x03)&0x03;
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
    
 
    
        rc=VicIIChip[0x19]&0x8f;
        rr=VicIIChip[0x1a]&0xf;
        if(rr!=0){
            aStringToBuffer ("Interupts Enabled :");
            if((rr&1)!=0)
                aStringToBuffer(" Raster         ");
            if((rr&2)!=0)
                aStringToBuffer(" Sprite2Gfx     ");
            if((rr&4)!=0)
                aStringToBuffer(" Sprite2Sprite  ");
            if((rr&8)!=0)
                aStringToBuffer(" LitePen        ");
            aStringToBuffer("\n");
        }
        else{
            aStringToBuffer("Interupts Enabled : None    \n");
        }
    
        if(rc!=0){
            aStringToBuffer("Interupts Pending :");
            if((rc&1)!=0)
                aStringToBuffer(" Raster         ");
            if((rc&2)!=0)
                aStringToBuffer(" Sprite2Gfx     ");
            if((rc&4)!=0)
                aStringToBuffer(" Sprite2Sprite  ");
            if((rc&8)!=0)
                aStringToBuffer(" LitePen        ");
            aStringToBuffer("\n");
        }
        else{
            aStringToBuffer("Interupts Pending : None    \n");
        }
        aStringToBuffer("\n");
    };

    public void upDateVicII(){
        int indexx;
        aux1=VicIIChip[VICMEMPT]&0xff;
        aux2=(Bus.readCiaB()^0x03)&0x03;
        bankid=aux2;                      //which of 4 16k banks
        matrixid=aux1;
        aux2=(aux2<<0x0e)&0xffff;         //aux2*16384
        vicbank=aux2;                     //which 16k bank
        aux1=(aux1&0xf0)<<0x06;           //aux1*64
        aux1=(aux1+aux2)&0xffff;          //add address of vic
        videomatrix=0x400;                 //screen
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
        for(indexx=0;indexx<40;indexx++){
//            VMLI[indexx]=Bus.readByte(screenbase+indexx);
//            VMLC[indexx]=Bus.readColorRam(VCCOUNT+indexx);
        }        
    };
    
    public int readVIC(int address){
        address&=0x1f;
        switch(address){
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
            case 0x1E://sprite to sprite collision register
            case 0x1F://sprite to background collision register
                //when read collision registers are cleared
                aux1=VicIIChip[address];
                VicIIChip[address]=0;
                return aux1;
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
        }
        return 0xff;        
    };
    
    private void doChanges(){
        aux1=VicIIChip[VICMEMPT]&0xff;
        aux2=(Bus.readCiaB()^0x03)&0x03;
        bankid=aux2;                      //which of 4 16k banks
        matrixid=aux1;
        aux2=(aux2<<0x0e)&0xffff;         //aux2*16384
        vicbank=aux2;                     //which 16k bank
        aux1=(aux1&0xf0)<<0x06;           //aux1*64
        aux1=(aux1+aux2)&0xffff;          //add address of vic
        videomatrix=0x400;                 //screen
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
    };
    
    public void writeVIC(int data,int address){
        address&=0x3f;
        data&=0xff;
        switch(address)
        {
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
                VicIIChip[address]=data;
                break;
            case 0x11://upper bit of write latch
                aux2=VicIIChip[address];
                aux1=data&0x80;
                VicIIChip[address]=(data&0x7f)|(aux2&0x80);
                rasterlatch=(rasterlatch&0xff)|(aux1<<0x01);
                rasterlatch&=0x1ff;
                aux2&=0x78;
                aux1=data&0x78;
                scrolly=data&0x7;
                if(aux1!=aux2) doChanges();
                if(raster==rasterlatch)
                {
                    aux1=VicIIChip[VICFLAG]&0xff;
                    aux2=VicIIChip[VICMASK]&0xff;
                    aux1|=IRST;          //flag a raster interupt.
                    if((aux2&IRST)!=0){        //is raster interupt enabled
                        aux1|=IRQV;      //cause interupt
                        Bus.setIRQ(VicIIirq); //tell bus about it
                    }
                    VicIIChip[VICFLAG]=aux1;
                }
                break;
            case 0x12:// lower byte of write latch
                rasterlatch=(rasterlatch&0x100)|data;
                if(raster==rasterlatch){
                    aux1=VicIIChip[VICFLAG]&0xff;
                    aux2=VicIIChip[VICMASK]&0xff;
                    aux1|=IRST;          //flag a raster interupt.
                    if((aux2&IRST)!=0){        //is raster interupt enabled
                        aux1|=IRQV;      //cause interupt
                        Bus.setIRQ(VicIIirq); //tell bus about it
                    }
                    VicIIChip[VICFLAG]=aux1;
                }
                break;
            case 0x13:
            case 0x14:
            case 0x15:
                VicIIChip[address]=data;
                break;
            case 0x16:
                aux1=VicIIChip[address]&0x38;
                VicIIChip[address]=data;
                aux2=data&0x38;
                scrollx=data&0x7;
                if(aux1!=aux2) doChanges();
                break;
            case 0x17:
                VicIIChip[address]=data;
                break;
            case 0x18:
                VicIIChip[address]=data|0x01;
                break;
                case 0x19://vic flag
                Bus.clearIRQ(VicIIirq);
                data=(~data)&0xf;
                aux1=VicIIChip[VICFLAG]&data;
                data=aux1;
                aux1&=VicIIChip[VICMASK];
                if(aux1!=0) data|=0x80;
                VicIIChip[VICFLAG]=data|0x70;
                break;
            case 0x1A://vic mask
                data&=0xf;
                VicIIChip[address]=data|0xf0;
                aux1=VicIIChip[VICFLAG]&0xff;
                data&=aux1;
                Bus.clearIRQ(VicIIirq);
                if(data!=0){
                    aux1|=0x80;
                    VicIIChip[VICFLAG]=aux1;
                }
                else
                {
                    aux1&=0x7f;
                    VicIIChip[VICFLAG]=aux1;
                }
                break;
            case 0x1B:
            case 0x1C:
            case 0x1D:
                VicIIChip[address]=data;
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
                VicIIChip[address]=data|0xf0;
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
    };
    
    public void hardReset(){
        raster=0;
//        display=0;
        rasterlatch=0;
        VCBASE=VCCOUNT=RC=0;
        in_screen_window=false;
        in_dma_window=false;        
        for(i=0;i<0x40;i++){
            VicIIChip[i]=0;
        }
    };
 
    private void StandardTxtMode(){
        int indexx;
        col0=VicIIChip[VBC0]&0x0f;
        for(indexx=0;indexx<320;indexx++){
            GfxLine[indexx]=col0;
        }
    
        for(indexx=0;indexx<40;indexx++){
            aux1=(VMLI[indexx]&0xff)<<3; //character
            col1=VMLC[indexx]&0x0f;        //color ram
            if(pixelrom){
                aux2=Bus.readChar(aux1+gfxchar+RC&0xff);
            }
            else{
                aux2=Bus.readByte(aux1+gfxchar+RC)&0xff;
            }
            
            if(aux2!=0){
                aux3=0x80;
                for(indexy=0;indexy<8;indexy++){
                    if((aux2&aux3)!=0){
                        sp2gfxcol[indexy]=1;
                        GfxLine[indexy+scrollx]=col1;
                    }
                    aux3>>=1;
                }
            }
        }
    };
 
    private void DrawC64DisplayMode(){
        if((raster>=DelYstart)&&(raster<=DelYend)&&display){
            in_deltay_window=true;
            if(idlestate_flag==false){
                switch(screenmode){
                    case 0x00:
                        StandardTxtMode();
                        break;
                    case 0x01:
                        StandardTxtMode();
                        break;
                    case 0x02:
                        StandardTxtMode();
                        break;
                    case 0x03:
                        StandardTxtMode();
                        break;
                    case 0x04:
                        StandardTxtMode();
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
            else{
                StandardTxtMode();
            }
        }
    };
    
    private void ComputeBadline(){
        badline_flag=false;
        VCCOUNT=VCBASE;
        in_screen_window=true;
        scrolly=txtmode&0x7;
 
        /* is the line within dma start and end? */
        if((raster>=DISPLAY_DMA_START)&&(raster<=DISPLAY_DMA_END)){
            in_dma_window=true;
        
            /* is this a badline? */
            if((scrolly==(raster&0x07))&&(badline_enable_flag)){
                RC=0;
                Bus.CPUyesBadline();
                display=screen_enable_flag;
                idlestate_flag=false;
                badline_flag=true;  
                upDateVicII();
            }
        }
    };
    
    public void renderline(){
        col0=GfxLine[0];
        col0=VicIIChip[BORDERCOLOR];
        g.setColor(coltable[col0&0xf]);        
        for(indexx=0;indexx<323;indexx++){
            col1=GfxLine[indexx];
            if(col0!=col1){
                col0=GfxLine[indexx];
                g.setColor(coltable[col1&0xf]);
            }
            g.setColor(Color.red);
            g.drawLine(indexx,raster,indexx,raster);          
        }
    };
    
    public void execute(){
        aux1=VicIIChip[VICCONREG1]&0xff;       //vic control reg 1
        aux2=aux1&0x7f;                   //copy read latch
        aux3=VicIIChip[VICFLAG]&0xff;          //interupt flag
        aux4=VicIIChip[VICMASK]&0xff;          //interupt mask
        raster++;
        isdone=false;
        if(raster>311){  
            raster=0;
            isdone=true;
        }
        
        VicIIChip[RASTER]=raster&0xff;
        aux1=(aux2|((raster>>1)&0x80));
        VicIIChip[VICCONREG1]=aux1;       //vic control reg 1
        //check for raster interupt
        if(raster==rasterlatch){
            aux3|=IRST;          //flag a raster interupt.
            if((aux4&IRST)!=0){        //is raster interupt enabled
                aux3|=IRQV;      //cause interupt
                Bus.setIRQ(VicIIirq); //tell bus about it
            }
            VicIIChip[VICFLAG]=aux3;
        }
        in_dma_window=false;
        in_deltay_window=false;
        txtmode=aux1;
    
        aux3=aux1&DEN;
        if(raster==DISPLAY_DMA_START){
            if(aux3!=0) badline_enable_flag=true;
        }
        
        if( aux3!=0){
            screen_enable_flag=true;
        }
        
        scrmode=VicIIChip[VICCONREG2]&0xff;       //vic control reg 2
        if((raster>=0x33)&&(raster<=0xfb)){
            for(indexx=0;indexx<0x180;indexx++){
                sp2gfxcol[indexx]=0;
            }
        }
    
        Bus.CPUnoBadline();
        if((raster>=FIRST_RASTER_LINE)&&
                (raster<=LAST_RASTER_LINE)){
            ComputeBadline();
            /* is display turned on? */

            if((raster>=DISPLAY_ROW25_START)&&
                    (raster<=DISPLAY_ROW25_END)){
                gfxchar=charbank;              // start of character pointer
                pixelrom=false;
                if((bankid==0)||(bankid==2)){
                    pixelrom=true;
                    if(matrixid==6) gfxchar=0x800;
                    if(matrixid==4) gfxchar=0x00;
                }
            
                //calculate number of rows
                if((txtmode&RSEL)!=0){
                    DelYstart=DISPLAY_ROW25_START;
                    DelYend=DISPLAY_ROW25_END;
                }
                else{
                    DelYstart=DISPLAY_ROW24_START;
                    DelYend=DISPLAY_ROW24_END;
                }
            
                DrawC64DisplayMode();
                //   DrawSprites();
                if((in_deltay_window==false)
                        ||((txtmode&DEN)==0)){
                    col0=VicIIChip[BORDERCOLOR]&0x0f;
                    for(indexx=0;indexx<320;indexx++){
                        GfxLine[indexx+24]=col0;
                    }
                }
            
                //how many columns
                if((scrmode&CSEL)!=0){
                    // left & right by 8 pixels for scrolling
                    col0=VicIIChip[BORDERCOLOR]&0x0f;
                    for(indexx=0;indexx<4;indexx++){
                        GfxLine[24+indexx]=col0;
                        GfxLine[(24+313)+indexx]=col0;
                    }
                }
            }
            else{
                if(raster<=0xff){
                    //    DrawSprites();
                }
            }
        
            if(RC==0x7){
                idlestate_flag=true;
            }
            else{
                RC++;
                RC&=0x7;
            }
     
            if(badline_flag){
                idlestate_flag=false;
                VCBASE+=40;
            }
        
            badline_flag=false;
        
            if(raster==0xfb){
                VCBASE=VCCOUNT=0;
                RC=0;
                idlestate_flag=false;
                badline_flag=false;
                badline_enable_flag=false;
                display=false;
                /* clear collision buffer */
                for(indexx=0;indexx<0x180;indexx++){
                    sp2gfxcol[indexx]=0;
                }
            }
        }
        /* Render C64 raster line to PC raster line */
//        Render(GfxLine+24,raster,VicIIChip[BORDERCOLOR]&0xf); 
        renderline();
        };
    }
