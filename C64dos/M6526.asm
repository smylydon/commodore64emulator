;===================================================================================
;-----------------------------------------------------------------------------------
CIA1base		equ	0x0dc00
CIA2base		equ	0x0dd00

ciapra		equ	0
ciaprb		equ	1
ciaddra		equ	2
ciaddrb		equ	3
timerAlwrite	equ	4
timerAhwrite	equ	5
timerBlwrite	equ	6
timerBhwrite	equ	7
TODtenswrite	equ	8
TODsecswrite	equ	9
TODminswrite	equ	10
TODhourwrite	equ	11
ciasdr		equ	12
ciaicr		equ	13
ciacra		equ	14
ciacrb		equ	15

;not latched pra
;not latched prb
;not latched ddra
;not latched ddrb

TBCNT			equ	16
TBCTA			equ	17
TBPHI2		equ	18
	
timerAlread		equ	20
timerAhread		equ	21
timerBlread		equ	22
timerBhread		equ	23
TODtensread		equ	24
TODsecsread		equ	25
TODminsread		equ	26
TODhourread		equ	27
ciasdrread		equ	28
ciaicrmask		equ	29

;not latched cra
;not latched crb

TAlatch		equ	32
TAcount		equ	36
TBlatch		equ	40
TBcount		equ	44
;ciaicrmask		equ	48

TAPHI2		equ	48
TACNT			equ	49
TApulse		equ	51
TBpulse		equ	52

TODstart		equ	53
TODupdates		equ	54
	
TODtensalarm	equ	56
TODsecsalarm	equ	57
TODminsalarm	equ	58
TODhoursalarm	equ	59
TODtimer		equ	60

STARTT		equ	0x01
PBON			equ	0x02
OUTMODE		equ	0x04
RUNMODE		equ	0x08
FORCE			equ	0x10
INMODEA		equ	0x20
SPMODE		equ	0x40
TODALARM			equ	0x80
	
OUTPRB		equ	0x02
TOGGLE		equ	0x04
PRB6			equ	0x40
PRB7			equ	0x80
	
TAINT			equ	0x01
TBINT			equ	0x02
TODINT		equ	0x04
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

;===================================================================================
;===================================================================================

	BITS 32
	GLOBAL 	_CIA1
	GLOBAL 	_Reset6526
	GLOBAL 	readCIA1
	GLOBAL 	readCIA2
	GLOBAL 	writeCIA1
	GLOBAL 	writeCIA2
	GLOBAL 	_CIACHIP1
	GLOBAL 	_CIACHIP2
	GLOBAL 	_keyarr
	GLOBAL 	_invkeyarr

	EXTERN	_cpu
	EXTERN 	_ciatimer
	EXTERN 	_joy1
	EXTERN 	_joy2
	EXTERN	_UpdateVIC2
		
;-----------------------------------------------------------------------------------
%include "include\ciadata.inc"
;-----------------------------------------------------------------------------------
;-----------------------------------------------------------------------------------
[SECTION .text]
;lookup readVIC2 4 mechanics of read/write tables
	ALIGN 32
readCIA1:
	xor	ebx,ebx
	mov	esi,_CIACHIP1
	sub	eax,CIA2base
	and	eax,0x0f
	jmp	[eax*4+CIA1readtable]
;-----------------------------------------------------------------------------------
;lookup readVIC2 4 mechanics of read/write tables
	ALIGN 32
readCIA2:
	xor	ebx,ebx
	mov	esi,_CIACHIP2
	sub	eax,CIA2base
	and	eax,0x0f
	jmp	[eax*4+CIA2readtable]
	nop
	nop
;-----------------------------------------------------------------------------------
; CIA1 specific functions
	ALIGN 32
readCIA1pra
	push	edx
	push	ecx
	mov	al,[esi+ciapra]
	mov	bl,[esi+ciaprb]
	mov	ah,[esi+ciaddra]
	mov	bh,[esi+ciaddrb]
	not	ah
	not	bh
	or	al,ah				;val=pra|~ddra
	or	bl,bh				;tst=prb|~ddrb
	and	bl,[_joy1]			;tst&=joy1
	xor	edx,edx
	mov	cl,0x01			;count=1
testpra
	test	bl,cl				;if(!(tst&count))
	jnz	testpra2
	mov	bh,[edx+_invkeyarr]
