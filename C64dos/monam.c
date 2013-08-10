#include "include/mytypes.h"
#include <dpmi.h>
#include <dos.h>
#include <go32.h>
#include <pc.h>
#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/farptr.h>
#include <sys/segments.h>
#include <sys/movedata.h>
#include <dir.h>
#include "include/M6510.h"

extern	void set_mode(byte);
extern	void set_palette(void);
extern	void key_init(void);
extern	void key_delete(void);
extern	byte rawkey;

dword AbsoluteModes(byte mnemonic,byte *op);
dword accimp(byte mnemonic);
void	assem(void);
void	badmn(void);
void	badad(void);
void	disa(dword count);
void	dumpvic(void);
void	dumpcia1(void);
void	get_args(char *x, char *arg1, char *arg2);
dword get_number(byte *x);
dword Immed(byte mnemonic, byte *op);
dword Indirectxy(byte mnemonic, byte *op);
dword indjmp(byte mnemonic, byte *op);
dword jmpjsr(byte mnemonic,byte *op);
void	monam(void);
void	printhelp(void);
void	printregs(void);
char	*read_line(char *prompt);
dword relative(byte mnemonic,byte *op);
void	run_x_inst(dword x);
byte	scan_adr(byte adr,byte mnemonic);

static char monamline[256];
dword nybbles=0;
dword ERROR=0;
dword mem;
enum {
  A_IMPL,
  A_ACCU,	// A
  A_IMM,	// #zz
  A_REL,	// Branches
  A_ZERO,	// zz
  A_ZEROX,	// zz,x
  A_ZEROY,	// zz,y
  A_ABS,	// zzzz
  A_ABSX,	// zzzz,x
  A_ABSY,	// zzzz,y
  A_IND,	// (zzzz)
  A_INDX,	// (zz,x)
  A_INDY	// (zz),y
};

// Mnemonics
static byte *mn[]=
{
	" adc "," and "," asl "," bcc "," bcs "," beq "," bit "," bmi "," bne "," bpl ",
	" brk "," bvc "," bvs "," clc "," cld "," cli "," clv "," cmp "," cpx "," cpy ",
	" dec "," dex "," dey "," eor "," inc "," inx "," iny "," jmp "," jsr "," lda ",
	" ldx "," ldy "," lsr "," nop "," ora "," pha "," php "," pla "," plp "," rol ",
	" ror "," rti "," rts "," sbc "," sec "," sed "," sei "," sta "," stx "," sty ",
	" tax "," tay "," tsx "," txa "," txs "," tya ",
	//undocumented mnemonics
	"*anc ","*ane ","*arr ","*asr ","*dcp ","*isb ","*jam ","*nop ","*las ","*lax ",
	"*lxa ","*rla ","*rra ","*sax ","*sbc ","*sbx ","*sha ","*shs ","*shx ","*shy ",
	"*slo ","*sre "
};

	
static byte *assmn[]=
{
	"adc","and","asl","bcc","bcs","beq","bit","bmi","bne","bpl",
	"brk","bvc","bvs","clc","cld","cli","clv","cmp","cpx","cpy",
	"dec","dex","dey","eor","inc","inx","iny","jmp","jsr","lda",
	"ldx","ldy","lsr","nop","ora","pha","php","pla","plp","rol",
	"ror","rti","rts","sbc","sec","sed","sei","sta","stx","sty",
	"tax","tay","tsx","txa","txs","tya",
	//undocumented mnemonics
	"anc","ane","arr","asr","dcp","isb","jam","nop","las","lax",
	"lxa","rla","rra","sax","sbc","sbx","sha","shs","shx","shy",
	"slo","sre"
};

enum {
	M_ADC=0, M_AND, M_ASL, M_BCC, M_BCS, M_BEQ, M_BIT, M_BMI, M_BNE, M_BPL,
	M_BRK, M_BVC, M_BVS, M_CLC, M_CLD, M_CLI, M_CLV, M_CMP, M_CPX, M_CPY,
	M_DEC, M_DEX, M_DEY, M_EOR, M_INC, M_INX, M_INY, M_JMP, M_JSR, M_LDA,
	M_LDX, M_LDY, M_LSR, M_NOP, M_ORA, M_PHA, M_PHP, M_PLA, M_PLP, M_ROL,
	M_ROR, M_RTI, M_RTS, M_SBC, M_SEC, M_SED, M_SEI, M_STA, M_STX, M_STY,
	M_TAX, M_TAY, M_TSX, M_TXA, M_TXS, M_TYA,

