
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <dpmi.h>
#include <go32.h>
#include <inlines/pc.h>
#include <math.h>
#include <sys\farptr.h>

// ### DEFINES ##############################################################
#define DSPRESET 0x6
#define DSPREAD 0xA
#define DSPWRITE 0xC
#define DSPSTAT 0xE
#define wantedRate 44100

#define ATTACK 0	//A of ADSR
#define DECAY 1	//D of ADSR
#define SUSTAIN 2	//S of ADSR
#define RELEASE 3	//R of ADSR

#define waveson 1
#define sbon  2

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long qword;

typedef struct
{
	long length;
	long loopto;
	short *buf;
} sndsamp;

typedef struct
{
      sndsamp *cursamp;
	long sampptr;
	qword envx,envclk;
	int envstate;
	int ar,dr,sl,sr;
	int counter;
} sndvoice;

typedef enum
{
	SB_SUCCESS,
	SB_FAILURE,
	SB_BAD_BLASTER,
	SB_BAD_ADDRESS,
	SB_BAD_IRQ,
	SB_BAD_DMA,
	SB_BAD_FILE,
	SB_BUSY,
	SB_BAD_POINTER
} sb_status;
	
struct
{                        /* This holds the vital info on the Sound- */
	word reset;                   /* Blaster, like where it's located, which */
	word readData;                /* IRQ it uses, and which DSP it had.      */
	word writeData;
	word dataAvail;
	word base;
	int IRQ;
	int DMA;
	int dspVersion;
} Sound;

// ### PROTOTYPES ############################################################
void sb_microWait(word usec);
void sb_waitInit(void);


void WriteDSP(byte val);
int ReadDSP(void);
int ResetDSP(void);

int InitWaves(void);
long SNDDoEnv(long voice);
void SNDMix(void);
void PokeSID(void);
void SNDNoteOn(unsigned char i);
void SNDNoteOff(unsigned char i);
int SNDInit(long freq, long urate);

int findInterrupt(void);
sb_status findSoundBlaster(void);
int sb_install(void);
void sb_uninstall(void);
void sb_reset(void);

// ### VARIABLES ############################################################
extern byte SIDCHIP[128];
extern dword gatez;
static volatile int testCount;
static int dontSetDMA=0;

byte *DMABuffer;

byte *SBbuf;


sndsamp *SNDsamples[256];
sndvoice SNDvoices[3];
short *SNDbuf;

byte v1on=1,v2on=1,v3on=1;
long SNDfreq,SNDurate,SNDkeys,SNDvmask,SNDmixlen,leds;
dword SIDPoke,SIDAddr;

qword C[0x10]=
{
        0x20000000,0x8000000,0x4000000,0x2AAAAAA,0x1AF286B,
        0x1249249,0xF0F0F0,0xCCCCCC,0xA3D70A,0x418937,0x20C49B,0x147AE1,0x10624D,
        0x57619,0x346DC,0x20C49
};
        
int percent;
qword cyc_orig=985000;
int cycles=1;
int skipper=1;
qword dmabuflen;
int deinstall=0;
_go32_dpmi_seginfo  OldIRQ, MyIRQ;

//Pointer to DOS DMA buffer
_go32_dpmi_seginfo         DOSBuf;
int                     DOSBufOfs;

// ### SB Detection ########################################################
static void testInt2(void)
{
	testCount=2;
	inportb(Sound.dataAvail);
	outportb(0x20,0x20);
}

static void testInt3(void)
{
	testCount=3;
	inportb(Sound.dataAvail);
	outportb(0x20,0x20);
}

static void testInt5(void)
{
	testCount=5;
	inportb(Sound.dataAvail);
	outportb(0x20,0x20);
}

static void testInt7(void)
{
	testCount=7;
	inportb(Sound.dataAvail);
	outportb(0x20,0x20);
}

static void testInt10(void)
{
	testCount=10;
	inportb(Sound.dataAvail);
	outportb(0x20,0x20);
	outportb(0xA0,0x20);
}

static void SB8Convert(void)
{
	long int i;
	for(i=SNDmixlen-1;i>=0;i--)
	{
		SBbuf[i]=(SNDbuf[i]>>8)^0x80;
	}
}