;	not	bh
	and	al,bh				;val&=~invkeyarr
testpra2
	inc	dl
	shl	cl,1
	jnz	testpra			;while(count!=0)
		
	mov	edx,0xff
	mov	cl,[_joy2]
	mov	bl,al				;bl=val
	and	bl,cl				;bl=val&joy2
	and	ebx,edx
	and	eax,edx
	pop	ecx
	pop	edx
	ret
	ALIGN 32
readCIA1prb
	push	edx
	push	ecx
	mov	edx,[_joy2]
	mov	al,[esi+ciaddrb]
	mov	bl,[esi+ciapra]
	mov	bh,[esi+ciaddra]
	not	al				;val=~ddrb
	not	bh
	or	bl,bh				;tst=pra|~ddra
	and	bl,dl				;tst&=joy2
	xor	edx,edx
	mov	cl,0x01
testprb
	test	bl,cl
	jnz	testprb2
	mov	bh,[edx+_keyarr]
;	not	bh
	and	al,bh
testprb2
	inc	dl
	shl	cl,1
	jnz	testprb
	mov	ebx,[_joy1]
	mov	dl,[esi+ciaprb]
	mov	cl,[esi+ciaddrb]
	and	cl,dl
	or	al,cl
	and	bl,al
	pop	ecx
	pop	edx
	ret
	ALIGN 32
readCIA1icr
	xor	ebx,ebx
	xor	eax,eax
	mov	bl,[esi+ciaicr]
	mov	[esi+ciaicr],al
	mov	[edi+ciairq],al
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
; CIA2 specific functions
	ALIGN 32
readCIA2pra
	push	ecx
	mov	bl,[esi+ciapra]
	mov	cl,[esi+ciaddra]
	not	cl
	or	bl,cl
	and	bl,0x3f
	pop	ecx
	ret
	ALIGN 32
readCIA2prb
	push	ecx
	mov	bl,[esi+ciaprb]
	mov	cl,[esi+ciaddrb]
	not	cl
	or	bl,cl
	pop	ecx
	ret
	ALIGN 32
readCIA2icr
	xor	ebx,ebx
	xor	eax,eax
	mov	bl,[esi+ciaicr]
	mov	[esi+ciaicr],al
	mov	[edi+nmi],eax
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
readCIAddra
readCIAddrb
	mov	bl,[esi+eax]
	ret
	ALIGN 32
readCIAtal
	mov	ebx,[esi+timerAlread]
	and	ebx,0xff
	ret
	ALIGN 32
readCIAtah
	mov	ebx,[esi+timerAhread]
;	shr	ebx,8
	and	ebx,0xff
	ret
	ALIGN 32
readCIAtbl
	mov	bl,[esi+timerBlread]
	ret
	ALIGN 32
readCIAtbh
	mov	bl,[esi+timerBhread]
	ret
	ALIGN 32
readCIATODtens
	mov	bl,[esi+TODtensread]
	mov	byte[esi+TODupdates],0x00
	ret
	ALIGN 32
readCIATODsecs
	mov	bl,[esi+TODsecsread]
	ret
	ALIGN 32
readCIATODmins
	mov	bl,[esi+TODminsread]
	ret
	ALIGN 32
readCIATODhour
	mov	bl,[esi+TODhourread]
	mov	byte[esi+TODupdates],0xff
	ret

;-----------------------------------------------------------------------------------
;lookup readVIC2 4 mechanics of read/write tables
	ALIGN 32
writeCIA1:
	mov		eax,ebp
	mov		esi,_CIACHIP1
	and		eax,0x0f
	jmp		[eax*4+CIA1writetable]
;-----------------------------------------------------------------------------------
;lookup readVIC2 4 mechanics of read/write tables
	ALIGN 32
writeCIA2:
	mov		eax,ebp
	mov		esi,_CIACHIP2
	and		eax,0x0f
	jmp		[eax*4+CIA2writetable]
	nop
	nop
	nop
	ALIGN 32
;-----------------------------------------------------------------------------------		

write2CIA2pra
	mov		[esi+eax],bl
	call		_UpdateVIC2
	ret
	ALIGN 32
