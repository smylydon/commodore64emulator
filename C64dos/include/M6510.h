
// CPU Defines and Structures

#define	N65	128
#define	V65	64
#define	U65	32
#define	B65	16
#define	D65	8
#define	I65	4
#define	Z65	2
#define	C65	1

#define	NTSC		0
#if		NTSC
#define	CPUCYCLES	65
#define	CIACYCLES	65
#define	VICLINES	263
#define	PERIOD	1022727
#else
#define	CPUCYCLES	63
#define	CIACYCLES	63
#define	VICLINES	312
#define	PERIOD	985248
#endif

extern struct{
	dword	areg,xreg,
		yreg,psr,
		psp,ppc,
		ppc2,irq,
		nmi,clk,
		inst,cycles,
		RDY,BA,AEC,			// future use
		RWM,DEBUG;
}cpu;

// EXTERNALS FROM M6510.asm

extern void M6510(void);
extern void VIC2(void);
extern void CIA1(void);
extern void Reset6510(void);
extern void Reset6569(void);
extern void Reset6526(void);

extern byte mnspeek(dword);
extern byte mnsspeek(dword);
extern word mnsdeek(dword);

extern void mnspoke(dword,dword);
extern void mnsspoke(dword,dword);
extern void mnsdoke(dword,dword);

extern byte RAM[];
extern byte VIC2CHIP[];
extern byte COLORRAM[];
extern byte CIACHIP1[];
extern byte CIACHIP2[];
extern byte SIDCHIP[];
extern byte PixelRom[];
extern dword cputimer;
extern dword ciatimer;
extern dword victimer;
extern dword mnsvline;
extern byte myfont[];
extern byte invkeyarr[];
extern byte keyarr[];