void sb_playDMA()
{
	dword  Page, OffSet;
	word length=SNDmixlen-1;

	int DMA=Sound.DMA;

	Page = DOSBufOfs >> 16;					//Calculate page
	OffSet = DOSBufOfs & 0xFFFF;				//Calculate offset in the page
	
	outportb (0x0A, 4 | DMA);             //Mask DMA channel
	outportb (0x0C, 0);                   //Clear byte pointer
	outportb (0x0B, 0x48 | DMA);          //Set mode
	outportb (DMA << 1, OffSet & 0xFF);   //Write the offset to the DMA controller
	outportb (DMA << 1, OffSet >> 8);     //Write the offset to the DMA controller
  /*
    The mode consists of the following:
	0x48+x = binary 01 00 10 xx
                    |  |  |  |
                    |  |  |  +- DMA channel
                    |  |  +---- Read operation (the DSP reads from memory)
                    |  +------- Single cycle mode
                    +---------- Block mode
  */

  //Write the page to the DMA controller
	if (DMA == 0) outportb (0x87, Page);
	if (DMA == 1) outportb (0x83, Page);
	if (DMA == 3) outportb (0x82, Page);

  //Set the block length
	outportb ((DMA << 1) + 1, length&0xff);
	outportb ((DMA << 1) + 1, length>> 8);
	outportb (0x0A, DMA);             //Unmask DMA channel

  //DSP-command 14h - 8bit single cycle playback
	WriteDSP (0x14);
	WriteDSP (length& 0xFF);
	WriteDSP (length>> 8);
}

static void ServiceIRQ ()
{
	int x=Sound.DMA;

	//Copy as one or in parts

	SNDMix();

	dosmemput (SBbuf, SNDmixlen-1, DOSBufOfs);
	SB8Convert();
	//Relieve DSP
	inportb(Sound.dataAvail);
	//Acknowledge hardware interrupt
	outportb (0x20, 0x20);
	//Acknowledge cascade interrupt for IRQ 2, 10 and 11
	if (x == 2 || x == 10 || x == 11)
	{
		outportb (0xA0, 0x20);
	}
	sb_playDMA();
}

void StartDMA()
{

	WriteDSP (0xD1);						//DSP-command D1h - Enable speaker, required
									//on older SB's
	WriteDSP (0x40);						//DSP-command 40h - Set sample frequency
	WriteDSP ((256-(1000000/SNDfreq)));			//Write time constant
	SNDMix();
	SB8Convert();
						//DSP-command 1Ch - Start auto-init playback
	sb_playDMA();

}

void AllocateBuffer()
{
	_go32_dpmi_seginfo TempBuf; //Temporary pointer
	unsigned int  Page1, Page2; //Words

	//Assign 32K to DMA Buffer
	DMABuffer = (char *)malloc (32768);

	//Assign 32K (2048 paragraphs) of DOS memory
	TempBuf.size = 2048;
	_go32_dpmi_allocate_dos_memory (&TempBuf);

	//Calculate linear address
	DOSBufOfs = TempBuf.rm_segment << 4;

	//Calculate page at start of buffer
	Page1 = DOSBufOfs >> 16;

	//Calculate page at end of buffer}
	Page2 = (DOSBufOfs + 32767) >> 16;

	//Check to see if a page boundary is crossed
	if (Page1 != Page2)
	{
	//If so, assign another part of memory to the buffer
		DOSBuf.size = 2048;
		_go32_dpmi_allocate_dos_memory (&DOSBuf);
		DOSBufOfs = DOSBuf.rm_segment << 4;
		_go32_dpmi_free_dos_memory (&TempBuf);
	}
	else
	{
	//otherwise, use the part we've already allocated
		DOSBuf = TempBuf;
	}

	//Clear DMA buffers
	memset (DMABuffer, 128, 0x7FFF);
	dosmemput (DMABuffer, 32768, DOSBufOfs);
}

void sb_waitInit(void)
{
	int temp;
	temp=inportb(0x61);
	temp&=0xFD;
	temp|=0x01;
	outportb(0x61,temp);
}