  //undocumented opcodes

	M_IANC, M_IANE, M_IARR, M_IASR, M_IDCP, M_IISB, M_IJAM, M_INOP, M_ILAS,
	M_ILAX, M_ILXA, M_IRLA, M_IRRA, M_ISAX, M_ISBC, M_ISBX, M_ISHA, M_ISHS,
	M_ISHX, M_ISHY, M_ISLO, M_ISRE
};

//lookup
const byte decode[256] = {
  M_BRK , M_ORA , M_IJAM, M_ISLO, M_INOP, M_ORA, M_ASL , M_ISLO,	// 00
  M_PHP , M_ORA , M_ASL , M_IANC, M_INOP, M_ORA, M_ASL , M_ISLO,
  M_BPL , M_ORA , M_IJAM, M_ISLO, M_INOP, M_ORA, M_ASL , M_ISLO,	// 10
  M_CLC , M_ORA , M_INOP, M_ISLO, M_INOP, M_ORA, M_ASL , M_ISLO,
  M_JSR , M_AND , M_IJAM, M_IRLA, M_BIT , M_AND, M_ROL , M_IRLA,	// 20
  M_PLP , M_AND , M_ROL , M_IANC, M_BIT , M_AND, M_ROL , M_IRLA,
  M_BMI , M_AND , M_IJAM, M_IRLA, M_INOP, M_AND, M_ROL , M_IRLA,	// 30
  M_SEC , M_AND , M_INOP, M_IRLA, M_INOP, M_AND, M_ROL , M_IRLA,
  M_RTI , M_EOR , M_IJAM, M_ISRE, M_INOP, M_EOR, M_LSR , M_ISRE,	// 40
  M_PHA , M_EOR , M_LSR , M_IASR, M_JMP , M_EOR, M_LSR , M_ISRE,
  M_BVC , M_EOR , M_IJAM, M_ISRE, M_INOP, M_EOR, M_LSR , M_ISRE,	// 50
  M_CLI , M_EOR , M_INOP, M_ISRE, M_INOP, M_EOR, M_LSR , M_ISRE,
  M_RTS , M_ADC , M_IJAM, M_IRRA, M_INOP, M_ADC, M_ROR , M_IRRA,	// 60
  M_PLA , M_ADC , M_ROR , M_IARR, M_JMP , M_ADC, M_ROR , M_IRRA,
  M_BVS , M_ADC , M_IJAM, M_IRRA, M_INOP, M_ADC, M_ROR , M_IRRA,	// 70
  M_SEI , M_ADC , M_INOP, M_IRRA, M_INOP, M_ADC, M_ROR , M_IRRA,
  M_INOP, M_STA , M_INOP, M_ISAX, M_STY , M_STA, M_STX , M_ISAX,	// 80
  M_DEY , M_INOP, M_TXA , M_IANE, M_STY , M_STA, M_STX , M_ISAX,
  M_BCC , M_STA , M_IJAM, M_ISHA, M_STY , M_STA, M_STX , M_ISAX,	// 90
  M_TYA , M_STA , M_TXS , M_ISHS, M_ISHY, M_STA, M_ISHX, M_ISHA,
  M_LDY , M_LDA , M_LDX , M_ILAX, M_LDY , M_LDA, M_LDX , M_ILAX,	// a0
  M_TAY , M_LDA , M_TAX , M_ILXA, M_LDY , M_LDA, M_LDX , M_ILAX,
  M_BCS , M_LDA , M_IJAM, M_ILAX, M_LDY , M_LDA, M_LDX , M_ILAX,	// b0
  M_CLV , M_LDA , M_TSX , M_ILAS, M_LDY , M_LDA, M_LDX , M_ILAX,
  M_CPY , M_CMP , M_INOP, M_IDCP, M_CPY , M_CMP, M_DEC , M_IDCP,	// c0
  M_INY , M_CMP , M_DEX , M_ISBX, M_CPY , M_CMP, M_DEC , M_IDCP,
  M_BNE , M_CMP , M_IJAM, M_IDCP, M_INOP, M_CMP, M_DEC , M_IDCP,	// d0
  M_CLD , M_CMP , M_INOP, M_IDCP, M_INOP, M_CMP, M_DEC , M_IDCP,
  M_CPX , M_SBC , M_INOP, M_IISB, M_CPX , M_SBC, M_INC , M_IISB,	// e0
  M_INX , M_SBC , M_NOP , M_ISBC, M_CPX , M_SBC, M_INC , M_IISB,
  M_BEQ , M_SBC , M_IJAM, M_IISB, M_INOP, M_SBC, M_INC , M_IISB,	// f0
  M_SED , M_SBC , M_INOP, M_IISB, M_INOP, M_SBC, M_INC , M_IISB
};