write2CIA1pra
write2CIAprb
write2CIAddra
write2CIAddrb
write2CIAsdr
	mov		[esi+eax],bl
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------		
write2TAhigh
	mov		[esi+eax],bl
	mov		ebx,[esi+timerAlwrite]
	and		ebx,0xffff
	mov		[esi+TAlatch],ebx
	mov		al,[esi+ciacra]
	test		al,STARTT
	jnz		.byetahigh
	mov		[esi+TAcount],ebx
	mov		[esi+timerAlread],bl
	mov		[esi+timerAhread],bh
.byetahigh
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------		
write2TBhigh
	mov		[esi+eax],bl
	mov		ebx,[esi+timerBlwrite]
	and		ebx,0xffff
	mov		[esi+TBlatch],ebx
	mov		al,[esi+ciacrb]
	test		al,STARTT
	jnz		.byetbhigh
	mov		[esi+TBcount],ebx
	mov		[esi+timerBlread],bl
	mov		[esi+timerBhread],bh
.byetbhigh
	ret	
	ALIGN 32
;-----------------------------------------------------------------------------------		
write2TODtens:
	and		bl,0x0f
	mov		eax,[TODperiod]
	mov		byte[esi+TODstart],0xff		;turn on TOD
	mov		[esi+TODtimer],eax		;reset counter
	test		byte[esi+ciacrb],TODALARM
	jnz		near .writeon1
	mov		[esi+TODtenswrite],bl
	ret
	ALIGN 32
.writeon1
	mov		[esi+TODtensalarm],bl
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
write2TODsecs
	and		bl,0x7f
	test		byte[esi+ciacrb],TODALARM
	jnz		.writeon2
	mov		[esi+eax],bl
	ret
	ALIGN 32
.writeon2
	mov		[esi+TODsecsalarm],bl
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------		
write2TODmins
	and		bl,0x7f
	test		byte[esi+ciacrb],TODALARM
	jnz		.writeon3
	mov		[esi+eax],bl
	ret
	ALIGN 32
.writeon3
	mov		[esi+TODminsalarm],bl
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
write2TODhours:
	and		bl,0x9f
	mov		eax,[TODperiod]
	mov		byte[esi+TODstart],0x00		;turn off TOD
	mov		[esi+TODtimer],eax		;reset counter
	test		byte[esi+ciacrb],TODALARM
	jnz 		near .writeon4
	mov		[esi+TODhourwrite],bl
	ret
	ALIGN 32
.writeon4
	mov		[esi+TODhoursalarm],bl
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------		
write2cia1icr
	xor		eax,eax
	mov		bh,bl
	mov		al,[esi+ciaicrmask]
	and		al,0x1f
	test		byte[edi+RMW],0xff
	jz		cia10
	mov		[esi+ciaicr],ah
	mov		[edi+ciairq],ah
cia10
	test		bh,0x80
	jz		cia12
	or		al,bl
	jmp		cia13
cia12
	not		bl
	and		al,bl
cia13	
	and		al,0x1f
	mov		[esi+ciaicrmask],al
	ret
	ALIGN 32
write2cia2icr
	xor		eax,eax
	mov		bh,bl
	mov		al,[esi+ciaicrmask]
	and		al,0x1f
	test		byte[edi+RMW],0xff
	jz		cia20
	mov		[esi+ciaicr],ah
	mov		[edi+nmi],ah
cia20
	test		bh,0x80
	jz		cia22
	or		al,bl
	jmp		cia23
cia22
	not		bl
	and		al,bl
cia23
	and		al,0x1f
	mov		[esi+ciaicrmask],al
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------		
write2ciacra:
	test		bl,FORCE
	jz		.byecra
	mov		eax,[esi+timerAlwrite]
	and		bl,(0xff-FORCE)			;kill force bit
	and		eax,0xffff
	mov		[esi+TAlatch],eax
	mov		[esi+TAcount],eax
.byecra
	mov		[esi+ciacra],bl
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------		
write2ciacrb:
	test		bl,FORCE
	jz		.byecrb
	mov		eax,[esi+timerBlwrite]
	and		bl,(0xff-FORCE)			;kill force bit
	and		eax,0xffff
	mov		[esi+TBlatch],eax
	mov		[esi+TBcount],eax
.byecrb
	mov		[esi+ciacrb],bl
	and		bl,32+64
	jnz		tbta
	mov		byte[esi+TBPHI2],0xff
	jmp		ciacrbout
