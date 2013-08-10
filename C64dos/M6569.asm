;===================================================================================
; VIC 2 Emulation begins here
;-----------------------------------------------------------------------------------
VIC2base		equ	0x0d000
	
sprite0x		equ	64		;00	sprite 0 x cord
sprite0y		equ	96		;01	sprite 0 y cord
sprite1x		equ	68		;02
sprite1y		equ	100		;03
sprite2x		equ	72		;04
sprite2y		equ	104		;05
sprite3x		equ	76		;06
sprite3y		equ	108		;07
sprite4x		equ	80		;08
sprite4y		equ	112		;09
sprite5x		equ	84		;0a
sprite5y		equ	116		;0b
sprite6x		equ	88		;0c
sprite6y		equ	120		;0d
sprite7x		equ	92		;0e	sprite 7 x cord
sprite7y		equ	124		;0f	sprite 7 y cord

spritesx8		equ	16		;10	bit 8 of sprites x cords
viccontreg1		equ	17		;11	vic control register 1 write latch
rastwrite		equ	18		;12	vic raster write latch
litepenx		equ	19		;13
litepeny		equ	20		;14
spenable		equ	21		;15	sprite enable
viccontreg2		equ	22		;16	0x0c0 vic control register 2
spexpany		equ	23		;17	sprite y expansion
vicmempt		equ	24		;18	0x001 vic memory pointer
vicflag		equ	25		;19	0x070 vic interupt reg. writelatch
vicmask		equ	26		;1a	0x0f0 vic interupt enable
spritedp		equ	27		;1b	sprite data priority
spmulticolor	equ	28		;1c	sprite multicolor
spexpanx		equ	29		;1d	sprite x expantion
spspcollide		equ	30		;1e	sprite-sprite collision
spgrfxcollide	equ	31		;1f	sprite-data collision

bordercolor		equ	32		;20	0x0f0 border color
vbc0			equ	33		;21	0x0f0 background color 0
vbc1			equ	34		;22	0x0f0 mcm1
vbc2			equ	35		;23	0x0f0 mcm2
vbc3			equ	36		;24	0x0f0 pen1
spmulticolor1	equ	37		;25	0x0f0 sprite multicolor 1
spmulticolor2	equ	38		;26	0x0f0 sprite multicolor 2
sprite0c		equ	39		;27	0x0f0 sprite 0 color
sprite1c		equ	40		;28	0x0f0
sprite2c		equ	41		;29	0x0f0
sprite3c		equ	42		;2a	0x0f0
sprite4c		equ	43		;2b	0x0f0
sprite5c		equ	44		;2c	0x0f0
sprite6c		equ	45		;2d	0x0f0
sprite7c		equ	46		;2e	0x0f0 sprite 7 color

;vicirqr		equ	47		;2f	read latch 4 vicirq @ $d019
rastread		equ	48		;30	read latch 4 raster @ $d012
rst8read		equ	49		;32	dummy 4 rastread bit 8
raster		equ	50

IRST			equ	1		;raster irq
IMDC			equ	2		;sprite-2-grafix collision irq
IMMC			equ	4		;sprite-2-sprite collision irq
ILP			equ	8		;light pen irq
IRQV			equ	128		;VIC irq enable

DISPLAY_ROW25_START	equ	0x33
DISPLAY_ROW25_END		equ	0xfa
DISPLAY_ROW24_START	equ	0x37
DISPLAY_ROW24_END		equ	0xf6
DISPLAY_DMA_START		equ	0x30
DISPLAY_DMA_END		equ	0xf7	
;-----------------------------------------------------------------------------------
;cpu structure
areg		equ	0
xreg		equ	4
yreg		equ	8
psr		equ	12
psp		equ	16
ppc		equ	20
ppc2		equ	24
irq		equ	28
ciairq	equ	28
vicirq	equ	30
nmi		equ	32
clk		equ	36
inst		equ	40
cycles	equ	44
RDY		equ	48
BA		equ	52
AEC		equ	56
RMW		equ	60
DEBUG		equ	64
;-----------------------------------------------------------------------------------
	
	BITS 32
	GLOBAL	_Reset6569

	GLOBAL	_VIC2
	GLOBAL	_VIC2CHIP
	GLOBAL	_COLORRAM
	GLOBAL	readVIC2
	GLOBAL	writeVIC2
	
	GLOBAL	_StandardTxtMode
	GLOBAL	_ExtendedTxtMode
	GLOBAL	_MultiColorTxtMode
	GLOBAL	_StandardBitMapMode
	GLOBAL	_MultiColorBitMapMode
	GLOBAL	_InvalidTxtMode
	GLOBAL	_InvalidBitMapMode1
	GLOBAL	_InvalidBitMapMode2
	GLOBAL	_UpdateVIC2	
	
	EXTERN	_cpu	
	EXTERN 	_CIACHIP1
	EXTERN 	_CIACHIP2	
	EXTERN	_victimer
	EXTERN	_mnsvline
	EXTERN	_rasterlatch
	EXTERN	_matrixid
	EXTERN	_Spriteptrs
	EXTERN	_CharBank
	EXTERN	_VicBank
	EXTERN	_bitmapbase
	EXTERN	_screenbase
	EXTERN	_VideoMatrix
	EXTERN	_screenmode
	EXTERN	_bankid
	EXTERN	_VCBASE
	EXTERN	_VCCOUNT
	EXTERN	_scrollx
	EXTERN	_scrolly
	
	EXTERN	_gcoll
	EXTERN	_gfxchar
	EXTERN	_vmliptr
	EXTERN	_vmlcptr
	EXTERN	_RC
	EXTERN	_sp2gfxcol
	EXTERN	_scrmode
	EXTERN	_mnshline
	EXTERN	_mcmcol3
	EXTERN	_mcmcol2
	EXTERN	_mcmcol1
	EXTERN	_RAM	

	EXTERN	_BorderColor
	EXTERN	_PaperColor
	EXTERN	_in_dma_window
	EXTERN	_in_deltay_window
	EXTERN	_txtmode
	EXTERN	_badline_enable_flag
	EXTERN	_screen_enable_flag
		
