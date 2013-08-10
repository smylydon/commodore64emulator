
;===================================================================================
;===================================================================================

	BITS 32
	GLOBAL	_M6510			;start of program
	GLOBAL	_Reset6510			;reset cpu

	GLOBAL	_mnspeek			;memory routines
	GLOBAL	_mnsdeek
	GLOBAL	_mnspoke
	GLOBAL	_mnsdoke
	GLOBAL	_mnsspeek
	GLOBAL	_mnsspoke
	GLOBAL	_cpu				;cpu
	GLOBAL	_PixelRom			;character rom

	GLOBAL	_SIDCHIP			;SIDCHIP
	GLOBAL	_RAM				;Ram

	GLOBAL	_mnsvline			;rasterline

	GLOBAL	_cputimer			;timers
	GLOBAL	_ciatimer
	GLOBAL	_victimer
	GLOBAL	_rasterlatch
	GLOBAL	_myfont
	GLOBAL	_gatez
	GLOBAL	_WriteFM			;FM Poke	
	EXTERN	readVIC2
	EXTERN	writeVIC2
		
	EXTERN 	readCIA1
	EXTERN 	readCIA2
	EXTERN	writeCIA1
	EXTERN 	writeCIA2	
	EXTERN	_COLORRAM
	EXTERN 	_PokeSID
	EXTERN 	_SIDPoke
	EXTERN 	_SIDAddr

		
%include "include\data.inc"
;-----------------------------------------------------------------------------------
[SECTION .text]
%include "include\opcodes.inc"
;===================================================================================

	ALIGN 32
_M6510:
	mov	edi,_cpu			;edi=cpu structure
	mov	cl,[edi+areg]
	mov	ch,[edi+psr] 
	mov	edx,[edi+ppc]
	mov	esi,memory			;esi=memory
;-----------------------------------------------------------------------------------
; eax=opcode+byte
;-----------------------------------------------------------------------------------
mainloop
	xor	ebx,ebx
	and	edx,0xffff
	mov	byte[edi+RMW],0x00
		
	test	dword[edi+nmi],-1
	jnz near M6502NMI
	test	dword[edi+irq],-1
	jnz near	M6502IRQ
noIRQ
	mov	eax,edx
	call	readmem
	mov	eax,[esi+eax]		;get mnemonic
	mov	dword[edi+ppc2],edx
	mov	bl,al
	mov	esi,memory			;esi=memory, insurance
	shr	eax,8
	and	eax,0xffff			;insurance against cock up
	jmp	[ebx*4+optable]
	ALIGN 32
EXIT6510
	xor	eax,eax
	mov	[edi+cycles],eax
	and	edx,0xffff
	mov	[edi+areg],cl
	mov	[edi+psr],ch
	mov	[edi+ppc],edx
	ret
;-----------------------------------------------------------------------------------
; C64 Interupt Handlers
;-----------------------------------------------------------------------------------
	ALIGN 32
M6502IRQ
	test	ch,INTERUPT
	jnz near	noIRQ

	xor	ebx,ebx
	mov	dword[edi+ppc2],edx
	and	ch,(0xff-BREAK)
	mov	eax,[edi+psp]

	mov	[esi+eax],dh		;place high byte
	dec	al

	mov	[esi+eax],dl		;place low byte
	dec	al
	or	ch,UNDEFINED
	mov	[esi+eax],ch
	dec	al

	or	ch,INTERUPT

	mov	[edi+psp],eax
	mov	eax,0xfffe
	call	readmem
	mov	edx,[esi+eax]
	finito 7
;-----------------------------------------------------------------------------------
M6502NMI
	xor	ebx,ebx
	mov	dword[edi+ppc2],edx
	and	ch,(0xff-BREAK)

	mov	[edi+nmi],ebx		;only cpu can kill this baby
	mov	eax,[edi+psp]

	mov	[esi+eax],dh		;place high byte
	dec	al

	mov	[esi+eax],dl		;place low byte
	dec	al
	or	ch,UNDEFINED
	mov	[esi+eax],ch
	dec	al

	or	ch,INTERUPT

	mov	[edi+psp],eax

	mov	eax,0xfffa
	call	readmem
	mov	edx,[esi+eax]
	finito 7
	ALIGN 32
;===================================================================================
;readmem - eax must contain the address
;        - bl=return value
	
readmem
	mov	esi,memory
	and	eax,0xffff
	mov	bl,[esi+1]
	mov	bh,bl
	and	bh,3
	jz near readRAM			;64k ram mode....

	cmp	eax,0x0a000
	jb	readRAM			;read from ram

	cmp	eax,0x0e000
	jae near readKERNAL

	cmp	eax,0x0c000
	jae near readCUSTOM		;custom read? 

	cmp	bh,3
	jz	near readBASIC		;basic has been kill