tbta
	cmp		bl,0x40
	jnz		ciacrbout
	mov		byte[esi+TBCTA],0xff
ciacrbout
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
; CIA 1 : Timer A, Timer B
	ALIGN 32
_CIA1
	mov		esi,_CIACHIP1
	mov		edi,_cpu
;-----------------------------------------------------------------------------------
;This of code is rather complex look at the C version explanation
WorkonTA1
	mov		cl,[esi+ciacra]			;copy cra to cl for speed
	mov		dl,[esi+ciaicrmask]		;ditto
	mov		ebx,[esi+TAcount]			;timer A counter
	
	test		cl,STARTT				;is timer A running?
	jz		near WorkonTB1			;nope - work on TimerB
	
	sub		ebx,[_ciatimer]			;decrement timer A
	jnc		near UpdateTA1			;update timer A
;-----------------------------------------------------------------------------------
ToggleTA1
;do we need to pulse or toggle?
	xor		eax,eax
	test		cl,OUTPRB				;do we need to output to prb?
	jz 		near SortTA1
	and		byte[esi+ciaddrb],0xbf		;yep - force bit 6 ddrb to output mode
	test		cl,TOGGLE				;pulse or toggle?
	jz		pulseTA1
	xor		byte[esi+ciaprb],PRB6		;toggle
	jmp		SortTA1
pulseTA1
	or		byte[esi+ciaprb],PRB6		;pulse
	mov		byte[esi+TApulse],0xff
;-----------------------------------------------------------------------------------
SortTA1
	mov		ebx,[esi+TAlatch]			;load latch
	and		ebx,0xffff				;insurance
	mov		[esi+TAcount],ebx
	test		cl,RUNMODE				;are we in one-shot mode?
	jz		TA1ints
	and		cl,(0xff-STARTT)			;yes-stop timer A
	mov		[esi+TAPHI2],al			;kill PHI2 
	mov		[esi+TACNT],al			;kill CNT
TA1ints	
	or		byte[esi+ciaicr],TAINT		;set Timer A interupt bit 1
	test		dl,TAINT				;is int enable for TA?
	jz		AdjustTA1				;nope so just updateTA1
	mov		byte[edi+ciairq],0xff
	or		byte[esi+ciaicr],0x80		;yep-generate int bit
;-----------------------------------------------------------------------------------
AdjustTA1
	mov		[esi+TAcount],ebx			;updating timer A counter.
	mov		[esi+timerAlread],bl
	mov		[esi+timerAhread],bh
	
	test		byte[esi+TBCTA],0xff			;tb count ta?
	jz		near WorkonTB1
	
	mov		cl,[esi+ciacrb]			;copycrb to cl for speed
	mov		ebx,[esi+TBcount]
	sub		ebx,0x1
	jnc		near UpdateTB1
	jmp		SortTB1
	
UpdateTA1
	mov		[esi+TAcount],ebx			;updating timer A counter.
	mov		[esi+timerAlread],bl
	mov		[esi+timerAhread],bh
;-----------------------------------------------------------------------------------
WorkonTB1
	test		byte[esi+TBPHI2],0xff
	jz		near TODA				;phi2 counting
	
	mov		cl,[esi+ciacrb]			;copycrb to cl for speed
	mov		ebx,[esi+TBcount]

	sub		ebx,[_ciatimer]			;decrement timer B
	jnc		near UpdateTB1			;if (timera>0) goto update
;-----------------------------------------------------------------------------------
SortTB1
	xor		eax,eax
	mov		ebx,[esi+TBlatch]			;load latch
	and		ebx,0xffff
	mov		[esi+TBcount],ebx
	test		cl,RUNMODE				;are we in one-shot mode?
	jz		TB1ints
	and		cl,(0xff-STARTT)			;yes-stop timer A
	mov		[esi+TBPHI2],al
	mov		[esi+TBCTA],al
	mov		[esi+TBCNT],al
TB1ints
	or		byte[esi+ciaicr],TBINT		;set Timer B interupt bit 2
	test		dl,TBINT				;is int enable for TB?
	jz		ToggleTB1				;nope so just updateTA1
	mov		byte[edi+ciairq],0xff
	or		byte[esi+ciaicr],0x80		;yep-generate int bit