;-----------------------------------------------------------------------------------
%include "include\vicdata.inc"
;-----------------------------------------------------------------------------------
[SECTION .text]
;-----------------------------------------------------------------------------------
%macro DOCHANGES 0
	pushad
	xor		ebx,ebx
	mov		edx,[_VCBASE]
	mov		bl,[esi+0x18]
	mov		[_matrixid],ebx
	
	mov		al,[_CIACHIP2]
	xor		eax,0x03
	and		eax,0x03
	mov		[_bankid],eax
		
	shl		eax,0x0e
		
	mov		[_VicBank],eax
	and		ebx,0xf0
	shl		ebx,0x06
		
	add		ebx,eax
	mov		[_VideoMatrix],ebx
	mov		ecx,ebx
		
	add		ecx,edx
	mov		[_screenbase],ecx

	mov		ecx,ebx
	add		ecx,0x3f8
	and		ecx,0xffff
	mov		[_Spriteptrs],ecx
	
	mov		ecx,[_matrixid]
	and		ecx,0x08
	shl		ecx,0x0a
	shl		edx,0x03
	add		ecx,eax
	add		ecx,edx
	mov		[_bitmapbase],ecx

	mov		ecx,[_matrixid]
	mov		edx,eax
	and		ecx,0x0e
	mov		[_matrixid],ecx
	shl		ecx,0x0a
	add		ecx,eax
	mov		[_CharBank],ecx
	
	mov		al,[esi+0x16]
	mov		bl,[esi+0x11]
	mov		dl,al
	mov		cl,bl
	and		eax,0x10
	and		ebx,0x60
	or		eax,ebx
	shr		eax,0x04
	mov		[_screenmode],eax
	mov		eax,0x07
	and		ecx,eax
	and		edx,eax
	add		dl,24
	mov		[_scrolly],ecx
	mov		[_scrollx],edx
	popad
%endmacro
;-----------------------------------------------------------------------------------	
	ALIGN	32
_VIC2:
	mov	edx,[_victimer]		;lines per frame
	mov	esi,_VIC2CHIP
	mov	cl,[esi+viccontreg1]
	mov	ch,cl				;must latch bit 7
	mov	bl,[esi+vicmask]		;vic intenable
	and	ch,0x07f
	mov	bh,[esi+vicflag]		;vic irq read latch

;-----------------------------------------------------------------------------------
;update raster what pig!! re. bit 8

	mov	eax,[esi+raster]
	inc	eax
	cmp	eax,edx			;option NTSC/Pal option
	jb	rasterupdated
	xor	eax,eax			;raster finished... NB vbcount
rasterupdated
	mov	[_mnsvline],eax
	mov	[esi+raster],eax
	mov	[esi+rastread],al		;store raster count
	shl	ah,0x07
	or	ch,ah				;get latched bit
	mov	[esi+rst8read],ch		;all done with rasty 4 now

;-----------------------------------------------------------------------------------	
;VIC interupt enable address $d01a has same bits as $d019, but no IRQV bit
;and is not latched

generateVIC2ints
	mov	edi,_cpu
	mov	ecx,0x1ff
	mov	edx,[_rasterlatch]
	mov	eax,[esi+raster]
	and	edx,ecx
	and	eax,ecx
	cmp	edx,eax			;is raster writelatch=raster readlatch?
	jnz	VIC2out
	or	bh,IRST
	test	bl,IRST
	jz	rastIRQ
	or	bh,IRQV
	mov	byte[edi+vicirq],0xff	;send irq to 6510
rastIRQ
	mov	[esi+vicflag],bh		;save
VIC2out:
	cld
	xor	eax,eax
	mov	[_in_dma_window],al
	mov	[_in_deltay_window],al
			
	mov	edx,_VIC2CHIP
	mov	esi,edx
	mov	cl,[esi+viccontreg1]
			
	mov	bl,cl
	mov	eax,[_mnsvline]		;get current rasterline
	mov	[_txtmode],cl
			
	and	bl,0x10			;is screen enabled?
	jz	.displayoff
	mov	eax,[_mnsvline]
		
	cmp	eax,DISPLAY_DMA_START	;was DEN bit set on or before raster line $0x30?
	jnz	.displayoff
	mov	[_badline_enable_flag],bl
.displayoff:
	mov	[_screen_enable_flag],bl
	mov	cl,[esi+viccontreg2]
	mov	[_scrmode],cl
		
	cmp	eax,51			;is beam>=51
	jle	inborder
	cmp	eax,251
	jg	inborder			;and beam<=251
		
	mov	edi,_sp2gfxcol		;ok then lets clear collision gfx buffer
	mov	[_gcoll],edi
	xor	eax,eax
	mov	ecx,0x60
	rep
	stosd
inborder:			
	ret
;-----------------------------------------------------------------------------------
;This rountine places into ax the location of pseudo VIC2 registers.
;the read-table only differs from the write-table minutely.
;eg. offset/register 12 points to the raster read-latch it the
;    write-table, it points to raster write latch @ offset 12
;I know its a waste of memory, but its faster than a load of "ifs"
	ALIGN 32
readVIC2:
	mov	ebx,eax
	mov	esi,_VIC2CHIP
	and	eax,0xd03f
	and	ebx,0x3f				;mirror mirror on the wall :-)
	jmp	[ebx*4+VIC2readtable]
	nop
	ALIGN 32
readVICnorm:
	mov	bl,[esi+ebx]
	ret
	ALIGN 32
readVICcolor:
	mov	bl,[esi+ebx]
	or	bl,0xf0
	ret
	ALIGN 32
readVICcon2
	mov	bl,[esi+ebx]
	or	bl,0xc0
	ret
	ALIGN 32
readVICflag
	mov	bl,[esi+ebx]
	or	bl,0x70
	ret
	ALIGN 32
readVICmask
	mov	bl,[esi+ebx]
	or	bl,0xf0
	ret
	ALIGN 32
readVICmempt
	mov	bl,[esi+ebx]
	or	bl,0x01
	ret
	ALIGN 32
readVICcollide
	push	edx
	mov	edx,ebx
	mov	bl,[esi+ebx]
	mov	byte[esi+edx],0x00			;collision register is clear upon read
	and	ebx,0xff
	pop	edx
	ret
	ALIGN 32
readVICraster
	mov	bl,[esi+rastread]				;low byte of raster counter
	ret
	ALIGN 32
readVICrast8
	mov	bl,[esi+rst8read]				;hi byte of raster counter
	ret
	ALIGN 32
readVICundef
	xor	ebx,ebx
	mov	bl,0xff
	ret
	ALIGN 32
readSPRX0
	mov	bl,[esi+sprite0x]				;lo byte of X position
	ret
	ALIGN 32
readSPRX1
	mov	bl,[esi+sprite1x]
	ret
	ALIGN 32
readSPRX2
	mov	bl,[esi+sprite2x]
	ret
	ALIGN 32
readSPRX3
	mov	bl,[esi+sprite3x]
	ret
	ALIGN 32
readSPRX4
	mov	bl,[esi+sprite4x]
	ret
	ALIGN 32
