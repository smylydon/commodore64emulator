#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "gfx.h"
#include "gui.h"
#include "AddressBus.h"


#define t64ver  0x20
#define t64max  0x22
#define t64nfp  0x24


#define t64ftype   0x40
#define t64start   0x42
#define t64ifstart 0x48
#define t64end     0x44
#define t64fname   0x28

//extern void LoadVic(unsigned int *p);
//extern void SaveVic(unsigned int *p);

struct T64 {
    unsigned int memStart;
    unsigned int memEnd;
    unsigned int fileStart;
    unsigned int fileEnd;
    unsigned int fileLength;
    char fileName[0xff];
    unsigned char buffer[0xff];
    unsigned char *memory;
};


//extern unsigned int joystick;

/*---------------------------------------------------------*/
/* C64 file functions                                      */
/*---------------------------------------------------------*/
/* Load State */
void s64load(struct fileselector *F) {
    FILE *file;
    struct ioState p;
    file=fopen(F->filename,"rb");
    if(file)								/* does the file exist */
    {
        fread(p.mem,sizeof(char),0x10000,file);		/* read 64k into ram */
        fread(p.pokevic,sizeof(char),0x40,file);		/* Load Vic II (poke) Image */
        fread(p.cia1,sizeof(char),0x40,file);
        fread(p.cia2,sizeof(char),0x40,file);
        fread(p.sid,sizeof(char),0x40,file);		/* read 4k into color ram */
        fread(p.colram,sizeof(char),0x800,file);
        fread(p.cpu,sizeof(unsigned int),0x7f,file);		/* read cpu status to temp buffer */
        fread(p.peekvic,sizeof(unsigned int),0xff,file);
        BusLoadState(&p);
    }
     else {
        mns_setColors(cWhite,cBlack);
        mns_textOut(400,50,"Failed to LoadState");
    }
    fclose(file);
}

/* Save state */
void s64save(struct fileselector *F) {
    FILE *file;
    struct ioState p;

    file=fopen(F->filename,"wb");
    if(file)								/* does file exist */
    {
        BusSaveState(&p);
        fwrite(p.mem,sizeof(char),0x10000,file);		/* read 64k into ram */
        fwrite(p.pokevic,sizeof(char),0x40,file);		/* Load Vic II (poke) Image */
        fwrite(p.cia1,sizeof(char),0x40,file);
        fwrite(p.cia2,sizeof(char),0x40,file);
        fwrite(p.sid,sizeof(char),0x40,file);		/* read 4k into color ram */
        fwrite(p.colram,sizeof(char),0x800,file);
        fwrite(p.cpu,sizeof(unsigned int),0x7f,file);
        fwrite(p.peekvic,sizeof(unsigned int),0xff,file);
    }
    else {
        mns_setColors(cWhite,cBlack);
        mns_textOut(400,50,"Failed to SaveState");
    }
    fclose(file);
}

/* Load from T64 image */
void t64load(struct fileselector *F) {
    FILE *file;
    struct T64 tape64;
    unsigned int indexx,i;
    tape64.memory=0;
    tape64.memory=malloc(sizeof(unsigned char)*0x100ff);
    if(tape64.memory==0) {
        mns_setColors(cWhite,cBlack);
        mns_textOut(400,50,"Failed to allocate memory");
        return;
    }

    file=fopen(F->filename,"rb");
    if(file)           /* does file exist? */
    {
        for(indexx=0;indexx<0xff;indexx++) {
            tape64.buffer[indexx]=fgetc(file);
        }
        /*
              i=fread(tape64.buffer,sizeof(unsigned char),0xff,file);
              if(i!=0xff) {
                  mns_setColors(cWhite,cBlack);
                  mns_textOut(400,50,"HEADER READ ERROR");
                  fclose(file);
                  return;
              }*/
        rewind(file);
        tape64.memStart=tape64.buffer[t64start]+(tape64.buffer[t64start+1]<<8)&0xffff;
        tape64.memEnd=(tape64.buffer[t64end]+(tape64.buffer[t64end+1]<<8))&0xffff;
        tape64.fileStart=(tape64.buffer[t64ifstart]+(tape64.buffer[t64ifstart+1]<<8))&0xffff;

        tape64.fileLength=(tape64.memEnd-tape64.memStart)&0xffff;
        fseek(file,tape64.fileStart,SEEK_SET);
        /*        i=fread(tape64.memory,sizeof(unsigned char),tape64.fileLength,file);

                if(i!=tape64.fileLength) {
                    mns_setColors(cWhite,cBlack);
                    mns_textOut(400,60,"BODY READ ERROR");
                    fclose(file);
                        free(tape64.memory);
                    return;
                }*/
        for(indexx=0;indexx<tape64.fileLength;indexx++) {
            //            BusWriteRam(tape64.memory[indexx],tape64.memStart++);
            BusWriteRam(fgetc(file),tape64.memStart++);
        }
        fclose(file);
    }
    mns_setColors(cWhite,cBlack);
    mns_textOut(400,60,"SUCCESS");
    free(tape64.memory);
}

void p00load(struct fileselector *F) {
    FILE *file;
    int quit=0;
    unsigned int x;
    file=fopen(F->filename,"r");
    fseek(file,0x1a,SEEK_SET);
    x=fgetc(file)&0xff;
    x=(x+((fgetc(file)&0xff)<<8))&0xffff;
    if(file) {
        while(!quit) {
            if(feof(file)) {
                quit=0xff;
            }
            else {
                BusWriteRam(fgetc(file),x++);
            }
        }
        fclose(file);
    }
}

void c64load(struct fileselector *F) {
    FILE *file;
    int quit=0;
    unsigned int x=0;
    file=fopen(F->filename,"rb");
    // fseek(file,0x1a,SEEK_SET);
    x=fgetc(file)&0xff;
    x=(x+((fgetc(file)&0xff)<<8))&0xffff;
    while(!quit) {
        if(feof(file)) {
            quit=0xff;
        }
        else {
            BusWriteRam(fgetc(file),x++);
        }
    }
}