// adr lookup
const byte adrmode[256] = {
  A_IMPL, A_INDX, A_IMPL, A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO,	// 00
  A_IMPL, A_IMM , A_ACCU, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
  A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX,	// 10
  A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
  A_ABS , A_INDX, A_IMPL, A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO,	// 20
  A_IMPL, A_IMM , A_ACCU, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
  A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX,	// 30
  A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
  A_IMPL, A_INDX, A_IMPL, A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO,	// 40
  A_IMPL, A_IMM , A_ACCU, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
  A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX,	// 50
  A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
  A_IMPL, A_INDX, A_IMPL, A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO,	// 60
  A_IMPL, A_IMM , A_ACCU, A_IMM , A_IND  , A_ABS  , A_ABS  , A_ABS,
  A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX,	// 70
  A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
  A_IMM , A_INDX, A_IMM , A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO,	// 80
  A_IMPL, A_IMM , A_IMPL, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
  A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROY, A_ZEROY,	// 90
  A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSY , A_ABSY,
  A_IMM , A_INDX, A_IMM , A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO,	// a0
  A_IMPL, A_IMM , A_IMPL, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
  A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROY, A_ZEROY,	// b0
  A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSY , A_ABSY,
  A_IMM , A_INDX, A_IMM , A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO,	// c0
  A_IMPL, A_IMM , A_IMPL, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
  A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX,	// d0
  A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
  A_IMM , A_INDX, A_IMM , A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO,	// e0
  A_IMPL, A_IMM , A_IMPL, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
  A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX,	// f0
  A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX
};

void badmn()
{
	printf("**** ERROR - Unknown Mnemonic\n");
	printf("**** type h or ? for help\n\n");
}

void badad()
{
	printf("**** ERROR - Illegal addressing mode\n");
	printf("**** type h or ? for help\n\n");
}

void printregs()
{
	printf("\nAR=%2.2x XR=%2.2x YR=%2.2x ",cpu.areg,cpu.xreg,cpu.yreg);
	printf("SP=%2.2x ",cpu.psp);
	printf("PC=%4.4x PC2=%4.4x\n\n",cpu.ppc,cpu.ppc2);
	printf("N V - B D I Z C\n");
	printf("%1.1d ",(cpu.psr&0x80)>>7);
	printf("%1.1d ",(cpu.psr&0x40)>>6);
	printf("%1.1d ",(cpu.psr&0x20)>>5);
	printf("%1.1d ",(cpu.psr&0x10)>>4);
	printf("%1.1d ",(cpu.psr&0x08)>>3);
	printf("%1.1d ",(cpu.psr&0x04)>>2);
	printf("%1.1d ",(cpu.psr&0x02)>>1);
	printf("%1.1d \n\n",cpu.psr&0x01);
	printf("IRQ : %4.4x\n\n",(mnspeek(0xffff)<<8)+mnspeek(0xfffe));
}

void printhelp()
{
	printf("\n*********** MNS MonAm main help ***********\n\n");
	printf("a [addr] - assember \n");
	printf("c        - display CIA status\n");
	printf("d [addr] - disassembler\n");
	printf("l [addr] - set program count to [addr]\n");
	printf("p        - view registers\n");
	printf("r [inst] - run x instructions\n");
	printf("s        - stepover byte\n\n");
	printf("q        - quit monam\n");
	printf("v        - display VIC2 status\n");
	printf("h or ?   - help\n");
	printf("format   - [command] [option]\n\n");
}

void run_x_inst(dword x)
{
	int count=0;
	while(count<x)
	{
		M6510();
		count++;
	}
}