readRAM
	xor	ebx,ebx
	mov	bl,[esi+eax]
	ret
	ALIGN 32
readBASIC
	xor	ebx,ebx
	add	eax,basicstart-(memory+0x0a000)
	mov	bl,[esi+eax]
	and	ebx,0xff
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
readKERNAL
	test	bl,HIRAMB
	jz	near readRAM			;default readram
	xor	ebx,ebx
	add	eax,kernalstart-(memory+0x0e000)
	mov	bl,[esi+eax]
	ret
	ALIGN 32
;-----------------------------------------------------------------------------------
readCUSTOM
	cmp	eax,0x0d000			;remember the 4k after basic?
	jb near readRAM
	and	bl,7
	cmp	bl,4
	jb near readCHARROM		;Char Rom is banked in

	mov	bl,ah
	and	ebx,0xf
	jmp	[ebx*4+readtable]
	ALIGN 32
readCHARROM
	xor	ebx,ebx
	add	eax,charstart-(memory+0xd000)
	mov	bl,[esi+eax]
	and	bl,0xff
	ret
;-----------------------------------------------------------------------------------
	ALIGN 32
readSID
	sub	eax,SIDbase			;get offset
	and	eax,0x1f			
	mov	eax,[eax*4+SIDreadtable]
	xor	ebx,ebx
	cmp	al,0x1b
	jz	near .try2
	mov	bl,[esi+eax]
	ret
	ALIGN 32
.try2
	mov	bl,[edi+clk]
	and	bl,[edi+cycles]
	ret
;-----------------------------------------------------------------------------------
	ALIGN 32
readCOLORRAM
	mov	esi,_COLORRAM
	and	eax,0x3ff
	xor	ebx,ebx
	mov	bl,[esi+eax]
	or	bl,0xf0
	ret
;===================================================================================
;custom chips located @ $d000-dfff, any writes outside of this range fall to ram
;NOTE eax is trashed
	ALIGN 32
writemem
	mov	esi,memory
	and	ebp,0xffff
	mov	al,[esi+1]
	mov	ah,al
	and	ah,3
	jz	writeRAM			;64k ram mode..

	cmp	ebp,0x0d000
	jb	writeRAM			;writing 2 ram anywayz

	cmp	ebp,0x0e000
	jb near writeCUSTOM		;can't write 2 kernal either!!
writeRAM
	mov	[esi+ebp],bl
	ret
	ALIGN 32
writeCUSTOM
	test	al,CHARENB
	jz near writeRAM			;char is mapped in so bounce

	mov	eax,ebp
	shr	eax,8
	and	eax,0x0f
	jmp	[eax*4+writetable]
	nop
	nop
	nop
	nop
;-----------------------------------------------------------------------------------
;-----------------------------------------------------------------------------------
;-----------------------------------------------------------------------------------
	ALIGN 32
writeSID
	pushad
	mov	esi,_SIDCHIP
	mov	edx,ebp
	mov	eax,ebx
	and	edx,0x1f
	and	eax,0xff

	mov	[_SIDAddr],edx
	mov	[_SIDPoke],eax
	and	ebp,0x1f			;mirror mirror on the wall :-)
;	mov	[esi+ebp],bl
	call	_PokeSID
	popad
	ret	
	
;-----------------------------------------------------------------------------------
	ALIGN 32
writeCOLORRAM
	and	ebp,0x3ff
	mov	esi,_COLORRAM
;	mov	eax,ebp
;	add	eax,_COLORRAM-(memory+COLORRAMbase)
	mov	[esi+ebp],bl
	ret
;-----------------------------------------------------------------------------------
;===================================================================================
;GLOBAL void Reset()
	ALIGN 32
_Reset6510
	mov	edi,_cpu
	mov	eax,memory
	mov	esi,eax
	xor	ebx,ebx
	mov	bx,[eax+(0x1ffc+kernalstart-memory)]
	add	dword[edi+clk],7
	mov	dword[edi+cycles],7
	add	dword[edi+inst],1
	mov	dword[edi+ppc],ebx
	mov	dword[edi+ppc2],ebx
	mov	dword[edi+psr],0x20		;force undifined bit hi
	mov	dword[edi+psp],0x01ff		;undefined on reset, but needed the 1 in hi byte
	xor	eax,eax
	mov	ecx,0xffff
clrram:
	mov	[esi+ecx],al
	loop	clrram
	mov	byte[esi],0xef
	mov	byte[esi+1],0xef
	ret

;===================================================================================
;-----------------------------------------------------------------------------------
; GLOBAL char peek(unsigned int)
	ALIGN 32