void sb_microWait(word usec)
{
	word elapsed;
	unsigned long failsafe;

	outportb(0x43,0xB0);
	outportb(0x42,0xFF);
	outportb(0x42,0xFF);

	/* Sometimes this timer doesn't seem to work, and our program hangs! */
	failsafe = usec * 10000;

	do
	{
		outportb(0x43,0x80);
		elapsed=inportb(0x42);
		elapsed|=(inportb(0x42)<<8);
		elapsed=~elapsed;
	}
	while(elapsed<usec && failsafe--);
}

// ### ROUTINES #############################################################


void WriteDSP(byte val)
{
	while((inportb(Sound.writeData)&0x80)!=0);
	outportb(Sound.writeData,val);
}

int ReadDSP(void)
{
	while((inportb(Sound.dataAvail)&0x80)==0);
	return inportb(Sound.readData);
}

int ResetDSP(void)
{
	int a;
	int success;

	outportb(Sound.reset,1);

	sb_microWait(4);

	outportb(Sound.reset,0);

	success=0;
	for(a=0;a<1000;a++)
	{
		if(inportb(Sound.dataAvail)&0x80)
		{
			success=1;
			break;
		}
	}

	if(success)
	{
		for(a=0;a<1000;a++)
		{
			if(inportb(Sound.readData)==0xAA)
			{
				success=2;
				break;
			}
		}
	}

	if(success!=2) return 0;
	WriteDSP(0xE1);
	Sound.dspVersion=ReadDSP();
	Sound.dspVersion<<=8;
	Sound.dspVersion|=ReadDSP();
	return 1;
}

void setvect (int Vector)
{
	//Get location of the new keyboard handler
	MyIRQ.pm_offset = (int)ServiceIRQ;
	MyIRQ.pm_selector = _go32_my_cs ();
	//Save the old interrupt handler
	_go32_dpmi_get_protected_mode_interrupt_vector (Vector, &OldIRQ);
	//Set the new interrupt handler in the chain
	_go32_dpmi_chain_protected_mode_interrupt_vector (Vector, &MyIRQ);
}

void resetvect (int Vector)
{
	//Set interrupt vector to the BIOS handler
	_go32_dpmi_set_protected_mode_interrupt_vector (Vector, &OldIRQ);
}
// ### Init&Mixer ###########################################################

static sndsamp *NewSamp(long size)
{
	sndsamp *s;
	if(((s=malloc(sizeof(sndsamp)))==NULL)||((s->buf=malloc(size*2))==NULL)) return(NULL);
	s->length=size;
	s->loopto=-1;
	return(s);
}

int InitWaves(void)
{
	int i,j,z;
	long t;

	short *TRI;
	short *SAW;
	short *SQR;
	short *NSE;
	
	if(((TRI=malloc(128))==NULL)||((SQR=malloc(128))==NULL))
	{
		printf("not enuff mem for TRI&SQR waves\n");
		return (SB_FAILURE);
	}
	for(z=0;z<16;z++)
	{
		SQR[z]=-8192;
	}
	for(;z<32;z++)
	{
		SQR[z]=8192;
	}
	for(;z<48;z++)
	{
		SQR[z]=-8192;
	}
	for(;z<64;z++)
	{
		SQR[z]=8192;
	}
	t=-8192;
	for(i=0;i<32;i++)
	{
		t+=512;
		TRI[i]=t;
		SQR[i]=-8192;
	}
	
	for(;i<64;i++)
	{
		t-=512;
		TRI[i]=t;
		SQR[i]=8192;
	}
	
	if((SAW=malloc(128))==NULL)
	{
		printf("not enuff mem for SAW wave\n");
		return (SB_FAILURE);
	}	
	for(i=0;i<64;i++)
	{
		t+=256;
		SAW[i]=t;
	}
	
	if((NSE=malloc(128))==NULL)
	{
		printf("not enuff mem for NSE wave\n");
		return (SB_FAILURE);
	}
	srandom(0x1fff);
	for(i=0;i<64;i++)
		NSE[i]=(rand()&0x1FFF);
	for(j=0;j<256;j++)
	{
		if((SNDsamples[j]=NewSamp(64))==NULL)
		{
			printf("not enuff mem to process waves\n");
			return (SB_FAILURE);
		}
		SNDsamples[j]->loopto=0;
		for(i=0;i<64;i++)
		{
			t=0xFFFF;
//			t=0x0000;
			if(j&1){t=t+TRI[i];}
			if(j&2){t=t+SAW[i];}
			if(j&4){t=t+SQR[i];}
			if(j&8){t=t+NSE[i];}
			SNDsamples[j]->buf[i]=t;
		}
	}
	free(TRI);
	free(SQR);
	free(NSE);
	free(SAW);
	printf("Waves initiated successfully \n");
	return (SB_SUCCESS);
}