;-----------------------------------------------------------------------------------
ToggleTB1
;do we need to pulse or toggle?
	test		cl,PRB7				;do we need to output to prb?
	jz 		near UpdateTB1
	and		byte[esi+ciaddrb],0x7f		;yep - force bit 6 ddrb to output mode
	test		cl,TOGGLE				;pulse or toggle?
	jz		pulseTB1
	xor		byte[esi+ciaprb],0x80		;toggle
	jmp		UpdateTB1
pulseTB1
	or		byte[esi+ciaprb],0x80		;pulse
UpdateTB1
	mov		[esi+TBcount],ebx
	mov		[esi+timerBlread],bl
	mov		[esi+timerBhread],bh
;-----------------------------------------------------------------------------------
;Time Of Day timer
TODA
	xor		edx,edx
	
	test		byte[esi+TODstart],0xff		;is TOD running?
	jz		near _CIA2
	
	mov		ebx,[esi+TODtimer]		;cycles to next update
	dec		ebx
	jnz 		near	finishTODA
	mov		ecx,[esi+TODtensalarm]		;get alarm time
;-----------------------------------------------------------------------------------
;calculate tenths of seconds
	mov		al,[esi+TODtenswrite]		;tenths digits
	inc		al
	mov		[esi+TODtenswrite],al		;store count
	
	cmp		al,10					;is tenths up to 10?
	jb 		near	xoutTOD1
	mov		[esi+TODtenswrite],dl		;clear tenths count
	
;-----------------------------------------------------------------------------------
;calculate seconds
	mov		al,[esi+TODsecswrite]
	inc		al
	daa							;decimal adjust seconds
	mov		[esi+TODsecswrite],al		;store count
	
	cmp		al,0x60				;is secs>=60?
	jb 		near xoutTOD1
	mov		[esi+TODsecswrite],dl		;yep clear secs count
;-----------------------------------------------------------------------------------
;calculate minutes
	mov		al,[esi+TODminswrite]
	inc		al
	daa
	mov		[esi+TODminswrite],al
	
	cmp		al,0x60				;is mins>=60?
	jb		xoutTOD1
	mov		[esi+TODminswrite],dl		;reset mins to zero
;-----------------------------------------------------------------------------------
;calculate hours
	mov		al,[esi+TODhourwrite]
	and		al,0x1f
	inc		al
	daa
	cmp		al,13
	jb		.sorthours
	mov		al,1					;hours back to one
.sorthours
	mov		ah,[esi+TODhourwrite]
	and		ah,0x80
	xor		al,ah
	and		al,0x9f				;insurance <shrugs>
	mov		[esi+TODhourwrite],al		;save count
;-----------------------------------------------------------------------------------
xoutTOD1
	mov		eax,[esi+TODtenswrite]		;get count
	test		byte[esi+TODupdates],0xff	;does tod update readout?
	jnz		.finishit
	mov		[esi+TODtensread],eax		;update readout
.finishit
	cmp		ecx,eax				;have tod hit alarm time?
	jnz		NoTOD1alarm
	
	mov		dl,[esi+ciaicrmask]
	or		byte[esi+ciaicr],TODINT
	test		dl,TODINT
	jz		NoTOD1alarm
	or		byte[esi+ciaicr],0x80
	mov		byte[edi+ciairq],0xff
;-----------------------------------------------------------------------------------
NoTOD1alarm
	mov		ebx,[TODperiod]			;reload period
finishTODA
	mov		[esi+TODtimer],ebx
;-----------------------------------------------------------------------------------
;-----------------------------------------------------------------------------------
_CIA2
	mov		esi,_CIACHIP2
	mov		edi,_cpu
;-----------------------------------------------------------------------------------
;This of code is rather complex look at the C version explanation
WorkonTA2
	mov		cl,[esi+ciacra]			;copy cra to cl for speed
	mov		dl,[esi+ciaicrmask]		;ditto
	mov		ebx,[esi+TAcount]			;timer A counter
	
	test		cl,STARTT				;is timer A running?
	jz		near WorkonTB2			;nope - work on TimerB
	sub		ebx,[_ciatimer]			;decrement timer A
	jnc		near UpdateTA2			;update timer A