readSPRX5
	mov	bl,[esi+sprite5x]
	ret
	ALIGN 32
readSPRX6
	mov	bl,[esi+sprite6x]
	ret
	ALIGN 32
readSPRX7
	mov	bl,[esi+sprite7x]
	ret
	ALIGN 32
readSPRY0
	mov	bl,[esi+sprite0y]				;Y position max is 0xff
	ret
	ALIGN 32
readSPRY1
	mov	bl,[esi+sprite1y]
	ret
	ALIGN 32
readSPRY2
	mov	bl,[esi+sprite2y]
	ret
	ALIGN 32
readSPRY3
	mov	bl,[esi+sprite3y]
	ret
	ALIGN 32
readSPRY4
	mov	bl,[esi+sprite4y]
	ret
	ALIGN 32
readSPRY5
	mov	bl,[esi+sprite5y]
	ret
	ALIGN 32
readSPRY6
	mov	bl,[esi+sprite6y]
	ret
	ALIGN 32
readSPRY7
	mov	bl,[esi+sprite7y]
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
;This routine places into ax the location of pseudo VIC registers.
;read blur 4 readVIC rountine.
	ALIGN 32
writeVIC2:
	mov	esi,_VIC2CHIP
	mov	eax,ebp
	and	eax,0x3f
	jmp	[eax*4+VIC2writetable]
	nop
	nop
	ALIGN 32
;-----------------------------------------------------------------------------------
writeviccollS
writeviccollG
	nop					;writing to collision register has no effect
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
writeVICKY
	mov	[esi+eax],bl
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
writeVICCON2
	mov	bh,[esi+eax]
	mov	[esi+eax],bl		;store wrst8
	mov	al,bl
	and	al,0x38
	and	bh,0x38
	cmp	al,bh
	jz	near nochangecon2
	DOCHANGES
nochangecon2
	and	ebx,0x7
	mov	[_scrollx],ebx
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
writevicrst8
	mov	bh,[esi+eax]
	mov	[esi+eax],bl		;store wrst8
	mov	al,bl
	and	al,0x78
	and	bh,0x78
	cmp	al,bh
	jz	near nochangerst8
	DOCHANGES
nochangerst8
	push	edx
	push	ecx
	mov	cl,bl
	and	ecx,0x7
	mov	[_scrolly],ecx
	mov	ecx,[_rasterlatch]
	mov	edx,[esi+raster]
		
	mov	al,bl
	and	bl,0x7f
	mov	bl,[esi+rst8read]
	and	al,0x80
	mov	ah,al
	or	al,bl
	mov	[esi+rst8read],al		;copy lower bits to rrst8
	shr	ah,0x7
	mov	al,cl				;new rasterlatch
	mov	ebx,0x1ff

	and	eax,ebx			;new rasterlatch			
	and	ecx,ebx			;old rasterlatch
	and	edx,ebx		

	mov	bh,[esi+vicflag]
	mov	bl,[esi+vicmask]
	cmp	eax,ecx
	jz	sortrasty2
	cmp	eax,edx			;is raster writelatch=raster readlatch?
	jnz	sortrasty2
	or	bh,IRST
	test	bl,IRST
	jz	sortrasty2
	or	bh,IRQV
	mov	byte[edi+vicirq],0xff	;send irq to 6510
sortrasty2
	mov	[esi+vicflag],bh		;save
	mov	[_rasterlatch],eax	
	pop	ecx
	pop	edx	
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
write2raster
	push	edx
	push	ecx
	mov	ecx,[_rasterlatch]
	mov	edx,[esi+raster]
	mov	[esi+eax],bl
	mov	eax,ecx
	mov	cl,bl
	mov	ebx,0x1ff
	and	eax,ebx
	and	ecx,ebx			;new rasterlatch
	and	edx,ebx
	mov	bh,[esi+vicflag]
	mov	bl,[esi+vicmask]
		
	cmp	ecx,eax
	jz	sortrasty4
	cmp	ecx,edx			;is raster writelatch=raster readlatch?
	jnz	sortrasty4
	or	bh,IRST
	test	bl,IRST
	jz	sortrasty4
	or	bh,IRQV
	mov	byte[edi+vicirq],0xff	;send irq to 6510
sortrasty4
	mov	[esi+vicflag],bh		;save
	mov	[_rasterlatch],ecx	
	pop	ecx
	pop	edx
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
writevicflag
	mov	byte[edi+vicirq],0x00		;clear interupt register
	not	bl
	and	bl,0x0f
	mov	al,[esi+vicflag]
	and	al,bl
	mov	bl,al
	and	al,[esi+vicmask]
	jz	.bye
	or	bl,0x80				;trigger IRQ
.bye	
	mov	[esi+vicflag],bl			;new setting for interupts
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
writevicmask
	and	bl,0x0f
	mov	[esi+vicmask],bl
	mov	al,[esi+vicflag]
	and	bl,al
	jz	.nointerrupt
	or	al,0x80
	mov	byte[edi+vicirq],0xff
	mov	[esi+vicflag],al
	ret
.nointerrupt
	and	al,0x7f
	mov	byte[edi+vicirq],0x00
	mov	[esi+vicflag],al	
	ret
	ALIGN 32
writeundefVICKY
	nop
	nop
	xor	ebx,ebx
	xor	eax,eax
	ret
	ALIGN 32
writeVICBDCOL
	and	bl,0x0f
	mov	[esi+eax],bl
	push	eax
		
	mov	bh,bl					;copy to upper nybble
		
	mov	eax,ebx				;copy background color
					
	shl	eax,0x10				;copy to upper bytes pair
	or	eax,ebx				;copy to lower byte pair
	mov	[_BorderColor],eax
	pop	eax
	ret
	ALIGN 32
writeVICPPRCOL
	and	bl,0x0f
	mov	[esi+eax],bl
	push	eax
		
	mov	bh,bl					;copy to upper nybble
		
	mov	eax,ebx				;copy background color
					
	shl	eax,0x10				;copy to upper bytes pair
	or	eax,ebx				;copy to lower byte pair
	mov	[_PaperColor],eax
	pop	eax
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
; write sprite co-ords

writeSPRX0
	mov	[esi+sprite0x],bl
	ret
	ALIGN 32
writeSPRX1
	mov	[esi+sprite1x],bl
	ret
	ALIGN 32
writeSPRX2
	mov	[esi+sprite2x],bl
	ret
	ALIGN 32
writeSPRX3
	mov	[esi+sprite3x],bl
	ret
	ALIGN 32
writeSPRX4
	mov	[esi+sprite4x],bl
	ret
	ALIGN 32
writeSPRX5
	mov	[esi+sprite5x],bl
	ret
	ALIGN 32