long SNDDoEnv(long voice)
{
	qword envx=SNDvoices[voice].envx;
	SNDvoices[voice].counter++;
	if (SNDvoices[voice].counter>10)
	{
		leds&=~(1<<voice);
	}
	switch (SNDvoices[voice].envstate)
	{
		case ATTACK:
			envx+=C[SNDvoices[voice].ar]<<1;
			if(envx>=0x7F000000)
			{
				envx=0x7F000000;
				if(SNDvoices[voice].sl!=0xF)
				{
					SNDvoices[voice].envstate=DECAY;
				}
				else
				{
					SNDvoices[voice].envstate=SUSTAIN;
				}
			}
			break;
		case DECAY:
			envx-=(C[SNDvoices[voice].dr]/3)<<1;
			if((envx<=0x8000000*SNDvoices[voice].sl)||(envx>0x80000000))
			{
				envx=0x8000000*SNDvoices[voice].sl;
				SNDvoices[voice].envstate=SUSTAIN;
			}
			break;
		case RELEASE:
			envx-=(C[SNDvoices[voice].sr]/3)<<1;
			if((envx>0x80000000)||(envx==0))
			{
				envx=0;
				SNDkeys&=~(1<<voice);
			}
			break;
		}
	return SNDvoices[voice].envx=envx;
}

void SNDMix(void)
{
	long i,voice,ssmp,vol,temp,voc;
	qword pitch,ratio,sptr,len;
	short *sbuf;

	memset(SNDbuf,0,SNDmixlen<<1);
	if (v1on==0)
	{
		SNDkeys&=~(1<<0);
	}
	if (v2on==0)
	{
		SNDkeys&=~(1<<1);
	}
	if (v3on==0)
	{
		SNDkeys&=~(1<<2);
	}
	for(voice=0;voice<3;voice++)
	{
		if(SNDkeys&(1<<voice))
		{
			voc=voice*7;
			vol=SNDDoEnv(voice)>>23;
			pitch=(qword)((SIDCHIP[voc]|(SIDCHIP[voc+1]<<8))&0xffff)<<2;
			ratio=(pitch<<14)/SNDfreq;
			sbuf=SNDvoices[voice].cursamp->buf;
			sptr=SNDvoices[voice].sampptr;
			len=SNDvoices[voice].cursamp->length<<14;
			for(i=0;i<SNDmixlen;i++)
			{
				ssmp=sbuf[sptr>>14];
				ssmp=(ssmp*vol)>>8;
				temp=ssmp+SNDbuf[i];
				if(temp>32767)
				{
					temp=32767;
				}
				else
				{
					if(temp<-32767)
					{
						temp=-32767;
					}
				}
				SNDbuf[i]=temp;
				sptr+=ratio;
				if(sptr>=len)
				{
					sptr-=len;
				}
			}
			SNDvoices[voice].sampptr=sptr;
      	}
	}
}

void SNDNoteOn(unsigned char i)
{
	long cursamp;
	byte adsr1,adsr2;
	int voc=i*7;
	cursamp=((SIDCHIP[voc+4]>>4)&0xF);
	if (cursamp!=1&&cursamp!=2&&cursamp!=4&&cursamp!=8) cursamp=8;
	SNDvoices[i].cursamp=SNDsamples[cursamp%16];
	SNDvoices[i].sampptr=0;
	SNDkeys|=(1<<i);
//	leds|=(1<<i);
	SNDvoices[i].counter=0;
	//figure ADSR/GAIN
	adsr1=SIDCHIP[voc+5];
	adsr2=SIDCHIP[voc+6];
	SNDvoices[i].envx=0;
	SNDvoices[i].envclk=0;
	SNDvoices[i].envstate=ATTACK;
	SNDvoices[i].ar=adsr1>>4;
	SNDvoices[i].dr=adsr1&0xF;
	SNDvoices[i].sr=adsr2&0xf;
	SNDvoices[i].sl=adsr2>>4;
}