void dumpvic()
{
	dword rc;
	dword rr;
	printf("\nDisplay Mode   :");
	rr=VIC2CHIP[0x11]&0xff;
	rc=VIC2CHIP[0x16]&0xff;
	rc=((rc&0x10)|(rr&0x60))>>4;
	// ECM4 BMM2 MC
	if(rr&0x10)
	{
		switch(rc)
		{
			case 0x00:
				printf(" Standard Text ");
				break;
			case 0x01:
				printf(" MultiColor Text ");
				break;
			case 0x02:
				printf(" Standard BitMap ");
					break;
			case 0x03:
				printf(" MultiColor BitMap ");
				break;
			case 0x04:
				printf(" Extended Text ");
				break;
			case 0x05:
				printf(" Invalid Text Mode ");
				break;
			case 0x06:
				printf(" Invalid BitMap Mode One ");
				break;
			case 0x07:
				printf(" Invalid BitMap Mode Two ");
				break;
		}
	}
	else
	{
		printf(" Disabled Off ");
	}
	printf("\n");
	rc=(dword)(VIC2CHIP[0x12]|((VIC2CHIP[0x11]>>7)<<8));
	rr=mnspeek(0xd011)&0x80;
	rr=((rr)*256)|(mnspeek(0xd012)&0xff);
	printf("Raster Compare : %4.4x \n",rc);
	printf("Raster Count   : %4.4x \n\n",rr);
	rr=(dword) VIC2CHIP[0x18]&0xff;
	rc=(dword) ((CIACHIP2[00]&0x3)^0x3);
	printf("Video bank     :");
	if(rc==0) printf(" 0000-3fff\n");
	if(rc==1) printf(" 4000-7fff\n");
	if(rc==2) printf(" 8000-bfff\n");
	if(rc==3) printf(" c000-ffff\n");
	rc<<=14;
	printf("Video Matrix   : %4.4x\n",((rr&0xf0)<<0x6)+rc);
	printf("Character bank : %4.4x\n",((rr&0xe)<<10)+rc);
	printf("BitMap Matrix  : %4.4x\n",((rr&0x8)<<10)+rc);
	printf("\n");

	rc=(dword) VIC2CHIP[0x19]&0x8f;
	rr=(dword) VIC2CHIP[0x1a]&0xf;
	if(rr)
	{
		printf ("Interupts Enabled :");
		if(rr&1) printf(" Raster ");
		if(rr&2) printf(" Sprite2Gfx ");
		if(rr&4) printf(" Sprite2Sprite  ");
		if(rr&8) printf(" LitePen ");
		printf("\n");
	}
	else
	{
		printf("Interupts Enabled : None\n");
	}
	
 	if(rc)
	{
		printf("Interupts Pending :");
		if(rc&1) printf(" Raster ");
		if(rc&2) printf(" Sprite2Gfx ");
		if(rc&4) printf(" Sprite2Sprite  ");
		if(rc&8) printf(" LitePen ");
		printf("\n");
	}
	else
	{
		printf("Interupts Pending : None\n");
	}
	printf("\n");
 } 

void dumpcia1()
{
	byte temp;
	temp=CIACHIP1[0xe];
	printf("\n");
	if(temp&1)
	{
		printf("Timer A : ON\n");
	}
	else
	{
		printf("Timer A : OFF\n");
	}
	printf(" latch  : %4.4x",(CIACHIP1[33]<<8)+CIACHIP1[32]);
	printf(" count  : %4.4x\n",(CIACHIP1[0x15]<<8)+CIACHIP1[0x14]);
	printf("\n");
	temp=CIACHIP1[0xf];
	if(temp&1)
	{
		printf("Timer B : ON\n");
	}
	else
	{
		printf("Timer B : OFF\n");
	}
	printf(" latch  : %4.4x",(CIACHIP1[41]<<8)+CIACHIP1[40]);
	printf(" count  : %4.4x\n",(CIACHIP1[0x17]<<8)+CIACHIP1[0x16]);

	printf("interrupts\n");
	temp=CIACHIP1[0x0d];
	if(temp&0x1f)
	{
		printf(" Pending :");
		if(temp&1) printf(" TA");
		if(temp&2) printf(" TB");
		printf("\n");
	}
	else
	{
		printf(" Pending : None\n");
	}
	temp=CIACHIP1[0x1d];
	if(temp&0x1f)
	{
		printf(" Enabled :");
		if(temp&1) printf(" TA");
		if(temp&2) printf(" TB");
		printf("\n");
	}
	else
	{
		printf(" Enabled : None\n");
	}
	printf("\n");
	
	temp=CIACHIP2[0xe];
	printf("\n");
	if(temp&1)
	{
		printf("Timer A : ON\n");
	}
	else
	{
		printf("Timer A : OFF\n");
	}
	printf(" latch  : %4.4x",(CIACHIP2[33]<<8)+CIACHIP2[32]);
	printf(" count  : %4.4x\n",(CIACHIP2[0x15]<<8)+CIACHIP2[0x14]);
	printf("\n");
	temp=CIACHIP2[0xf];
	if(temp&1)
	{
		printf("Timer B : ON\n");
	}
	else
	{
		printf("Timer B : OFF\n");
	}
	printf(" latch  : %4.4x",(CIACHIP2[41]<<8)+CIACHIP2[40]);
	printf(" count  : %4.4x\n\n",(CIACHIP2[0x17]<<8)+CIACHIP2[0x16]);
	printf("interrupts\n");
	temp=CIACHIP2[0x0d];
	if(temp&0x1f)
	{
		printf(" Pending :");
		if(temp&1) printf(" TA");
		if(temp&2) printf(" TB");
		printf("\n");
	}
	else
	{
		printf(" Pending : None\n");
	}
	temp=CIACHIP2[0x1d];
	if(temp&0x1f)
	{
		printf(" Enabled :");
		if(temp&1) printf(" TA");
		if(temp&2) printf(" TB");
		printf("\n");
	}
	else
	{
		printf(" Enabled : None\n");
	}
	printf("\n");
} 