_mnspeek: 
	push	ebp
	mov	ebp,esp
	mov	eax,[ebp+8]
		
	mov	esi,memory			;esi=memory
	mov	edi,_cpu			;edi=cpu structure
	mov	cl,[edi+areg]
	mov	ch,[edi+psr] 
	mov	edx,[edi+ppc]
	push	ebp
		
	and	eax,0xffff			;truncate 2 16bits, incase of stoopidity
	mov	ebp,eax
	call	readmem
	mov	eax,ebx
	and	eax,0xff
	pop	ebp
	mov	esp,ebp
	pop	ebp	
	ret
;-----------------------------------------------------------------------------------
; GLOBAL char speek(unsigned int)	;4 checking write latches
;	ALIGN 32
_mnsspeek: 
	push	ebp
	mov	ebp,esp
	mov	eax,[ebp+8]
	mov	esi,memory
		
	mov	edi,_cpu			;edi=cpu structure
	mov	cl,[edi+areg]
	mov	ch,[edi+psr] 
	mov	edx,[edi+ppc]
		
	and	eax,0xffff			;truncate 2 16bits, incase of stoopidity
	call	writemem
	mov	bl,[esi+eax]
	xchg	ebx,eax
	
	mov	esp,ebp
	pop	ebp	
	ret

;-----------------------------------------------------------------------------------
; GLOBAL word deek(dword)
;	ALIGN 32
_mnsdeek: 
	push	ebp
	mov	ebp,esp
	mov	eax,[ebp+8]
	mov	esi,memory
		
	mov	edi,_cpu			;edi=cpu structure
	mov	cl,[edi+areg]
	mov	ch,[edi+psr] 
	mov	edx,[edi+ppc]
		
	and	eax,0xffff			;no word alignment cause deek is non-distructive
	call	readmem
	mov	bx,[esi+eax]
	xchg	ebx,eax
	
	mov	esp,ebp
	pop	ebp	
	ret

;-----------------------------------------------------------------------------------
;GLOBAL void poke(dword,byte)
	ALIGN 32
_mnspoke: 
	push	ebp
	mov	ebp,esp

	mov	eax,[ebp+8]
	mov	ebx,[ebp+12]
	mov	esi,memory
	push	ebp		
	and	eax,0xffff	
	mov	ebp,eax
	and	ebx,0xff

	call	writemem
		
	pop	ebp
	mov	esp,ebp
	pop	ebp	
	ret
;-----------------------------------------------------------------------------------
;GLOBAL void spoke(dword,byte)	; WARNING!! writes 2 write latches
;	ALIGN 32
_mnsspoke: 
	push	ebp
	mov	ebp,esp
	push	ebp
		
	mov	eax,[ebp+8]
	mov	ebx,[ebp+12]
	mov	esi,memory
		
	mov	edi,_cpu			;edi=cpu structure
	mov	cl,[edi+areg]
	mov	ch,[edi+psr] 
	mov	edx,[edi+ppc]
		
	and	eax,0xffff			;truncation 2 16bits incase of accidents
	and	ebx,0x00ff

	call	readmem
	mov	[esi+eax],bl
	pop	ebp
	mov	esp,ebp
	pop	ebp	
	ret

;-----------------------------------------------------------------------------------
;GLOBAL void doke(dword,word)
;	ALIGN 32
_mnsdoke: 
	push	ebp
	mov	ebp,esp

	mov	eax,[ebp+8]
	mov	ebx,[ebp+12]
	mov	esi,memory
		
	mov	edi,_cpu			;edi=cpu structure
	mov	cl,[edi+areg]
	mov	ch,[edi+psr] 
	mov	edx,[edi+ppc]
		
	and	eax,0xfffe			;word ( even ) alignment - more idiot proofing
	and	ebx,0xffff			;poking 16bit digit now.

	call	writemem

	mov	esp,ebp
	pop	ebp	
	ret
	
;-----------------------------------------------------------------------------------
;GLOBAL void WriteFM(dword reg,dword val)	; WARNING!! writes 2 write latches
;	ALIGN 32
AdlibFM	EQU	0x388
	
_WriteFM: 
	push	ebp
	mov	ebp,esp

		
	mov dx,AdlibFM
;	mov ax,reg

	mov	eax,[ebp+8]
	mov	ebx,[ebp+12]
	and	eax,0xffff
	and	ebx,0xffff
	out dx,al
		
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	inc dx
		
;	mov	al,reg
	
	mov eax,ebx
		
	out dx,al
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	in al,dx
	
	mov	esp,ebp
	pop	ebp	
	ret
;===================================================================================

;-----------------------------------------------------------------------------------
;===================================================================================