void SNDNoteOff(unsigned char i)
{
	SNDvoices[i].envstate=RELEASE;
	SNDvoices[i].envclk=0;
}

void PokeSID()
{
	byte temp=SIDCHIP[SIDAddr];
	SIDCHIP[SIDAddr]=SIDPoke;
	switch(SIDAddr)
	{
		case 4:
			if((SIDPoke^temp)&0xf1)
			{
				if(SIDPoke&0x1)
				{
					SNDNoteOn(0);
				}
				else
				{
					SNDNoteOff(0);
				}
			}
			break;
		case 5:
			SNDvoices[0].ar=SIDPoke>>4;
			SNDvoices[0].dr=SIDPoke&0xF;
			break;
		case 6:
			SNDvoices[0].sr=SIDPoke&0xf;
			SNDvoices[0].sl=SIDPoke>>4;
			break;
		case 11:
			if((SIDPoke^temp)&0xf1)
			{
				if(SIDPoke&0x1)
				{
					SNDNoteOn(1);
				}
				else
				{
					SNDNoteOff(1);
				}
			}
			break;
		case 12:
			SNDvoices[1].ar=SIDPoke>>4;
			SNDvoices[1].dr=SIDPoke&0xF;
			break;
		case 13:
			SNDvoices[1].sr=SIDPoke&0xf;
			SNDvoices[1].sl=SIDPoke>>4;
			break;
		case 18:
			if((SIDPoke^temp)&0xf1)
			{
				if(SIDPoke&0x1)
				{
					SNDNoteOn(2);
				}
				else
				{
					SNDNoteOff(2);
				}
			}
			break;
		case 19:
			SNDvoices[2].ar=SIDPoke>>4;
			SNDvoices[2].dr=SIDPoke&0xF;
			break;
		case 20:
			SNDvoices[2].sr=SIDPoke&0xf;
			SNDvoices[2].sl=SIDPoke>>4;
			break;
		default:
			break;
	}
}

void sbvol (unsigned char vol)
{
        outportb (Sound.base+4,4);
        outportb (Sound.base+5,(vol<<4)|vol);
}

int SNDInit(long freq, long urate)
{
	unsigned long t;
	long i;
	SNDfreq=freq;
	SNDurate=urate;
	SNDkeys=0;
	//Which keys are initially on (none)
	leds=0;
	SNDmixlen=SNDfreq/SNDurate;
	//How long mixing buffer is (samples)
	SNDbuf=malloc(SNDmixlen<<1);
	SBbuf=malloc(SNDmixlen<<1);	//Mixing buffer
	//Mixing buffer

	cyc_orig=9867L/SNDurate;

	//How many cycles per update should be

	C[0xF]*=(1000L/SNDurate);
	for(i=0xE;i;i--)
	{
		t=C[i]*(1000L/SNDurate);
	if(t<C[i+1])
		C[i]=0xFFFFFFFF;
	//overflow
	else
		C[i]=t;
	}
	if(InitWaves()==SB_FAILURE)
	{
		printf("could not initiate waves\n");
		return (SB_FAILURE);
	}
	for(i=0;i<3;i++)
	{
		SNDvoices[i].sampptr=-1;
		SNDvoices[i].envx=0;
	}
	printf("Samples installed successfully\n");
	return(SB_SUCCESS);
}

void SNDreset()
{
	long int i;
	SNDNoteOff(1);
	SNDNoteOff(2);
	SNDNoteOff(3);
	for(i=SNDmixlen-1;i>=0;i--)
	{
		SBbuf[i]=0;
	}
}