char *read_line(char *prompt)
{
	char *linep, *r_linep;
	fputs(prompt, stdout);
	fflush(stdout);
	r_linep = linep = fgets(monamline, 255, stdin);
	while (r_linep && *r_linep && isspace(*r_linep))
	r_linep++;
	return (r_linep);
}

void get_args(char *x, char *arg1, char *arg2)
{
	while (x && *x && (!isspace(*x)))
	{
		*arg1++=*x++;
	}
	*arg1=NULL;
	while (x && *x && isspace(*x))
	x++;
	while (x && *x && (!isspace(*x)))
	{
		*arg2++=*x++;
	}
	*arg2=NULL;
}

void disa(dword count)
{
	int temp;
	byte opnd1,opnd2,op;
	byte c=0;
	while((c<count)&&(c<=15))
	{
		temp=0;
		mem=(cpu.ppc2&0xffff);
		op=mnspeek(mem);
		opnd1=mnspeek(mem+1);
		opnd2=mnspeek(mem+2);
		printf("$%4.4x  ",mem);
		switch(adrmode[op])
		{
			case A_ACCU:
				printf("%2.2x  ",op);
				printf("        ");
				printf("%s a\n",mn[decode[op]]);
				cpu.ppc2++;
				break;
			case A_IMPL:
				printf("%2.2x  ",op);
				printf("        ");
				printf("%s\n",mn[decode[op]]);
				cpu.ppc2++;
				break;
			case A_REL:
				if(opnd1<0x80)
				{
					temp=((opnd1+mem-1)&0x00ff)|(mem&0xff00);
				}
				else
				{
					temp=((1+mem-(0xff-opnd1))&0x00ff)|(mem&0xff00);
				}
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("    ");
				printf("%s ",mn[decode[op]]);
				printf("$%2.2x\n",temp);
				cpu.ppc2+=2;
				break;
			case A_IMM:
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("    ");
				printf("%s ",mn[decode[op]]);
				printf("#$%2.2x\n",opnd1);
				cpu.ppc2+=2;
				break;
			case A_ZERO:
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("    ");
				printf("%s ",mn[decode[op]]);
				printf("$%2.2x\n",opnd1);
				cpu.ppc2+=2;
				break;	
			case A_ZEROX:
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("    ");
				printf("%s ",mn[decode[op]]);
				printf("$%2.2x,x\n",opnd1);
				cpu.ppc2+=2;
				break;
			case A_ZEROY:
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("    ");
				printf("%s ",mn[decode[op]]);
				printf("$%2.2x,y\n",opnd1);
				cpu.ppc2+=2;
				break;
			case A_INDX:
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("    ");
				printf("%s (",mn[decode[op]]);
				printf("$%2.2x,x)\n",opnd1);
				cpu.ppc2+=2;
				break;
			case A_INDY:
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("    ");
				printf("%s (",mn[decode[op]]);
				printf("$%2.2x),y\n",opnd1);
				cpu.ppc2+=2;
				break;
			case A_ABS:
				temp=(opnd1+(opnd2<<8))&0xffff;
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("%2.2x  ",opnd2);
				printf("%s ",mn[decode[op]]);
				printf("$%4.4x\n",temp);
				cpu.ppc2+=3;
				break;
			case A_ABSX:
				temp=(opnd1+(opnd2<<8))&0xffff;
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("%2.2x  ",opnd2);
				printf("%s ",mn[decode[op]]);
				printf("$%4.4x,x\n",temp);
				cpu.ppc2+=3;
				break;
			case A_ABSY:
				temp=(opnd1+(opnd2<<8))&0xffff;
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("%2.2x  ",opnd2);
				printf("%s ",mn[decode[op]]);
				printf("$%4.4x,y\n",temp);
				cpu.ppc2+=3;
				break;
			case A_IND:
				temp=(opnd1+(opnd2<<8))&0xffff;
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("%2.2x  ",opnd2);
				printf("%s (",mn[decode[op]]);
				printf("$%4.4x)\n",temp);
				cpu.ppc2+=3;
				break;
			default:
				printf("%2.2x  ",op);
				printf("%2.2x  ",opnd1);
				printf("%2.2x  ",opnd2);
				break;
		}
		c++;
	}
}