writeSPRX6
	mov	[esi+sprite6x],bl
	ret
	ALIGN 32
writeSPRX7
	mov	[esi+sprite7x],bl
	ret
	ALIGN 32
writeSPRXX
	pushad
	mov	[esi+eax],bl
	mov	al,bl
	and	al,0x1
	shr	bl,0x1
	mov	[esi+(sprite0x+1)],al
	mov	al,bl
	and	al,0x1
	shr	bl,0x1
	mov	[esi+(sprite1x+1)],al
	mov	al,bl
	and	al,0x1
	shr	bl,0x1
	mov	[esi+(sprite2x+1)],al
	mov	al,bl
	and	al,0x1
	shr	bl,0x1
	mov	[esi+(sprite3x+1)],al
	mov	al,bl
	and	al,0x1
	shr	bl,0x1
	mov	[esi+(sprite4x+1)],al
	mov	al,bl
	and	al,0x1
	shr	bl,0x1
	mov	[esi+(sprite5x+1)],al
	mov	al,bl
	and	al,0x1
	shr	bl,0x1
	mov	[esi+(sprite6x+1)],al
	and	bl,0x1
	mov	[esi+(sprite7x+1)],bl
	popad
	ret
	ALIGN 32
writeSPRY0
	mov	[esi+sprite0y],bl
	ret
	ALIGN 32
writeSPRY1
	mov	[esi+sprite1y],bl
	ret
	ALIGN 32
writeSPRY2
	mov	[esi+sprite2y],bl
	ret
	ALIGN 32
writeSPRY3
	mov	[esi+sprite3y],bl
	ret
	ALIGN 32
writeSPRY4
	mov	[esi+sprite4y],bl
	ret
	ALIGN 32
writeSPRY5
	mov	[esi+sprite5y],bl
	ret
	ALIGN 32
writeSPRY6
	mov	[esi+sprite6y],bl
	ret
	ALIGN 32
writeSPRY7
	mov	[esi+sprite7y],bl
	ret
	ALIGN 32
		
;-----------------------------------------------------------------------------------
; these routines draw a line on the virtual screen
_UpdateVIC2
	pushad
	xor		eax,eax
	xor		ebx,ebx
	xor		ecx,ecx
	mov		edx,[_VCBASE]
	
	mov		bl,[esi+0x18]
	
	mov		[_matrixid],ebx
	
	mov		al,[_CIACHIP2]
		
	xor		eax,0x03
	and		eax,0x03
	mov		[_bankid],eax
		
	shl		eax,0x0e
		
	mov		[_VicBank],eax
	and		ebx,0xf0
	shl		ebx,0x06
		
	add		ebx,eax
	mov		[_VideoMatrix],ebx
	mov		ecx,ebx
		
	add		ecx,edx
	mov		[_screenbase],ecx

	mov		ecx,ebx
	add		ecx,0x3f8
	and		ecx,0xffff
	mov		[_Spriteptrs],ecx
	
	mov		ecx,[_matrixid]
	and		ecx,0x08
	shl		ecx,0x0a
	shl		edx,0x03
	add		ecx,eax
	add		ecx,edx
	mov		[_bitmapbase],ecx

	mov		ecx,[_matrixid]
	mov		edx,eax
	and		ecx,0x0e
	mov		[_matrixid],ecx
	shl		ecx,0x0a
	add		ecx,eax
	mov		[_CharBank],ecx
	
	mov		al,[esi+0x16]
	mov		bl,[esi+0x11]
	mov		dl,al
	mov		cl,bl
	and		eax,0x10
	and		ebx,0x60
	or		eax,ebx
	shr		eax,0x04
	mov		[_screenmode],eax
	mov		eax,0x07
	and		ecx,eax
	and		edx,eax
	add		dl,24
	mov		[_scrolly],ecx
	mov		[_scrollx],edx
		
	mov		eax,[_vmliptr]
	mov		ebx,[_vmlcptr]
	mov		edi,eax			;char
	mov		esi,ebx			;color
		
	mov		eax,_RAM
	mov		ebx,_COLORRAM
	mov		edx,[_VCCOUNT]
		
	add		ebx,edx			;color
		
	mov		edx,[_screenbase]
	add		edx,eax			;screen
	mov		ebp,ebx			;color
	xor		ebx,ebx
	mov		ecx,10
.clp
	mov		eax,[ebp+ebx]			;color
	mov		[esi+ebx],eax
	mov		eax,[edx+ebx]
	mov		[edi+ebx],eax
	add		ebx,0x4
	loop		.clp
	popad
	ret
	ALIGN		32
;-----------------------------------------------------------------------------------
; these routines draw a line on the virtual screen
;-----------------------------------------------------------------------------------
;void StandardTxtMode()
_StandardTxtMode:
	pushad	
	cld							;make sure we moving forwards
	xor		ebx,ebx				;clear ebx
	mov		eax,[_PaperColor]
	mov		edx,[_mnshline]			;vscreen pointer
	mov		edi,edx				;store vscreen pointer
	mov		ecx,0x50				;length of screen in bits
	rep							;use dword copy
	stosd							;to paint background
		
	mov		edi,edx				;restore vscreen pointer	

	mov		esi,[_gcoll]			;esi=collision buffer
	mov		ebp,[_gfxchar]			;ebp=char bitmaps 
	mov		edx,[_vmliptr]			;char codes pointer
	mov		ecx,[_vmlcptr]			;color pointer

	
	mov		eax,-0x28				;screen width in bytes
								;count up from -40 to 0
stdmnloop:
	push		eax
	push		edx
					
	mov		bl,[edx]				;charcode=*ascii
	shl		ebx,0x03				;offset=ascii*8 
	add		ebx,[_RC]				;find row
	mov		al,[ebp+ebx]			;bitpatter=*(gfxchar+offset)
	test		al,0xff				;is it a space?
	jz	near	stdbit0x100				;if space skip decode
	xor		edx,edx
	mov		bl,[ecx]				;col1=*colorram
	and		bl,0x0f				;col1&=0xf
	inc		edx					;collision bit
	
stdbit0x180:
	test		al,0x80				;is bit 7 on?
	jz		stdbit0x140				;if nope jmp2 nxt bit
	mov		[edi],bl				;paint pixel col1
	mov		[esi],dl				;turn on collision bit
		
stdbit0x140:
	test		al,0x40
	jz		stdbit0x120
	mov		[edi+1],bl
	mov		[esi+1],dl

					
stdbit0x120:
	test		al,0x20
	jz		stdbit0x110
	mov		[edi+2],bl
	mov		[esi+2],dl
					