// ### SB Setup ############################################################
int findInterrupt(void)
{
	__dpmi_paddr old2, old3, old5, old7, old10;
	__dpmi_paddr new2, new3, new5, new7, new10;
	_go32_dpmi_seginfo wrap2, wrap3, wrap5, wrap7, wrap10;
	byte pic1Default, pic2Default;

	testCount=0;

	if(!(_go32_dpmi_lock_code(testInt2,(dword)sb_waitInit-(dword)testInt2)))
	{
		_go32_dpmi_lock_data(&testCount,sizeof(int));
	}
	else
	{
		return (SB_FAILURE);
	}

	__dpmi_get_protected_mode_interrupt_vector(0x0A,&old2);
	__dpmi_get_protected_mode_interrupt_vector(0x0B,&old3);
	__dpmi_get_protected_mode_interrupt_vector(0x0D,&old5);
	__dpmi_get_protected_mode_interrupt_vector(0x0F,&old7);
	__dpmi_get_protected_mode_interrupt_vector(0x72,&old10);

	wrap2.pm_offset=(int)testInt2;
	wrap2.pm_selector=_my_cs();
	wrap3.pm_offset=(int)testInt3;
	wrap3.pm_selector=_my_cs();
	wrap5.pm_offset=(int)testInt5;
	wrap5.pm_selector=_my_cs();
	wrap7.pm_offset=(int)testInt7;
	wrap7.pm_selector=_my_cs();
	wrap10.pm_offset=(int)testInt10;
	wrap10.pm_selector=_my_cs();

	_go32_dpmi_allocate_iret_wrapper(&wrap2);
	_go32_dpmi_allocate_iret_wrapper(&wrap3);
	_go32_dpmi_allocate_iret_wrapper(&wrap5);
	_go32_dpmi_allocate_iret_wrapper(&wrap7);
	_go32_dpmi_allocate_iret_wrapper(&wrap10);

	new2.offset32=wrap2.pm_offset;
	new2.selector=wrap2.pm_selector;
	new3.offset32=wrap3.pm_offset;
	new3.selector=wrap3.pm_selector;
	new5.offset32=wrap5.pm_offset;
	new5.selector=wrap5.pm_selector;
	new7.offset32=wrap7.pm_offset;
	new7.selector=wrap7.pm_selector;
	new10.offset32=wrap10.pm_offset;
	new10.selector=wrap10.pm_selector;

	pic1Default=inportb(0x21);
	pic2Default=inportb(0xA1);

	outportb(0x21,pic1Default&0x53);		/* Clear all relevent masks */
	outportb(0xA1,pic2Default&0xFB);
	__dpmi_set_protected_mode_interrupt_vector(0x0A,&new2);
	__dpmi_set_protected_mode_interrupt_vector(0x0B,&new3);
	__dpmi_set_protected_mode_interrupt_vector(0x0D,&new5);
	__dpmi_set_protected_mode_interrupt_vector(0x0F,&new7);
	__dpmi_set_protected_mode_interrupt_vector(0x72,&new10);

	WriteDSP(0xF2);				/* This will force the DSP to signal */
	while(testCount==0);				/* a hardware interrupt, which one of */
								/* the functions I set up will handle. */
	outportb(0x21,pic1Default);
	outportb(0xA1,pic2Default);
	__dpmi_set_protected_mode_interrupt_vector(0x0A,&old2);
	__dpmi_set_protected_mode_interrupt_vector(0x0B,&old3);
	__dpmi_set_protected_mode_interrupt_vector(0x0D,&old5);
	__dpmi_set_protected_mode_interrupt_vector(0x0F,&old7);
	__dpmi_set_protected_mode_interrupt_vector(0x72,&old10);

	_go32_dpmi_free_iret_wrapper(&wrap2);
	_go32_dpmi_free_iret_wrapper(&wrap3);
	_go32_dpmi_free_iret_wrapper(&wrap5);
	_go32_dpmi_free_iret_wrapper(&wrap7);
	_go32_dpmi_free_iret_wrapper(&wrap10);

	return testCount;
} 