inline dword get_number(byte *x)
{
	dword i=0;
	dword c=0;
	while((((*x>='0')&&(*x<='9'))||((*x>='a')&&(*x<='f')))&&x)
	{
		if(*x<'a')
		{
			i=(i<<4)+((*x)-'0');
		}
		else
		{
			i=(i<<4)+((*x)-'a'+10);
		}
		c++;
		x++;
	}
	nybbles=c;
	if((c==0)||(c>=5)) ERROR=0xff;
	return (i);
}


inline byte scan_adr(byte adr,byte mnemonic)
{
	int x=0;
	while(x<=255)
	{
		if(decode[x]==mnemonic)
		{
			if(adrmode[x]==adr)
			{
				ERROR=0;
				return((byte)x);
				break;
			}
		}
		x++;
	}
	ERROR=0xff;
	return((byte) x);
}

dword accimp(byte mnemonic)
{
	int x=0;
	byte adr;
	int address=cpu.ppc2;
	ERROR=0xff;
	while(x<=255)
	{
		adr=adrmode[x];
		if(mnemonic==decode[x])
		{
			if((adr==A_IMPL)||(adr==A_ACCU))
			{
				ERROR=0;
				break;
			}
		}
		x++;
	}
	if(ERROR==0)
	{
		mnspoke(address,x);
		disa(1);
		return(0);
	}
	badad();
	return(0x10);
}

dword Immed(byte mnemonic, byte *op)
{
	dword s=0;
	byte adr=A_IMM;
	mnemonic=scan_adr(adr,mnemonic);
	if(ERROR==0)
	{
		s=get_number(op);
		if((ERROR==0)&&(*(op+1)==NULL)&&(nybbles==1))
		{
			mnspoke(cpu.ppc2,(dword)mnemonic);
			mnspoke(cpu.ppc2+1,s);
			disa(1);
			return(0);
		}
		if((ERROR==0)&&(*(op+2)==NULL)&&(nybbles==2))
		{
			mnspoke(cpu.ppc2,(dword)mnemonic);
			mnspoke(cpu.ppc2+1,s);
			disa(1);
			return(0);
		}
	}
	badad();
	return(10);
}

dword relative(byte mnemonic,byte *op)
{
	int s=0;
	int address=cpu.ppc2;
	byte adr=A_REL;
	mnemonic=scan_adr(adr,mnemonic);
	if(ERROR==0)
	{
		s=get_number(op)&0xffff;
		if(ERROR==0)
		{
			if(nybbles>=3)
			{
				if(s>=address)
				{
					s=(s+1-(address))&0xff;
				}
				else
				{
					s=0xff-((address+1-s)&0xff);
				}
				mnspoke(address,(int)mnemonic);
				mnspoke(address+1,s);
				disa(1);
				return(0);
			}
			if(nybbles<=2)
			{
				mnspoke(address,(int)mnemonic);
				mnspoke(address+1,s);
				disa(1);
				return(0);
			}
		}
	}
	badad();
	return(10);
}

dword indjmp(byte mnemonic, byte *op)
{
	int s=0;
	int address=cpu.ppc2;
	s=get_number(op);

	if((ERROR==0)&&(nybbles<=4))
	{
		op+=nybbles;
		if((*op==')')&&(*(op+1)==NULL))
		{
			mnemonic=scan_adr(A_IND,mnemonic);
			if(ERROR==0)
			{
				mnspoke(address,(int)mnemonic);
				mnspoke(address+1,s&0xff);
				mnspoke(address+2,(s>>8)&0xff);
				disa(1);
				return(0);
			}
			badad();
			return(10);
		}
		printf("missing ) or rubbish after )\n");
		return(10);
	}
	badad();
	return(10);
}