stdbit0x110:
	test		al,0x10
	jz		stdbit0x108
	mov		[edi+3],bl
	mov		[esi+3],dl

stdbit0x108:
	test		al,0x08
	jz		stdbit0x104
	mov		[edi+4],bl
	mov		[esi+4],dl
	
stdbit0x104:
	test		al,0x04
	jz		stdbit0x102
	mov		[edi+5],bl
	mov		[esi+5],dl
		
stdbit0x102:
	test		al,0x02
	jz		stdbit0x101
	mov		[edi+6],bl
	mov		[esi+6],dl
				
stdbit0x101:
	test		al,0x01
	jz		stdbit0x100
	mov		[edi+7],bl
	mov		[esi+7],dl
	
stdbit0x100:
	mov		ebx,0x08
	pop		edx
	pop		eax
					
	add		edi,ebx			;vscreen+8
	add		esi,ebx			;collision buffer+8
			
	inc		edx				;asii++
	inc		ecx				;colorram++	
	inc		eax				;loop++
	jnz	near	stdmnloop
	popad
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
;void ExtendedTxtMode()
_ExtendedTxtMode:
	pushad	
	cld							;make sure we moving forwards
	xor		ebx,ebx
	mov		edx,[_mnshline]			;vscreen pointer
	mov		edi,edx				;store vscreen pointer
	mov		esi,[_gcoll]			;esi=collision buffer
	mov		ebp,[_gfxchar]			;ebp=char bitmaps 
	mov		edx,[_vmliptr]			;char codes pointer
	mov		ecx,[_vmlcptr]			;color pointer
	xor		ebx,ebx				;clear ebx
	mov		eax,-0x28				;screen width in bytes
								;count up from -40 to 0
ecmmnloop:
	push		eax
	push		edx
	xor		eax,eax				
	mov		bl,[edx]				;charcode=*ascii
	mov		al,[edx]
		
;	and		al,0xc0
	and		bl,0x3f				;first 64 only
	shr		al,0x06

	mov		dh,[_PaperColor]			;ECM color
		
	shl		ebx,0x03				;offset=ascii*8 
	add		ebx,[_RC]				;find row
	mov		al,[ebp+ebx]			;bitpatter=*(gfxchar+offset)
	mov		dl,[ecx]

	xor		ebx,ebx
	and		edx,0x0f0f				;col1&=0xf
	inc		ebx					;collision bit
	
ecmbit0x181:
	test		al,0x80				;is bit 7 on?
	jz		ecmbit0x180				;if nope jmp2 nxt bit
	mov		[edi],dl				;paint pixel col1
	mov		[esi],bl				;turn on collision bit
	jmp		ecmbit0x141
ecmbit0x180:
	mov		[edi],dh				;paint bkg
		
ecmbit0x141:
	test		al,0x40
	jz		ecmbit0x140
	mov		[edi+1],dl
	mov		[esi+1],bl
	jmp		ecmbit0x121
ecmbit0x140:
	mov		[edi+1],dh				;paint bkg
					
ecmbit0x121:
	test		al,0x20
	jz		ecmbit0x120
	mov		[edi+2],dl
	mov		[esi+2],bl
	jmp		ecmbit0x111
ecmbit0x120:
	mov		[edi+2],dh				;paint bkg				
	
ecmbit0x111:
	test		al,0x10
	jz		ecmbit0x110
	mov		[edi+3],dl
	mov		[esi+3],bl
	jmp		ecmbit0x1081
ecmbit0x110:
	mov		[edi+3],dh				;paint bkg 
	
ecmbit0x1081:
	test		al,0x08
	jz		ecmbit0x1080
	mov		[edi+4],dl
	mov		[esi+4],bl
	jmp		ecmbit0x1041
ecmbit0x1080:
	mov		[edi+4],dh				;paint bkg
		
ecmbit0x1041:
	test		al,0x04
	jz		ecmbit0x1040
	mov		[edi+5],dl
	mov		[esi+5],bl
	jmp		ecmbit0x1021
ecmbit0x1040:
	mov		[edi+5],dh				;paint bkg
		
ecmbit0x1021:
	test		al,0x02
	jz		ecmbit0x1020
	mov		[edi+6],dl
	mov		[esi+6],bl
	jmp		ecmbit0x1011
ecmbit0x1020:
	mov		[edi+6],dh				;paint bkg
		
ecmbit0x1011:
	test		al,0x01
	jz		ecmbit0x1010
	mov		[edi+7],dl
	mov		[esi+7],bl
	jmp		ecmbit0x1000
ecmbit0x1010:
	mov		[edi+7],dh				;paint bkg
		
ecmbit0x1000:
	mov		ebx,0x08
	pop		edx
	pop		eax
					
	add		edi,ebx			;vscreen+8
	add		esi,ebx			;collision buffer+8
			
	inc		edx				;asii++
	inc		ecx				;colorram++	
	inc		eax				;loop++
	jnz	near	ecmmnloop
	popad
	ret
	ALIGN 32

;-----------------------------------------------------------------------------------
;void MultiColorTxtMode()
_MultiColorTxtMode:
	pushad	
	cld		
	mov		edx,[_mnshline]		;vscreen pointer
	mov		eax,[_PaperColor]
	xor		ebx,ebx
	mov		ecx,0x50
	mov		edi,edx
	rep
	stosd						;paint background
	mov		edi,edx			;restore vscreen pointer

	mov		esi,[_gcoll]		;collision pointer
				
	mov		[_mcmcol3],eax		;copy col0 to mcm3 buffer
				
	mov		ebx,eax
	mov		cl,0x0f
	mov		bl,[_VIC2CHIP+0x22]	;mcm col reg 0
	mov		al,[_VIC2CHIP+0x23]	;mcm col reg 1
	and		bl,cl
	and		al,cl
	mov		bh,bl				;ebx=col0,col0,mcm1,mcm1
	mov		ah,al				;eax=col0,col0,mcm2,mcm2
	mov		ebp,[_gfxchar]		;bitmaps pointer
		
	mov		[_mcmcol2],eax		;mcm2 buffer=eax
	mov		[_mcmcol1],ebx		;mcm1 buffer=ebx
		
	mov		edx,[_vmliptr]		;c64scrnram pointer
	mov		ecx,[_vmlcptr]		;colorram pointer
		
	mov		eax,-0x28
				
