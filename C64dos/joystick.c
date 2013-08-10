#include <dpmi.h>
#include <dos.h>
#include <go32.h>
#include <pc.h>

	
#define JOYPORT		0x201   /* game port is at port 0x201 */
#define JOYFIREA		0x10    /* joystick 1, button A */
#define JOYFIREB		0x20    /* joystick 1, button B */
#define JOYXAXIS		0x01    /* joystick 1, x axis */
#define JOYYAXIS		0x02    /* joystick 1, y axis */
#define JOYUP		0x01
#define JOYDN		0x02
#define JOYLT		0x04
#define JOYRT		0x08
#define JOYFA		0x10
#define JOYFB		0x10
	
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

dword poll_joystick(void);
dword detect_joystick(void);	
	
dword joymaxx=0;
dword	joymaxy=0;
dword	joyminx=0;
dword	joyminy=0;
dword	joycx=0;
dword	joycy=0;
dword joystate=0;

dword poll_joystick(void)
{
	byte portvalue;
	dword z=0;
	joystate=0;
	joycx=joycy=0;

	portvalue=~inportb(0x201);
	if(portvalue&JOYFIREA) joystate|=JOYFA;
	if(portvalue&JOYFIREB) joystate|=JOYFA;
	__asm__("cli");
	outportb(JOYPORT,0xff);
	do{
		portvalue = inportb(JOYPORT);
		joycx+=portvalue & JOYXAXIS;
		joycy+=portvalue & JOYYAXIS;
	} while ((portvalue & 0x03) && (z++!=65536));
	__asm__("sti");
	if(joymaxx<joycx) joymaxx=joycx;
	if(joyminx>joycx) joyminx=joycx;
	if(joymaxy<joycy) joymaxy=joycy;
	if(joyminy>joycy) joyminy=joycy;
	joycx-=joyminx;
	joycy-=joyminy;
	if (joycx < ((joymaxx - joyminx)/3)) joystate|=JOYLT;
	if (joycx > (((joymaxx - joyminx)/3)*2)) joystate|=JOYRT;
	if (joycy < ((joymaxy - joyminy)/3)) joystate|=JOYUP;
	if (joycy > (((joymaxy - joyminy)/3)*2)) joystate|=JOYDN;
	return(joystate);
}

dword detect_joystick(void)
{
	byte portvalue;
	dword z=0;
	dword x=0;
	joystate=0;
	portvalue= inportb(0x201);
	__asm__("cli");
	outportb(JOYPORT,0xff);
	do{
		portvalue = inportb(JOYPORT);
		x=portvalue&JOYXAXIS;
	} while ((portvalue & 0x03) && (z++!=65536));
	__asm__("sti");
	x=(x!= 65535) ? 0xff: 0x00;
	joymaxx = joymaxy = 0;
	joyminx = joyminy = 65536;
	return x;
}