;-----------------------------------------------------------------------------------
ToggleTA2
;do we need to pulse or toggle?
	xor		eax,eax
	test		cl,OUTPRB				;do we need to output to prb?
	jz 		near SortTA2
	and		byte[esi+ciaddrb],0xbf		;yep - force bit 6 ddrb to output mode
	test		cl,TOGGLE				;pulse or toggle?
	jz		pulseTA2
	xor		byte[esi+ciaprb],PRB6		;toggle
	jmp		SortTA2
pulseTA2
	or		byte[esi+ciaprb],PRB6		;pulse
	mov		byte[esi+TApulse],0xff
;-----------------------------------------------------------------------------------
SortTA2
	mov		ebx,[esi+TAlatch]			;load latch
	and		ebx,0xffff				;insurance
	mov		[esi+TAcount],ebx
	test		cl,RUNMODE				;are we in one-shot mode?
	jz		TA2ints
	and		cl,(0xff-STARTT)			;yes-stop timer A
	mov		[esi+TAPHI2],al			;kill PHI2 
	mov		[esi+TACNT],al			;kill CNT
TA2ints	
	or		byte[esi+ciaicr],TAINT		;set Timer A interupt bit 1
	test		dl,TAINT				;is int enable for TA?
	jz		AdjustTA2				;nope so just updateTA1
	mov		byte[edi+nmi],0xff
	or		byte[esi+ciaicr],0x80		;yep-generate int bit
;-----------------------------------------------------------------------------------
AdjustTA2
	mov		[esi+TAcount],ebx			;updating timer A counter.
	mov		[esi+timerAlread],bl
	mov		[esi+timerAhread],bh
	test		byte[esi+TBCTA],0xff		;tb count ta?
	jz		near WorkonTB2
	mov		cl,[esi+ciacrb]			;copycrb to cl for speed
	mov		ebx,[esi+TBcount]
	sub		ebx,0x1
	jnc		near UpdateTB2
	jmp		SortTB2
UpdateTA2
	mov		[esi+TAcount],ebx			;updating timer A counter again (sighs!).
	mov		[esi+timerAlread],bl
	mov		[esi+timerAhread],bh
	
;-----------------------------------------------------------------------------------
;CIA2 TIMER B
WorkonTB2
	test		byte[esi+TBPHI2],0xff
	jz		near TODB				;phi2 counting

	mov		cl,[esi+ciacrb]			;copycrb to cl for speed
	mov		ebx,[esi+TBcount]
	sub		ebx,[_ciatimer]			;decrement timer B
	jnc		near UpdateTB2			;if (timera>0) goto update
;-----------------------------------------------------------------------------------
SortTB2
	xor		eax,eax
	mov		ebx,[esi+TBlatch]			;load latch
	and		ebx,0xffff
	mov		[esi+TBcount],ebx
	test		cl,RUNMODE				;are we in one-shot mode?
	jz		TB2ints
	and		cl,(0xff-STARTT)			;yes-stop timer A
	mov		[esi+TBPHI2],al
	mov		[esi+TBCTA],al
	mov		[esi+TBCNT],al
TB2ints
	or		byte[esi+ciaicr],TBINT		;set Timer B interupt bit 2
	test		dl,TBINT				;is int enable for TB?
	jz		ToggleTB2				;nope so just updateTA1
	mov		byte[edi+nmi],0xff
	or		byte[esi+ciaicr],0x80		;yep-generate int bit
;-----------------------------------------------------------------------------------
ToggleTB2
;do we need to pulse or toggle?
	test		cl,PRB7				;do we need to output to prb?
	jz 		near UpdateTB2
	and		byte[esi+ciaddrb],0x7f		;yep - force bit 6 ddrb to output mode
	test		cl,TOGGLE				;pulse or toggle?
	jz		pulseTB2
	xor		byte[esi+ciaprb],0x80		;toggle
	jmp		UpdateTB2
pulseTB2
	or		byte[esi+ciaprb],0x80		;pulse
UpdateTB2
	mov		[esi+TBcount],ebx
	mov		[esi+timerBlread],bl
	mov		[esi+timerBhread],bh
;-----------------------------------------------------------------------------------
;Time Of Day timer
TODB
	xor		edx,edx
	
	test		byte[esi+TODstart],0xff		;is TOD running?
	jz		near finishCIAs
	
	mov		ebx,[esi+TODtimer]		;cycles to next update
	dec		ebx
	jnz 		near	finishTODB
	mov		ecx,[esi+TODtensalarm]		;get alarm time