mcmmnloop:
	push		eax
	mov		bl,[ecx]			;get col1
	mov		al,[edx]			;get ascii code
				
	push		ecx
	push		edx
		
	and		eax,0xff			;insurance

	shl		eax,0x03			;(ascii*8)+..
	add		eax,[_RC]			;..rowcount=bitmap

	mov		al,[ebp+eax]		;al=bitmap
	test		al,0xff			;if space then..
	jz	near	mcmtxtbit0x00		;goto endloop
				
	mov		ebx,[_mcmcol3]		;ebx=mcmcol3
	mov		bl,[ecx]			;bl=colorram
	and		bl,0x0f
;	mov		edx,0x101
	cmp		bl,0x08			;if(bl<8) then ..
	jl	near	mcmstdbit0x80		;goto paintstdtxt
					
mcmtxtbit0x80:
	sub		bl,0x08			;bl=bl-8
	mov		bh,bl				;copy to up byte
	mov		ecx,[_mcmcol2]		;ecx=mcmcol2
	mov		edx,[_mcmcol1]		;edx=mcmcol1
				
	test		al,0x80			;if(al&0x80==0)
	jz		mcmtxtbit0x40		;then goto bitpattern01bbccdd
	mov		dword[esi],0x101		;collision buffer=0x11 ie foreground
		
	test		al,0x40			;if(al&0x40==0)
	jz		mcmtxtbit0x84
	mov		[edi],ebx			;bitpattern=11bbccdd so paint as mcmcol3
	jmp		mcmtxtbit0x20		;jmp2  bitpattern_aa11ccdd	
	
mcmtxtbit0x84:					;note bitpattern is 10bbccdd
	mov		[edi],ecx			;paint bitpair as mcmcol2
	jmp		mcmtxtbit0x20		;jmp2 bitpatternaa11ccdd

mcmtxtbit0x40:
	test		al,0x40
	jz		mcmtxtbit0x20		;if(bitpattern!=01bbccdd) jmp2 bitpattern_aa11ccdd
	mov		[edi],edx			;paint bitpair as mcmcol1

mcmtxtbit0x20:
	test		al,0x20
	jz		mcmtxtbit0x10
	mov		dword[esi+2],0x101
	test		al,0x10
	jz		mcmtxtbit0x21
	mov		[edi+2],ebx				
	jmp		mcmtxtbit0x08
mcmtxtbit0x21:
	mov		[edi+2],ecx				
	jmp		mcmtxtbit0x08		
mcmtxtbit0x10:
	test		al,0x10
	jz		mcmtxtbit0x08
	mov		[edi+2],edx					
			
mcmtxtbit0x08:
	test		al,0x08
	jz		mcmtxtbit0x04
	mov		dword[esi+4],0x101
	test		al,0x04
	jz		mcmtxtbit0x084
	mov		[edi+4],ebx				
	jmp		mcmtxtbit0x02						
mcmtxtbit0x084:
	mov		[edi+4],ecx				
	jmp		mcmtxtbit0x02		
mcmtxtbit0x04:
	test		al,0x04
	jz		mcmtxtbit0x02
	mov		[edi+4],edx	

mcmtxtbit0x02:
	test		al,0x02
	jz		mcmtxtbit0x01
	mov		dword[esi+6],0x101
	test		al,0x01
	jz		mcmtxtbit0x021
	mov		[edi+6],ebx				
	jmp		mcmbye
mcmtxtbit0x021:
	mov		[edi+6],ecx				
	jmp		mcmbye		
mcmtxtbit0x01:
	test		al,0x01
	jz		mcmbye
	mov		[edi+6],edx
mcmbye:	
	pop		edx
	pop		ecx
	pop		eax
	lea		esi,[esi+8]					
	lea		edi,[edi+8]
	inc		edx
	inc		ecx
	inc		eax
	jnz	near	mcmmnloop				
	jmp		byebyemcm					
	
mcmstdbit0x80:
	mov		dl,0x01
	test		al,0x80
	jz		mcmstdbit0x40
	mov		[edi],bl
	mov		[esi],dl

mcmstdbit0x40:
	test		al,0x40
	jz		mcmstdbit0x20
	mov		[edi+1],bl
	mov		[esi+1],dl

mcmstdbit0x20:
	test		al,0x20
	jz		mcmstdbit0x10
	mov		[edi+2],bl
	mov		[esi+2],dl

mcmstdbit0x10:
	test		al,0x10
	jz		mcmstdbit0x08
	mov		[edi+3],bl
	mov		[esi+3],dl

mcmstdbit0x08:
	test		al,0x08
	jz		mcmstdbit0x04
	mov		[edi+4],bl
	mov		[esi+4],dl

mcmstdbit0x04:
	test		al,0x04
	jz		mcmstdbit0x02
	mov		[edi+5],bl
	mov		[esi+5],dl
mcmstdbit0x02:
	test		al,0x02
	jz		mcmstdbit0x01
	mov		[edi+6],bl
	mov		[esi+6],dl

mcmstdbit0x01:
	test		al,0x01
	jz		mcmtxtbit0x00
	mov		[edi+7],bl
	mov		[esi+7],dl

mcmtxtbit0x00:
	xor		eax,eax
	lea		edi,[edi+eax+8]
	lea		esi,[esi+eax+8]
	pop		edx
	pop		ecx
	pop		eax
	inc		edx
	inc		ecx
	inc		eax
	jnz	near	mcmmnloop
byebyemcm:
	popad
	ret

	ALIGN	32
;-----------------------------------------------------------------------------------
;void StandardBitMapMode()
_StandardBitMapMode:
	pushad
	
	mov		edi,[_mnshline]
	mov		esi,[_gcoll]
	mov		ebx,[_bitmapbase]

	mov		ecx,[_vmliptr]
	add		ebx,[_RC]
	lea		ebp,[ebx+_RAM]
					
	mov		eax,-0x28
stdbmpmnloop:
	push		eax
	push		ecx
		
	mov		al,[ecx]
	mov		bl,al			;bl=col1
	mov		cl,al			;cl=col0
		
	xor		edx,edx
	and		cl,0x0f
	shr		bl,0x04
		
	mov		al,[ebp]
	inc		edx
	
stdbmpbit0x181:
	test		al,0x80
	jz		stdbmpbit0x180
	mov		[edi],bl
	mov		[esi],dl
	jmp		stdbmpbit0x141
stdbmpbit0x180:
	mov		[edi],cl

stdbmpbit0x141:
	test		al,0x40
	jz		stdbmpbit0x140
	mov		[edi+1],bl
	mov		[esi+1],dl
	jmp		stdbmpbit0x121
stdbmpbit0x140:
	mov		[edi+1],cl

stdbmpbit0x121:
	test		al,0x20
	jz		stdbmpbit0x120
	mov		[edi+2],bl
	mov		[esi+2],dl
	jmp		stdbmpbit0x111
stdbmpbit0x120:
	mov		[edi+2],cl

