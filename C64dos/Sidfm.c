/* 

{ 9 voice FM SID !

 FM   Emulate

  0 - SID0 TRI/NSE
  1 - SID0 SAW
  2 - SID0 SQR
  3 - SID1 TRI/NSE
  4 - SID1 SAW
  5 - SID1 SQR
  6 - SID2 TRI/NSE
  7 - SID2 SAW
  8 - SID2 SQR

}
*/
	
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <dpmi.h>
#include <go32.h>
#include <inlines/pc.h>
#include <math.h>
#include <sys\farptr.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long qword;

#define AdlibFM	0x388

extern void WriteFM(dword reg, dword val);

void InitFM();
void KillFM();

void PokeFMSID(); // called by POKE.ASM set SIDAddr & SIDPoke 
void ResetFM(); // called by CBM64.ASM 

extern byte SIDCHIP[];
extern byte music_flag,music_on;

word Op[9]=
{
	/* i mod 3 + (i div 3) shl 3 */
	0x000,0x001,0x002,0x008,0x009,0x00A,0x010,0x011,0x012
};

byte Waves[]={0,3,1,0,3,1,0,3,1}; // TRI,SAW,SQR...

byte Percussive,MaxVol,Notes[8],State[8];
word Freqs[8],Pulse[8];
dword Voice2,SIDAddr,SIDPoke;

void wait(word ms)
{
	int temp;
	int z;
	if(ms==0) ms=3;
	z=ms/3;
	while(z)
	{
		temp=inportb(0x61);
		temp&=0xFD;
		temp|=0x01;
		outportb(0x61,temp);
		z--;
	}
}
	
byte ReadFM()
{
	return (inportb(AdlibFM));
}

void ResetFM()
{
	byte r,v,Oper,w;
	/* zero all registers */
	for(r=0x01;r<=0xF5;r++) WriteFM(r,0);

	/* allow FM chips to control the waveform of each operator */
	WriteFM(0x01,0x20);
	
	/* WriteFM($08,$80); { ??? */

	memset(Notes,0,sizeof(Notes));
	memset(State,0,sizeof(Notes));
	memset(Freqs,0,sizeof(Freqs));
	memset(Pulse,0,sizeof(Pulse));

	for(v=0;v<9;v++)
	{
		Oper=Op[v];
		w=Waves[v];
		WriteFM(0xE0+Oper,w);
		WriteFM(0xE3+Oper,w);
		WriteFM(0x20+Oper,0x21);
		WriteFM(0x23+Oper,0x21);
		WriteFM(0xC0+v,0x01);
	}
}

int InitFM()
{
	byte s1,s2,x;
	WriteFM(0x04,0x60);	/* reset both timers        */
	WriteFM(0x04,0x80);		/* enable timer interrupts  */
	s1=ReadFM();		/* read status register     */
	WriteFM(0x02,0xFF);
	WriteFM(0x04,0x21);		/* start timer 1            */
	wait(80);			/* could do something useful*/
	s2=ReadFM();		/* read status register     */
	WriteFM(0x04,0x60);	/* reset both timers        */
	WriteFM(0x04,0x80);	/* enable timer interrupts  */
	x=0xff;
	if ( ((s1 & 0xE0)!=0)||((s2 & 0xE0)!=0xC0) )
	{
		/* "#16' no AdlibFM (388h) found \n"	*/
		x=0xff;
	}
	else
	{
		/*	"#16' AdlibFM Found at 388h\n"	*/
		music_flag++;
		music_on++;
		x=0;
		ResetFM();
	}
	return x
}

void KillFM()
{
	int i;
	ResetFM();
	for(i=0;i<16;i++) WriteFM(0x40+i,63);	/* no sound */
	for(i=0;i<16;i++) WriteFM(0xB0+i,0);	/* no freq  */
}

