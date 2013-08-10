
#include <dpmi.h>
#include <go32.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <sys/nearptr.h>
#include <conio.h>

#define	TWO_BYTE_MODE_2	0x34	/* timer 0,2-byte read/write,mode 2, binary */
#define	TIMER_0_PORT	0x40	/* Timer 0 data port address */
#define	TIMER_MODE	0x43	/* Timer mode port address */
#define	BIOS_TICK_SEG	0x40	/* BIOS data segment */
#define	BIOS_TICK_OFF	0x6c	/* Address of BIOS (18.2/s) tick count */
#define	SCALE		10000	/* Scale factor for timer ticks */

/* The following assumes 18.2 BIOS ticks per second resulting from
   the 8253 being clocked at 1.19 MHz. */
#define	us_BTIK		54925	/* Micro sec per BIOS clock tick */
#define	f_BTIK		4595	/* Fractional part of usec per BIOS tick */
#define	us_TTIK		8381	/* Usec per timer tick * SCALE. (4/4.77 MHz) */	

#define LOCK_VARIABLE(x) _go32_dpmi_lock_data((void *)x,(long)sizeof(x));
_go32_dpmi_seginfo old_key_handler,new_key_handler;
_go32_dpmi_seginfo old_timer_handler,new_timer_handler;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

void timer_delete(void);
void timer_handler(void);
void timer_init(void);
dword elapsed_time(void);
void pause_time(byte mode);
void key_delete(void);
void key_init(void);
dword uclock_read(void);
void io_init(void);
void io_close(void);

dword time=0;
byte padding=0;
byte pausetime=0;
byte rawkey=0;
byte extflag=0;

/*---------------------------------------------------------*/
/* interrupt & system handlers                             */
/*---------------------------------------------------------*/
static void key_handler(void)
{
	byte al;
	byte scankey=0;
	__asm__("cli; pusha");
	
	scankey=inportb(0x60);
	al = inportb(0x61); 
	al |= 0x82;
	outportb(0x61, al);       
	al &= 0x7f;
	outportb(0x61, al);
	if(scankey==0xe0)
	{
		rawkey=0;
		extflag=0xff;
	}
	else
	{
		rawkey=scankey;
		extflag=0;
	}
	outportb(0x20, 0x20);
	__asm__("popa; sti");
}

void timer_handler()
{
	if(!pausetime) time++;
}

void key_init(void) /* function to swap state */
{
	new_key_handler.pm_offset   = (int)key_handler;
	new_key_handler.pm_selector = _go32_my_cs();
	_go32_dpmi_get_protected_mode_interrupt_vector(0x9, &old_key_handler);
	_go32_dpmi_allocate_iret_wrapper(&new_key_handler);
	_go32_dpmi_set_protected_mode_interrupt_vector(0x9,&new_key_handler);
	_go32_dpmi_lock_code(key_handler,(unsigned int) timer_handler-(unsigned int) key_handler);
//	_go32_dpmi_lock_data(keycolumn,sizeof(keycolumn));
//	_go32_dpmi_lock_data(keyrow,sizeof(keyrow));
}

void key_delete(void)
{
	_go32_dpmi_set_protected_mode_interrupt_vector(0x9,&old_key_handler);
	_go32_dpmi_free_iret_wrapper(&new_key_handler);
}

void timer_init(void)
{
	_go32_dpmi_get_protected_mode_interrupt_vector(0x8, &old_timer_handler);
	new_timer_handler.pm_offset=(int)timer_handler;
	new_timer_handler.pm_selector=_go32_my_cs();
	_go32_dpmi_chain_protected_mode_interrupt_vector(0x8,&new_timer_handler);
	_go32_dpmi_lock_code(timer_handler,(unsigned int) key_init-(unsigned int) timer_handler);
	_go32_dpmi_lock_data(&time,sizeof(time));
	
	asm volatile("cli");
	outportb(TIMER_MODE, TWO_BYTE_MODE_2);
	outportb(TIMER_0_PORT, 0);        /* Initial count = 65636 */
	outportb(TIMER_0_PORT, 0);
	asm volatile("sti");
}

void timer_delete(void)
{
	_go32_dpmi_set_protected_mode_interrupt_vector(0x8,&old_timer_handler);	
}

dword uclock_read(void)
{
	unsigned char msb, lsb;
	unsigned int tim_ticks, count;

	asm volatile("cli");
	outportb(TIMER_MODE, 0);                          /* Latch count.  */
	lsb = (unsigned char)inportb(TIMER_0_PORT);       /* Read count.   */
	msb = (unsigned char)inportb(TIMER_0_PORT);

	/* Get BIOS tick count */
	dosmemget(BIOS_TICK_SEG*16+BIOS_TICK_OFF,sizeof(unsigned int),&count);

	asm volatile("sti");

	/* Merge PIT channel 0 count with BIOS tick count */
	tim_ticks = (unsigned)(-1) - ((msb << 8) | lsb);

	return((count * us_BTIK)+((int)tim_ticks*us_TTIK+(count * us_BTIK)%SCALE)/SCALE);

}

dword elapsed_time(void)
{
	return(time);
}

void pause_time(byte mode)
{
	if(mode)
	{
		pausetime=0xff;
	}
	else
	{
		pausetime=0;
	}
}

void io_init(void)
{
	LOCK_VARIABLE(time);
	key_init();
	timer_init();	
}

void io_close(void)
{
	timer_delete();
	key_delete();
}