stdbmpbit0x111:
	test		al,0x10
	jz		stdbmpbit0x110
	mov		[edi+3],bl
	mov		[esi+3],dl
	jmp		stdbmpbit0x1081
stdbmpbit0x110:
	mov		[edi+3],cl
	
stdbmpbit0x1081:
	test		al,0x08
	jz		stdbmpbit0x1080
	mov		[edi+4],bl
	mov		[esi+4],dl
	jmp		stdbmpbit0x1041
stdbmpbit0x1080:
	mov		[edi+4],cl

stdbmpbit0x1041:
	test		al,0x04
	jz		stdbmpbit0x1040
	mov		[edi+5],bl
	mov		[esi+5],dl
	jmp		stdbmpbit0x1021
stdbmpbit0x1040:
	mov		[edi+5],cl	
	
stdbmpbit0x1021:
	test		al,0x02
	jz		stdbmpbit0x1020
	mov		[edi+6],bl
	mov		[esi+6],dl
	jmp		stdbmpbit0x1011
stdbmpbit0x1020:
	mov		[edi+6],cl

stdbmpbit0x1011:
	test		al,0x01
	jz		stdbmpbit0x1010
	mov		[edi+7],bl
	mov		[esi+7],dl
	jmp		stdbmpbit0x0000
stdbmpbit0x1010:
	mov		[edi+7],cl	
	
stdbmpbit0x0000:
	mov		ebx,0x08
		
	add		ebp,ebx
		
	add		edi,ebx
	add		esi,ebx
	pop		ecx
	pop		eax		
	inc		ecx					
	inc		eax
	jnz	near	stdbmpmnloop
	popad
	ret
	ALIGN 32

;-----------------------------------------------------------------------------------
;void MultiColorBitMapMode(void)
_MultiColorBitMapMode:
	pushad	
	cld		
	xor		ebx,ebx
	mov		edx,[_mnshline]			;vscreen pointer
	mov		eax,[_PaperColor]
	mov		ecx,0x50		

	mov		edi,edx				;point->vscreen

	rep
	stosd							;paint vscree color0
	mov		edi,edx				;restore vscreen pointer
	
	mov		esi,[_gcoll]			;collision buffer
	
	mov		ebx,[_bitmapbase]			;bitmaps

	mov		[_mcmcol2],eax
	mov		[_mcmcol3],eax

	add		ebx,[_RC]
	mov		eax,-0x28
	lea		ebp,[ebx+_RAM]			;bitmaps+offset
	mov		edx,[_vmliptr]			;mcm col1-2
	mov		ecx,[_vmlcptr]			;col3
	xor		ebx,ebx	
mcmbmpmnloop:
	push		eax
	mov		bl,[ecx]				;col1
	mov		al,[edx]				;mcm col1-2
					
	push		ecx
	push		edx
			
	and		bl,0x0f
	mov		ah,al					;copy to upper byte
	mov		ecx,[_mcmcol3]
	mov		edx,[_mcmcol2]
					
	and		eax,0xf00f				;eax=mcm0,mcm0,mcm3<<4,mcm2
	mov		cl,bl
	mov		dl,al
	mov		ch,bl					;ecx=mcmcol2
	mov		dh,al					;edx=mcmcol1
					
	mov		ebx,[_mcmcol3]
		
	shr		ah,0x04				;ah>>8 .. now positioned properly
		
	mov		bl,ah
	mov		bh,ah					;ebx=mcmcol3
					
	mov		al,[ebp]				;get bitpattern
	test		al,0xff				;skip if zero
	jz	near	mcmbmpbit0x00
					
mcmbmpbit0x80:
					
	test		al,0x80
	jz		mcmbmpbit0x40
	mov		dword[esi],0x101
	test		al,0x40
	jz		mcmbmpbit0x84
	mov		[edi],ecx				;mcm3				
	jmp		mcmbmpbit0x20						
mcmbmpbit0x84:
	mov		[edi],edx				;mcm2
	jmp		mcmbmpbit0x20		
mcmbmpbit0x40:
	test		al,0x40
	jz		mcmbmpbit0x20
	mov		[edi],ebx
			
mcmbmpbit0x20:
	test		al,0x20
	jz		mcmbmpbit0x10
	mov		dword[esi+2],0x101
	test		al,0x10
	jz		mcmbmpbit0x21
	mov		[edi+2],ecx				
	jmp		mcmbmpbit0x08
mcmbmpbit0x21:
	mov		[edi+2],edx				
	jmp		mcmbmpbit0x08		
mcmbmpbit0x10:
	test		al,0x10
	jz		mcmbmpbit0x08
	mov		[edi+2],ebx					
	
mcmbmpbit0x08:
	test		al,0x08
	jz		mcmbmpbit0x04
	mov		dword[esi+4],0x101
	test		al,0x04
	jz		mcmbmpbit0x084
	mov		[edi+4],ecx				
	jmp		mcmbmpbit0x02						
mcmbmpbit0x084:
	mov		[edi+4],edx				
	jmp		mcmbmpbit0x02		
mcmbmpbit0x04:
	test		al,0x04
	jz		mcmbmpbit0x02
	mov		[edi+4],ebx	
					
mcmbmpbit0x02:
	test		al,0x02
	jz		mcmbmpbit0x01
	mov		dword[esi+6],0x101
	test		al,0x01
	jz		mcmbmpbit0x021
	mov		[edi+6],ecx				
	jmp		mcmbmpbit0x00
mcmbmpbit0x021:
	mov		[edi+6],edx				
	jmp		mcmbmpbit0x00		
mcmbmpbit0x01:
	test		al,0x01
	jz		mcmbmpbit0x00
	mov		[edi+6],ebx

mcmbmpbit0x00:
	mov		ebx,8
	pop		edx
	pop		ecx
	pop		eax
					
	add		edi,ebx
	add		esi,ebx
	add		ebp,ebx
					
	inc		edx
	inc		ecx
	inc		eax

	jnz	near	mcmbmpmnloop
	popad
	ret
	ALIGN 32
		
;-----------------------------------------------------------------------------------
;void InvalidTxtMode()
_InvalidTxtMode:
	pushad	
	cld		
	mov		edx,[_mnshline]		;vscreen pointer
	mov		esi,[_gcoll]		;collision pointer
	mov		ebp,[_gfxchar]		;bitmaps pointer
	xor		eax,eax
	xor		ebx,ebx
		
	mov		edi,edx		
	mov		ecx,0x50
	rep
	stosd						;paint background
	
	mov		edx,[_vmliptr]		;c64scrnram pointer
	mov		ecx,[_vmlcptr]		;colorram pointer
		
	mov		eax,-0x28