dword jmpjsr(byte mnemonic,byte *op)
{
	int s=0;
	int address=cpu.ppc2;
	s=get_number(op);
	if((ERROR==0)&&(nybbles<=4))
	{
		op+=nybbles;
		if(*op==NULL)
		{
			mnemonic=scan_adr(A_ABS,mnemonic);
			if(ERROR==0)
			{
				mnspoke(address,(int)mnemonic);
				mnspoke(address+1,s&0xff);
				mnspoke(address+2,(s>>8)&0xff);
				disa(1);
				return(0);
			}
			badad();
			return(10);
		}
		printf("rubbish after destination\n");
		return(10);
	}
	badad();
	return(10);
}

dword Indirectxy(byte mnemonic, byte *op)
{
	int s=0;
	int address=cpu.ppc2;
	s=get_number(op);
	if((nybbles<=2)&&(ERROR==0))
	{
		op+=nybbles;
		if((*op==',')&&(*(op+1)=='x'))
		{
			op+=2;
			if((*op==')')&&(*(op+1)==NULL))
			{
				mnemonic=scan_adr(A_INDX,mnemonic);
				if(ERROR==0)
				{
					mnspoke(address,(int)mnemonic);
					mnspoke(address+1,s);
					disa(1);
					return(0);
				}
			}
			badad();
			return(10);
		}
		
		if((*op==')')&&(*(op+1)==','))
		{
			op+=2;
			if((*op=='y')&&(*(op+1)==NULL))
			{
				mnemonic=scan_adr(A_INDY,mnemonic);
				if(ERROR==0)
				{
					mnspoke(address,(int)mnemonic);
					mnspoke(address+1,s);
					disa(1);
					return(0);
				}
			}
			badad();
			return(10);
		}	
	}
	badad();
	return(10);
}

dword AbsoluteModes(byte mnemonic,byte *op)
{
	int s=0;
	int address=cpu.ppc2;
	s=get_number(op);
	if((ERROR==0)&&(nybbles<=4))
	{
		op+=nybbles;
		if(*op==NULL)
		{
			if(nybbles<=2)
			{
				mnemonic=scan_adr(A_ZERO,mnemonic);
				if(ERROR==0)
				{
					mnspoke(address,(int)mnemonic);
					mnspoke(address+1,s);
					disa(1);
					return(0);
				}
			}
			else
			{
				mnemonic=scan_adr(A_ABS,mnemonic);
				if(ERROR==0)
				{
					mnspoke(address,(int)mnemonic);
					mnspoke(address+1,s&0xff);
					mnspoke(address+2,(s>>8)&0xff);
					disa(1);
					return(0);
				}
			}
		}
		
		if(*op==',')
		{
			op++;
			if(nybbles<=2)
			{
				if(((*op!='x')||(*op!='y'))&&(*(op+1)!=NULL))
				{
					badad();
					return(10);
				}
				if(*op=='x')
				{
					mnemonic=scan_adr(A_ZEROX,mnemonic);
				}
				else
				{
					mnemonic=scan_adr(A_ZEROY,mnemonic);
				}
				if(ERROR==0)
				{
					mnspoke(address,(int)mnemonic);
					mnspoke(address+1,s);
					disa(1);
					return(0);
				}
				badad();
				return(10);
			}
			else
			{
				if(((*op!='x')||(*op!='y'))&&(*(op+1)!=NULL))
				{
					badad();
					return(0);
				}
				if(*op=='x')
				{
					mnemonic=scan_adr(A_ABSX,mnemonic);
				}
				else
				{
					mnemonic=scan_adr(A_ABSY,mnemonic);
				}
				if(ERROR==0)
				{
					mnspoke(address,(int)mnemonic);
					mnspoke(address+1,s&0xff);
					mnspoke(address+2,(s>>8)&0xff);
					disa(1);
					return(0);
				}
				badad();
				return(10);
			}
		}
	}
	badad();
	return(10);
}