void PlayVoice(word Voice)
{
	word Wave,Freq,Puls;
	byte Note,Oper;
	Wave=Waves[Voice];				/* Wave Form, SQR need a Pulse Volume ? */
	Puls=Pulse[Voice];				/* PulseWidth */
	Note=Notes[Voice];				/* Note On or Off ($20 or $00) */

	Oper=Op[Voice];

	if(Note>0)
	{
		if(Wave==1)
		{
			WriteFM(0x40+Oper,Puls);
		}
		else
		{
			WriteFM(0x40+Oper,MaxVol);
		}
		if(Wave==1)
		{
			WriteFM(0x43+Oper,Puls);
		}
		else
		{
			WriteFM(0x43+Oper,MaxVol);
		}
		Freq=Freqs[Voice];
		State[Voice]=Freq;
	}
	else
	{
		Freq=State[Voice];
		State[Voice]=0;
		if(Freq==0)	return;
	}

	WriteFM(0xA0+Voice,Freq&0xff);
	WriteFM(0xB0+Voice,(Freq>>8) | Note);

}

void VoiceFreq(word Voice, word Freq)
{
/* Adlib Freq from C64 Freq :

   c64   = F*17.0284
   Adlib = F/0.763

   Adlib = c64/13

*/
	byte oct;
	Freq=(Freq / 13);
	oct=4;
	/* Ajdust Octave */
	while(Freq>0x2AE)
	{
		/* C4 */
		oct++;
		Freq=Freq >> 1;
	}
	
	while( (oct>0)&&(Freq<0x16B) ) /* C#4 */
	{
		oct--;
		Freq=Freq << 1;
	}
	Freq=Freq | (oct << (2+8));

	Voice=(Voice<<1)+Voice;
	Freqs[Voice]=Freq;
	Freqs[Voice+1]=Freq;
	Freqs[Voice+2]=Freq;

	PlayVoice(Voice);
	PlayVoice(Voice+1);
	PlayVoice(Voice+2);
}

byte PulseMod[0x20]=
{
   0x00,0x01,0x02,0x03,0x04,0x04,0x05,0x05,
   0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,
   0x0A,0x0A,0x0B,0x0B,0x0C,0x0C,0x0D,0x0D,
   0x0E,0x0E,0x0F,0x0F,0x10,0x10,0x10,0x10
};

void VoicePulse(word Voice,word APulse)
{
	APulse=PulseMod[APulse >> 7];		/* ??? from PC64 */
	Voice=(Voice<<1)+Voice+2;			/* Only SQR */
	Pulse[Voice]=APulse;
	PlayVoice(Voice);
}

void VoiceControl(word Voice)
{
	word Wave;
//	word Back,Freq;

	Wave=SIDPoke >> 4;

	if( (Voice==2) && (!Voice2) ) Wave=9;	/* Mute 2 */
	if (Wave==0) Wave=9;				/* Invalide ?*/

	Voice=(Voice<<1)+Voice;
	Notes[Voice]  =0x00;
	Notes[Voice+1]=0x00;
	Notes[Voice+2]=0x00;

	if( (Wave<9) && (SIDPoke&0x01))
	{
		if(Wave==8)					/* Noise */
		{

    			WriteFM(0xC0+Voice,0x0E);
			Notes[Voice]=0x20;
		}
		else
		{

			WriteFM(0xC0+Voice,0x01);

/*			{ Wave:=1+2+4; { Try this ! } */

			if( (Wave & 1)>0 ) Notes[Voice]=0x20;
			if( (Wave & 2)>0 ) Notes[Voice+1]=0x20;
			if( (Wave & 4)>0 ) Notes[Voice+2]=0x20;

		}

	}

	PlayVoice(Voice);
	PlayVoice(Voice+1);
	PlayVoice(Voice+2);
}

byte Attack[0x10]=
{
  16*(15-4 ),16*(15-6 ),16*(15-7 ),16*(15-8),
  16*(15-8 ),16*(15-9 ),16*(15-9 ),16*(15-9),
  16*(15-10),16*(15-11),16*(15-12),16*(15-13),
  16*(15-13),16*(15-14),16*(15-14),16*(15-14)
};

