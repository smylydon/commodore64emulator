
#include "include/mytypes.h"
#include <dpmi.h>
#include <dos.h>
#include <go32.h>
#include <pc.h>
#include <conio.h>
#include <ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/farptr.h>
#include <sys/segments.h>
#include <sys/movedata.h>
#include <dir.h>
#include "include/M6510.h"

#define 	t64desc		0
#define	t64ver		32
#define	t64max		34
#define	t64nfp		36
#define	t64user		40
#define	t64slots		64

#define	slotflag		0+t64slots
#define	t64ftype		1+t64slots
#define	t64start		2+t64slots
#define	t64end		4+t64slots
#define	t64ifstart		8+t64slots
#define	t64fname		16+t64slots

extern void LoadVic(dword *p);
extern void SaveVic(dword *p);

struct T64
{
	dword	mem_start;
	dword	mem_end;
	dword	file_start;
	dword	file_end;
	dword	file_length;
	byte	*file_name;
	byte	buffer[0xff];
};

extern dword joystick;

/*---------------------------------------------------------*/
/* C64 file functions                                      */
/*---------------------------------------------------------*/

/* Load State */
void s64load(byte *name)
{
	FILE *file;
	dword fileimage[0xff];
	dword *p;
	p=&cputimer;
	file=fopen(name,"rb");
	if(file)								/* does the file exist */
	{
		fread(RAM,sizeof(char),0x10000,file);		/* read 64k into ram */
		fread(VIC2CHIP,sizeof(char),0x40,file);		/* Load Vic II (poke) Image */
		fread(CIACHIP1,sizeof(char),0x40,file);
		fread(CIACHIP2,sizeof(char),0x40,file);
		fread(SIDCHIP,sizeof(char),0x40,file);		/* read 4k into color ram */
		fread(COLORRAM,sizeof(char),0x800,file);
		fread(fileimage,sizeof(dword),0x7f,file);		/* read cpu status to temp buffer */
		cpu.areg=fileimage[0x00];				/* copy temp buffer to cpu structure */
		cpu.xreg=fileimage[0x01];
		cpu.yreg=fileimage[0x02];
		cpu.psr=fileimage[0x03];
		cpu.psp=fileimage[0x04];
		cpu.ppc=fileimage[0x05];
		cpu.ppc2=fileimage[0x06];
		cpu.irq=fileimage[0x07];
		cpu.nmi=fileimage[0x08];
		*p++=fileimage[0x20];
		*p++=fileimage[0x21];
		*p++=fileimage[0x22];
		*p++=fileimage[0x23];	
		*p++=fileimage[0x24];
		if(joystick) joystick=(byte)fileimage[0x25];
		fread(fileimage,sizeof(dword),0xff,file);
		LoadVic(fileimage);					/* Load (peek) Vic II image */
		fclose(file);
		delay(100);
	}
}

/* Save state */
void s64save(byte *name)
{
	FILE *file;
	dword fileimage[0xff];						/* temporary buffer */
	dword *p;
	p=&cputimer;
	file=fopen(name,"wb");
	if(file)								/* does file exist */
	{
		fileimage[0x00]=cpu.areg;				/* copy cpu status to temp buffer */
		fileimage[0x01]=cpu.xreg;
		fileimage[0x02]=cpu.yreg;
		fileimage[0x03]=cpu.psr;
		fileimage[0x04]=cpu.psp;
		fileimage[0x05]=cpu.ppc;
		fileimage[0x06]=cpu.ppc2;
		fileimage[0x07]=cpu.irq;
		fileimage[0x08]=cpu.nmi;
		fileimage[0x20]=*p++;					/* misc. variables */
		fileimage[0x21]=*p++;
		fileimage[0x22]=*p++;
		fileimage[0x23]=*p++;	
		fileimage[0x24]=*p++;
		fileimage[0x25]=(dword)joystick;			/* copy joystick */	
		fwrite(RAM,sizeof(char),0x10000,file);		/* save 64k ram */
		fwrite(VIC2CHIP,sizeof(char),0x40,file);		/* save Vic Poke Image */
		fwrite(CIACHIP1,sizeof(char),0x40,file);
		fwrite(CIACHIP2,sizeof(char),0x40,file);
		fwrite(SIDCHIP,sizeof(char),0x40,file);
		fwrite(COLORRAM,sizeof(char),0x800,file);
		fwrite(fileimage,sizeof(dword),0x7f,file);
		SaveVic(fileimage);					/* save Vic Peek Image */
		fwrite(fileimage,sizeof(dword),0xff,file);
		fclose(file);
	}
}

/* Load from T64 image */
void t64load(byte *name)
{
	FILE *file;
	struct T64 tape64;
	int index;
	tape64.file_name=name;
	file=fopen(name,"rb");
	if(file)											/* does file exist? */
	{
		for(index=0;index<128;index++)
		{
			tape64.buffer[index]=fgetc(file);
		}
		tape64.mem_start=tape64.buffer[t64start]+(tape64.buffer[t64start+1]<<8);
		tape64.mem_end=tape64.buffer[t64end]+(tape64.buffer[t64end+1]<<8);
		tape64.file_start=tape64.buffer[t64ifstart]+(tape64.buffer[t64ifstart+1]<<8);
		rewind(file);
		tape64.file_length=(tape64.mem_end-tape64.mem_start);
		fseek(file,tape64.file_start,SEEK_SET);
		fread(RAM+tape64.mem_start,sizeof(char),tape64.file_length,file);
		fclose(file);
		delay(100);
	}
}

void p00load(byte *name)
{
	FILE *file;
	int quit=0;
	dword x=0x801;
	file=fopen(name,"rb");
	fseek(file,0x1a,SEEK_SET);
	x=fgetc(file)&0xff;
	x=x+((fgetc(file)&0xff)<<8);
	while(!quit)
	{
		if(feof(file))
		{
			quit=0xff;
		}
		else
		{
			RAM[x++]=fgetc(file);
		}
	}
}

void c64load(byte *name)
{
	FILE *file;
	int quit=0;
	dword x=0;
	file=fopen(name,"rb");
//	fseek(file,0x1a,SEEK_SET);
	x=fgetc(file)&0xff;
	x=x+((fgetc(file)&0xff)<<8);
	while(!quit)
	{
		if(feof(file))
		{
			quit=0xff;
		}
		else
		{
			RAM[x++]=fgetc(file);
		}
	}
}