sb_status findSoundBlaster(void)
{
	static word ioaddr[7] = { 0x220, 0x240, 0x210, 0x230, 0x250, 0x260, 0x280 };
	char card[80]="Unknown";
	int a;
	sb_status stat=SB_FAILURE;

	for(a=0;a<7;a++)
	{
		Sound.base=ioaddr[a];
		Sound.reset=ioaddr[a]+0x06;
		Sound.readData=ioaddr[a]+0x0A;
		Sound.writeData=ioaddr[a]+0x0C;
		Sound.dataAvail=ioaddr[a]+0x0E;

		if(ResetDSP())
		{       					          /* Found the right IO address! */
			a=7;
			if((Sound.IRQ=findInterrupt()))
			{							/* ...grab the interrupt vector */
				if(!dontSetDMA) Sound.DMA=1;	/* Assume DMA channel 1, because there */
				stat=SB_SUCCESS;				/* is no reliable way to find it.  */
			}
		}
	}
	
	if (Sound.dspVersion>= 0x100) { strcpy (card, "Sound Blaster"); }
	if (Sound.dspVersion>= 0x105) { strcpy (card, "Sound Blaster 1.5"); }
	if (Sound.dspVersion>= 0x200) { strcpy (card, "Sound Blaster Pro 2"); }
	if (Sound.dspVersion>= 0x300) { strcpy (card, "Sound Blaster Pro 3"); }
	if ((Sound.dspVersion>> 8) >= 4) { strcpy (card, "Sound Blaster 16/ASP/AWE 32/AWE 64"); }
	printf("SB IRQ  is %d \n",Sound.IRQ);
	printf("SB DMA  is %d \n",Sound.DMA);
	printf("SB DSP  is %xh \n",Sound.dspVersion);
	printf("SB CARD is %s\n",card);
	return stat;
}

void sb_kickup()
{
	int x=Sound.IRQ;
	AllocateBuffer();
	//Save old/set new IRQ vector
	if (x == 2 || x == 10 || x == 11)
	{
		if (x == 2) setvect (0x71);
		if (x == 10) setvect (0x72);
		if (x == 11) setvect (0x73);
	}
	else
	{
		setvect(8 + x);
	}
	//Enable IRQ
	if (x == 2 || x == 10 || x == 11)
	{
		if (x == 2) outportb (0xA1, inportb (0xA1) & 253);
		if (x == 10) outportb (0xA1, inportb (0xA1) & 251);
		if (x == 11) outportb (0xA1, inportb (0xA1) & 247);
		outportb (0x21, inportb (0x21) & 251);
	}
	else
	{
		outportb (0x21, inportb (0x21) & !(1 << x));
	}
	printf("Starting DMA \n");
	sbvol(12);
	StartDMA();
}

void sb_uninstall()
{
	int x=Sound.IRQ;
	//Stops DMA-transfer
	WriteDSP (0xD0);
	WriteDSP (0xDA);

	//Free the memory allocated to the sound buffer
	free (DMABuffer);
	//Free interrupt vectors used to service IRQs
	if (x == 2 || x == 10 || x == 11)
	{
		if (x == 2) resetvect (0x71);
		if (x == 10) resetvect (0x72);
		if (x == 11) resetvect (0x73);
	}
	else
	{
		resetvect (8 + x);
	}

	//Mask IRQs
	if (x == 2 || x == 10 || x == 11)
	{
		if (x == 2) outportb (0xA1, inportb (0xA1) | 2);
		if (x == 10) outportb (0xA1, inportb (0xA1) | 4);
		if (x == 11) outportb (0xA1, inportb (0xA1) | 8);
		outportb (0x21, inportb (0x21) | 4);
	}
	else
	{
		outportb (0x21, inportb (0x21) | (1 << x));
	}

	printf("Bye Bye SB\n");
}

int sb_install()
{
	int x;
	if(findSoundBlaster()==SB_SUCCESS)
	{
		printf("SUCCESS: Sound Blaster Detected\n");
		x=SNDInit(22050,40);

		if(x==SB_SUCCESS)
		{
			sb_kickup();
			printf("SUCCESS: Sound frequency set\n");
		}
		else
		{
			printf("FAILURE: Frequency not set\n");
			return(SB_FAILURE);
		}
	}
	else
	{
		printf("FAILURE: Sound Blaster NOT detected\n");
		return(SB_FAILURE);
	}
	return(SB_SUCCESS);
}

//*******************Main Routines********************

void sb_reset()
{
	SNDreset();
}

int install_sid()
{
	if(sb_install()==SB_SUCCESS)
	{
		return(SB_SUCCESS);
	}
	else
	{
		return(SB_FAILURE);
	}
}

void uninstall_sid()
{
	sb_uninstall();
}