;-----------------------------------------------------------------------------------
;calculate tenths of seconds
	mov		al,[esi+TODtenswrite]		;tenths digits
	inc		al
	mov		[esi+TODtenswrite],al		;store count
	
	cmp		al,10						;is tenths up to 10?
	jb 		near	xoutTOD2
	mov		[esi+TODtenswrite],dl		;clear tenths count
	
;-----------------------------------------------------------------------------------
;calculate seconds
	mov		al,[esi+TODsecswrite]
	inc		al
	daa							;decimal adjust seconds
	mov		[esi+TODsecswrite],al		;store count
	
	cmp		al,0x60				;is secs>=60?
	jb 		near xoutTOD2
	mov		[esi+TODsecswrite],dl		;yep clear secs count
;-----------------------------------------------------------------------------------
;calculate minutes
	mov		al,[esi+TODminswrite]
	inc		al
	daa
	mov		[esi+TODminswrite],al
	
	cmp		al,0x60				;is mins>=60?
	jb		xoutTOD2
	mov		[esi+TODminswrite],dl		;reset mins to zero
;-----------------------------------------------------------------------------------
;calculate hours
	mov		al,[esi+TODhourwrite]
	and		al,0x1f
	inc		al
	daa
	cmp		al,13
	jb		.sorthours
	mov		al,1					;hours back to one
.sorthours
	mov		ah,[esi+TODhourwrite]
	and		ah,0x80
	xor		al,ah
	and		al,0x9f				;insurance <shrugs>
	mov		[esi+TODhourwrite],al		;save count
;-----------------------------------------------------------------------------------
xoutTOD2
	mov		eax,[esi+TODtenswrite]		;get count
	test		byte[esi+TODupdates],0xff	;does tod update readout?
	jnz		.finishit
	mov		[esi+TODtensread],eax		;update readout
.finishit
	cmp		ecx,eax				;have tod hit alarm time?
	jnz		NoTOD2alarm
	
	mov		dl,[esi+ciaicrmask]
	or		byte[esi+ciaicr],TODINT
	test		dl,TODINT
	jz		NoTOD2alarm
	or		byte[esi+ciaicr],0x80
	mov		byte[edi+nmi],0xff
;-----------------------------------------------------------------------------------
NoTOD2alarm
	mov		ebx,[TODperiod]			;reload period
finishTODB
	mov		[esi+TODtimer],ebx
finishCIAs
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
;-----------------------------------------------------------------------------------		
FinishPulseTA1
	and	byte[esi+ciaprb],0xbf
	and	byte[esi+ciaddrb],0xbf
	mov	byte[esi+TApulse],0x00
	jmp	WorkonTA1
	ALIGN 32
;-----------------------------------------------------------------------------------
FinishPulseTB1
	and	byte[esi+ciaprb],0x7f
	and	byte[esi+ciaddrb],0x7f
	mov	byte[esi+TBpulse],0x00
	jmp	WorkonTB1
	ALIGN 32
;-----------------------------------------------------------------------------------
FinishPulseTA2
	and	byte[esi+ciaprb],0xbf
	and	byte[esi+ciaddrb],0xbf
	mov	byte[esi+TApulse],0x00
	jmp	WorkonTA2
	ALIGN 32
;-----------------------------------------------------------------------------------
FinishPulseTB2
	and	byte[esi+ciaprb],0x7f
	and	byte[esi+ciaddrb],0x7f
	mov	byte[esi+TBpulse],0x00
	jmp	WorkonTB2
	ALIGN 32
;-----------------------------------------------------------------------------------
_Reset6526
	xor	eax,eax
	mov	ebx,0xffff
	mov	edi,_CIACHIP1
	mov	esi,_CIACHIP2
	mov	[edi],al
	mov	[esi],al
	mov	ecx,0x3f
clrcia:
	mov	[esi+ecx],al
	mov	[edi+ecx],al
	loop	clrcia
	mov	[esi+TAlatch],ebx
	mov	[esi+TBlatch],ebx
	mov	[edi+TAlatch],ebx
	mov	[edi+TBlatch],ebx
	ret
;===================================================================================