byte Decay[0x10]=
{
  15-4 ,15-5 ,15-6 ,15-6,
  15-6 ,15-6 ,15-6 ,15-7,
  15-8 ,15-9 ,15-10,15-11,
  15-12,15-13,15-14,15-14
};

byte Sustain[0x10]=
{
  16*(15-0 ),16*(15-8 ),16*(15-10),16*(15-11),
  16*(15-12),16*(15-12),16*(15-13),16*(15-13),
  16*(15-13),16*(15-14),16*(15-14),16*(15-14),
  16*(15-14),16*(15-14),16*(15-15),16*(15-15)
};

byte Release[0x10]=
{
  15-0 ,15-1 ,15-2 ,15-3,
  15-4 ,15-5 ,15-6 ,15-7,
  15-8 ,15-9 ,15-10,15-11,
  15-12,15-13,15-14,15-14
};

void VoiceAD(word Voice)
{
	byte ATDC;
	word V,O;

	ATDC=Attack[SIDPoke >> 4] | Decay[SIDPoke & 0xf];

	Voice=(Voice<<1)+Voice;
	for(V=Voice;V<Voice+3;V++)
	{
		O=Op[V];
		WriteFM(0x60+O,ATDC);
		WriteFM(0x63+O,ATDC);
	}
}

void VoiceSR(word Voice)
{
	byte STRL;
	word V,O;
	STRL=Sustain[SIDPoke >> 4] | Release[SIDPoke & 0xf];
	Voice=(Voice<<1)+Voice;
	for(V=Voice;V<Voice+3;V++)
	{
		O=Op[V];
		WriteFM(0x80+O,STRL);
		WriteFM(0x83+O,STRL);
	}
}

void SetVolume()
{
//	byte v;
	Voice2=(SIDPoke & 0x80);
//	Voice2=0;
	MaxVol=(SIDPoke & 0xf);
	MaxVol=63-((MaxVol<<1)+32);
	PlayVoice(0);
	PlayVoice(1);
	PlayVoice(2);
	PlayVoice(3);
	
	PlayVoice(4);
	PlayVoice(5);
	PlayVoice(6);
	PlayVoice(7);
	
	PlayVoice(8);
}

word deeksid(byte sid)
{
	return (SIDCHIP[sid]|(SIDCHIP[sid+1]<<8));
}

void PokeFMSID()
{
	if(music_on)
	{
		switch(SIDAddr)
		{
			/*VOICE 0 */
			case 0:
			case 1:
				VoiceFreq(0,deeksid(0x00));
				break;
			case 2:
			case 3:
				VoicePulse(0,deeksid(0x02) & 0xFFF);
				break;

			case 4:
				VoiceControl(0);
				break;
			case 5:
				VoiceAD(0);
				break;
			case 6:
				VoiceSR(0);
				break;

			/* VOICE 1 */
			case 7:
			case 8:
				VoiceFreq(1,deeksid(07));
				break;
			case 9:
			case 10:
				VoicePulse(1,deeksid(0x09) & 0xFFF);
				break;

			case 11:
				VoiceControl(1);
				break;
			case 12:
				VoiceAD(1);
				break;
			case 13:
				VoiceSR(1);
				break;

			/*VOICE 2 */
			case 14:
			case 15:
				VoiceFreq(2,deeksid(0x0E));
				break;
			case 16:
			case 17:
				VoicePulse(2,deeksid(0x10) & 0xFFF);
				break;

			case 18:
				VoiceControl(2);
				break;
			case 19:
				VoiceAD(2);
				break;
			case 20:
				VoiceSR(2);
				break;

			/* GENERAL 
			   21,22: FilterFreq:=MemW[CIOSeg:$D400+21];
			   23   : SetFilter;
			*/	
			case 24   :
				SetVolume();
				break;
			default:
				break;
		}
	}
}