invtxtmnloop:
	push		edx
	push		ecx
	push		eax

	mov		al,[edx]			;get ascii code
		
	and		eax,0x3f			;insurance

	shl		eax,0x03			;(ascii*8)+..
	add		eax,[_RC]			;..rowcount=bitmap

	mov		al,[ebp+eax]		;al=bitmap
	test		al,0xff			;if space then..
	jz	near	invtxtbit0x00		;goto endloop
				
	mov		bl,[ecx]
	cmp		bl,0x08			;if(bl<8) then ..
	jl	near	invstdbit0x80		;goto paintstdtxt
					
invtxtbit0x80:		
	test		al,0x80			;if(al&0x80==0)
	jz		invtxtbit0x20		;then goto bitpattern01bbccdd
	mov		dword[esi],0x101		;collision buffer=0x11 ie foreground
		
invtxtbit0x20:
	test		al,0x20
	jz		invtxtbit0x08
	mov		dword[esi+2],0x101

			
invtxtbit0x08:
	test		al,0x08
	jz		invtxtbit0x02
	mov		dword[esi+4],0x101

invtxtbit0x02:
	test		al,0x02
	jz		invbye
	mov		dword[esi+6],0x101

invbye:	
	pop		eax
	pop		ecx
	pop		edx
		
	lea		esi,[esi+8]					
	inc		edx
	inc		ecx
	inc		eax
	jnz	near	invtxtmnloop				
	jmp		byebyeinvtxt					
	
invstdbit0x80:
	mov		dl,0x01
	test		al,0x80
	jz		invstdbit0x40
	mov		[esi],dl

invstdbit0x40:
	test		al,0x40
	jz		invstdbit0x20
	mov		[esi+1],dl

invstdbit0x20:
	test		al,0x20
	jz		invstdbit0x10
	mov		[esi+2],dl

invstdbit0x10:
	test		al,0x10
	jz		invstdbit0x08
	mov		[esi+3],dl

invstdbit0x08:
	test		al,0x08
	jz		invstdbit0x04
	mov		[esi+4],dl

invstdbit0x04:
	test		al,0x04
	jz		invstdbit0x02
	mov		[esi+5],dl
invstdbit0x02:
	test		al,0x02
	jz		invstdbit0x01
	mov		[esi+6],dl

invstdbit0x01:
	test		al,0x01
	jz		invtxtbit0x00
	mov		[esi+7],dl

invtxtbit0x00:
	xor		eax,eax
	lea		esi,[esi+eax+8]
	pop		eax
	pop		ecx
	pop		edx
		
	inc		edx
	inc		ecx

	inc		eax
	jnz	near	invtxtmnloop
byebyeinvtxt:
	popad
	ret
	ALIGN	32

;-----------------------------------------------------------------------------------
;void InvalidBitMap1Mode()
_InvalidBitMapMode1:
	pushad
	cld
	mov		edx,[_mnshline]
	mov		esi,[_gcoll]		;collision buffer
	mov		ebx,[_bitmapbase]
	mov		edi,edx
		
	xor		eax,eax
			
	mov		ecx,0x50
	rep
	stosd				
	
	add		ebx,[_RC]			;row counter
	lea		ebp,[ebx+_RAM]		;bitpattern=memory+row count
					
	mov		ecx,-0x28			;screen width
	xor		edx,edx
	xor		eax,eax
	mov		ebx,0x08
	inc		edx
invbmp1mnloop:

	mov		al,[ebp]
	test		al,0xff			;is it a space?
	jz	near	invbmp1bit0x00		;if space skip decode	
	
invbmp1bit0x80:
	test		al,0x80			;is bit7 on?
	jz		invbmp1bit0x40
	mov		[esi],dl			;insert 1 to collision buffer

invbmp1bit0x40:
	test		al,0x40
	jz		invbmp1bit0x20
	mov		[esi+1],dl

invbmp1bit0x20:
	test		al,0x20
	jz		invbmp1bit0x10
	mov		[esi+2],dl

invbmp1bit0x10:
	test		al,0x10
	jz		invbmp1bit0x08
	mov		[esi+3],dl
		
invbmp1bit0x08:
	test		al,0x08
	jz		invbmp1bit0x04
	mov		[esi+4],dl
		
invbmp1bit0x04:
	test		al,0x04
	jz		invbmp1bit0x02
	mov		[esi+5],dl
		
	
invbmp1bit0x02:
	test		al,0x02
	jz		invbmp1bit0x01
	mov		[esi+6],dl
		

invbmp1bit0x01:
	test		al,0x01
	jz		invbmp1bit0x00
	mov		[esi+7],dl
	
invbmp1bit0x00:
	mov		ebx,0x08
	add		ebp,ebx
	add		esi,ebx					
	inc		ecx
	jnz	near	invbmp1mnloop
	popad
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
;void InvalidBitMapMode2(void)
_InvalidBitMapMode2:
	pushad	
	cld		
	xor		ebx,ebx
	mov		edx,[_mnshline]			;vscreen pointer
	mov		edi,edx				;point->vscreen
	xor		eax,eax
	mov		ecx,0x50
	rep
	stosd							;paint vscree color0
	mov		edi,edx				;restore vscreen pointer
	mov		esi,[_gcoll]			;collision buffer
	mov		ebx,[_bitmapbase]			;bitmaps
	add		ebx,[_RC]
	mov		ecx,-0x28				;count
	lea		ebp,[ebx+_RAM]			;bitmaps+offset
	xor		eax,eax
	mov		edx,0x08
	mov		ebx,0x0101
invbmp2mnloop:
	mov		al,[ebp]				;get bitpattern
	test		al,0xff				;skip if zero
	jz	near	invbmp2bit0x00
					
invbmp2bit0x80:
	test		al,0x80
	jz		invbmp2bit0x20
	mov		[esi],ebx
	
invbmp2bit0x20:
	test		al,0x20
	jz		invbmp2bit0x08
	mov		[esi+2],ebx
	
invbmp2bit0x08:
	test		al,0x08
	jz		invbmp2bit0x02
	mov		[esi+4],ebx
					
invbmp2bit0x02:
	test		al,0x02
	jz		invbmp2bit0x00
	mov		[esi+6],ebx

invbmp2bit0x00:
					
	add		esi,edx
	add		ebp,edx
					
	inc		ecx
	jnz	near	invbmp2mnloop
	popad
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
_Reset6569
	xor	eax,eax
	mov	esi,_VIC2CHIP
	mov	ecx,0x7f
clrvic:
	mov	[esi+ecx],al
	loop	clrvic
	ret