void assem()
{
	char *l,*cmd,*par,prompt[10],arg1[256],arg2[256];
	int quit=0;
	byte mnemonic=0;
	int error=0;
	cmd=arg1;
	par=arg2;
	printf("\n***********      MNS Ass v1     ***********\n\n");
	while(quit==0)
	{
		ERROR=0xff;
		itoa((cpu.ppc2)&0xffff,prompt,16);
		strcat(prompt," > ");
		l=read_line(prompt);
		get_args(l,arg1,arg2);
		strlwr(arg1);
		strlwr(arg2);
		arg1[3]=NULL;
		if(*cmd=='q')
		{
			quit=0xff;
			printf("\nAdios Amigos\n\n");
		}
		else
		{
			mnemonic=0;
			while(mnemonic<=M_ISRE)
			{
				if(strcmp(arg1,assmn[mnemonic])==0)
				{
					ERROR=0;
					break;
				}
				mnemonic++;
			}
			
			if(ERROR==0)
			{
				if(arg2[0]==NULL)
				{
					error=accimp(mnemonic);
				}
				else
				{
					switch(arg2[0])
					{
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
						case 'a':
						case 'b':
						case 'c':
						case 'd':
						case 'e':
						case 'f':
							switch(arg1[0])
							{
								case 'b':
									error=relative(mnemonic,arg2);
									break;
								case 'j':
									error=jmpjsr(mnemonic,arg2);
									break;
								default:
									error=AbsoluteModes(mnemonic,arg2);
									break;
							}
							break;
						case '$':
							switch(arg1[0])
							{
								case 'b':
									error=relative(mnemonic,arg2+1);
									break;
								case 'j':
									error=jmpjsr(mnemonic,arg2+1);
									break;
								default:
									error=AbsoluteModes(mnemonic,arg2+1);
									break;
							}
							break;
						case '#':
							if(arg2[1]=='$')
							{
								error=Immed(mnemonic,arg2+2);
							}
							else
							{
								if(arg2[1]!=NULL)
								{
									error=Immed(mnemonic,arg2+1);
								}
								else
								{
									badad();
								}
							}
							break;
						case '(':
							if(arg2[1]=='$')
							{
								if(arg1[0]=='j')
								{
									error=indjmp(mnemonic,arg2+2);
								}
								else
								{
									error=Indirectxy(mnemonic,arg2+2);
								}
							}
							else
							{
								if(arg1[0]=='j')
								{
									error=indjmp(mnemonic,arg2+1);
								}
								else
								{
									error=Indirectxy(mnemonic,arg2+1);
								}
							}
							break;
						default :
							badmn();
							break;
					}
				}
			}
			else
			{
				badmn();
			}
		}
	}
}

void monam()
{
	int quit,x=1;
	char *l,*cmd,*par,prompt[10],arg1[256],arg2[256];
	quit=0;
	cmd=arg1;
	par=arg2;
	cpu.ppc2&=0xffff;
	cpu.ppc&=0xffff;
	key_delete();
	_set_screen_lines(40);
	printf("\n***********  MNS MonAm v1 (c)97/98 ***********\n\n");
	printregs();
	strcpy(prompt,">");
	while(quit!=1)
	{
		l=read_line(prompt);
		get_args(l,arg1,arg2);
		strlwr(arg1);
		strlwr(arg2);
		switch(*cmd)
		{
			case 'a':
				if(*par!=NULL)
				{
					cpu.ppc2=strtol(par,&l,16)&0xffff;
				}
				assem();
				break;
			case 'c' :
				dumpcia1();
				break;
			case 'd':
				printf("\n***********      MNS Disa v1    ***********\n\n");
				if(*par!=NULL)
				{
					cpu.ppc2=strtol(par,&l,16)&0xffff;
				}
				printregs();
				disa(20);
				break;
			case 'h' :
			case '?' :
				printhelp();
				break;
			case 'l' :
				if(*par!=NULL)
				{
					cpu.ppc2=strtol(par,&l,16)&0xffff;
				}
				cpu.ppc=cpu.ppc2;
				break;
			case 'p' :
				printregs();
				break;
			case 'q' :
				quit=1;
				break;
			case 'r' :
				if(*par!=NULL)
				{
					x=strtol(par,&l,16);
				}
				run_x_inst(x&0xff);
				printregs();
				disa(x);
				break;
			case 's' :
				cpu.ppc2=(cpu.ppc+=1)&0xffff;
				cpu.cycles=5;
				M6510();
				printregs();
				disa(20);
				break;
			case 'v' :
				dumpvic();
				break;
			default :
				printf("ERROR - Unknown command\n");
				printf("        type h or ? for help\n\n");
				break;
		}
	}
	rawkey=0;
//	set_mode(0x13);
//	set_palette();
	key_init();
}
