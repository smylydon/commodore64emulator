;===================================================================================
;-----------------------------------------------------------------------------------
%define	rmwsymbol		byte[edi+RMW]
	
%macro Readbyte 0
	call	readmem
%endmacro
%macro Writebyte 0
	call	writemem
%endmacro
%macro killRMWFlag 0
	mov	rmwsymbol,0x00
%endmacro
	
%macro RMWFlag 0
	mov	byte[edi+RMW],0xff
%endmacro

%macro ReadImm 0
	xor	ah,ah
	add	edx,2
	mov	ebx,eax
	mov	ebp,eax
%endmacro

%macro ReadZpg 0
	xor	ah,ah
	add	edx,2
	mov	bl,[esi+eax]
	mov	ebp,eax
%endmacro

%macro ReadZpgX 0
	xor	ah,ah
	add	edx,2
	add	al,[edi+xreg]
	mov	bl,[esi+eax]
	mov	ebp,eax
%endmacro

%macro ReadZpgY 0
	xor	ah,ah
	add	edx,2
	add	al,[edi+yreg]
	mov	bl,[esi+eax]
	mov	ebp,eax
%endmacro

%macro ReadAbs 0
	add	edx,3
	mov	ebp,eax
	Readbyte
%endmacro

%macro ReadAbsX 0
	mov	ebx,eax
	add   edx,3
	add	al,[edi+xreg]
	add	bl,[edi+xreg]
	adc	bh,0
	push	ebx
	cmp	bh,ah
	jz	%%absxnoxtra
	inc	dword[edi+cycles]
	inc	dword[edi+clk]
	mov	ebp,eax
	Readbyte				;Waste a cycle
%%absxnoxtra
	pop	eax
	mov	ebp,eax
	Readbyte
%endmacro

%macro ReadAbsY 0
	add	edx,3
	mov	ebx,eax			;cycles if boundary is crossed
	add	al,[edi+yreg]
	add	bl,[edi+yreg]
	adc	bh,0
	push	ebx
	cmp	bh,ah				;comment
	jz	%%absynoxtra		;if
	inc	dword[edi+cycles]
	inc	dword[edi+clk]
	mov	ebp,eax
	Readbyte
%%absynoxtra
	pop	eax
	mov	ebp,eax
	Readbyte
%endmacro

%macro ReadIndX 0
	xor	ah,ah
	add	edx,2
	add	al,[edi+xreg]			;zpg+x
	mov	bl,byte[esi+eax]
	inc	al
	mov	bh,byte[esi+eax]
	mov	eax,ebx
	mov	ebp,ebx
	Readbyte
%endmacro

%macro ReadIndY 0
	xor	ah,ah
	add	edx,2
	mov	bl,[esi+eax]		;(zpglo) get addr from zpg
	inc	al
	mov	bh,[esi+eax]		;(zpghi)
	mov	eax,ebx			;ax=(zpg)
	add	al,[edi+yreg]		;(zpg)+y addr+y
	add	bl,[edi+yreg]
	adc	bh,0
	push	ebx
	cmp	bh,ah
	jz	%%indynoxtra
	inc	dword[edi+cycles]
	inc	dword[edi+clk]
	mov	ebp,eax
	Readbyte
%%indynoxtra
	pop	eax
	mov	ebp,eax
	Readbyte
%endmacro
	
;-----------------------------------------------------------------------------------
	
%macro StoreZpg 0
	xor	ah,ah
	add	edx,2
	mov	ebp,eax
	mov	bl,[esi+eax]
%endmacro
	
%macro StoreZpgX 0
	xor	ah,ah
	add	edx,2
	add	al,[edi+xreg]
	mov	bl,[esi+eax]
	mov	ebp,eax
%endmacro

%macro StoreZpgY 0
	xor	ah,ah
	add	edx,2
	add	al,[edi+yreg]
	mov	ebp,eax
	mov	bl,[esi+eax]
%endmacro
	
%macro StoreAbs 0
	add	edx,3
	mov	ebp,eax
%endmacro
	
%macro StoreAbsX 0
	add   edx,3
	mov	ebx,eax
	add	al,[edi+xreg]
	add	bl,[edi+xreg]
	adc	bh,0
	mov	ebp,eax
	push	ebx
	Readbyte				;readbyte and discard it
	pop	eax
	mov	ebp,eax
%endmacro

%macro StoreAbsY 0
	add   edx,3
	mov	ebx,eax
	add	al,[edi+yreg]
	add	bl,[edi+yreg]
	adc	bh,0
	mov	ebp,eax
	push	ebx
	Readbyte				;readbyte and discard it
	pop	eax
	mov	ebp,eax
%endmacro
	
%macro StoreIndX 0
	xor	ah,ah
	add	edx,2
	add	al,[edi+xreg]
	mov	bl,[esi+eax]
	inc	al
	mov	bh,[esi+eax]
	mov	eax,ebx
	mov	ebp,ebx
%endmacro

%macro StoreIndY 0
	xor	ah,ah
	add	edx,2
	mov	bl,[esi+eax]		;(zpg) get addr from zpg
	inc	al
	mov	bh,[esi+eax]
	mov	eax,ebx			;ax=(zpg)
	add	al,[edi+yreg]
	add	bl,[edi+yreg]
	adc	bh,0				;(zpg)+y
	mov	ebp,eax
	push	ebx
	Readbyte				;do unecessary read
	pop	eax
	mov	ebp,eax
%endmacro
;-----------------------------------------------------------------------------------
;===================================================================================
;instruction macros
	
NNZCVF	equ	0x0ff-(SIGN|ZERO|CARRY|OVERFLOW)

NNZCF		equ	0x0ff-(SIGN|ZERO|CARRY)

NNZF		equ	0x0ff-(SIGN|ZERO)
	
%macro finito 1
	killRMWFlag
	mov	ebx,%1
	mov	eax,[edi+cycles]
	add	eax,ebx
	add	dword[edi+clk],ebx
	inc	dword[edi+inst]		;notch up another one to 6502
	mov	[edi+cycles],eax

	test	byte[edi+DEBUG],-1
	jnz	near EXIT6510

	sub	[_cputimer],ebx
	js	near EXIT6510	

	test	byte[edi+nmi],-1
	jnz	near M6502NMI

	test	byte[edi+irq],-1
	jnz	near M6502IRQ
		
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
%endmacro

%macro DoAdc 0
	mov	al,ch
	and	ch,NNZCVF
	shr	al,1
	adc	cl,bl					;adc byte to areg
	mov	eax,OVERFLOW
	jo	%%nooverflowadc
	mov	eax,0
%%nooverflowadc
	adc	ch,0
	mov	bl,cl
	or	ch,al
	or	ch,[ebx+flags]
%endmacro

%macro DoAbcd 0	
	push	edx
	mov	bh,ch
	mov	dh,ch
	and	bh,0x01
	and	ch,NNZCVF
	mov	al,cl
	shr	dh,1
	adc	al,bl
	mov	dl,0
		
	jno	%%nooverflowabcd
	mov	dl,OVERFLOW
%%nooverflowabcd
	adc	ch,0
	or	ch,dl
		
	test	al,0xff
	jnz	%%fixz
	or	ch,ZERO
%%fixz
	
	xor	eax,eax
	xor	edx,edx
		
	mov	al,cl
	mov	ah,bl
	and	ax,0x0f0f
	add	al,ah
	add	al,bh
		
	cmp	al,0x0a
	jb	%%fixedunits
	add	al,0x06
	mov	dh,0x01
	and	al,0x0f
%%fixedunits
	mov	bh,al
	mov	al,cl
	mov	ah,bl
		
	shr	al,4		;A>>4
	shr	ah,4		;s>>4
		
	add	al,dh
	add	al,ah		;AH>>4


	mov	ah,al
	shl	ah,0x4
	and	ah,0x80
	or	ch,ah		;sign
	
	cmp	al,0x0a
	jb	%%fixedtens
	add	al,0x06
	or	ch,CARRY
%%fixedtens
	shl	al,4
	or	al,bh
	mov	cl,al
	pop	edx
%endmacro
	
%macro instADC 1
	test	ch,DECIMAL
	jnz	near %%abcd
	DoAdc
	finito %1
%%abcd
	DoAbcd
	finito %1
%endmacro

%macro instAND 0
	and	ch,NNZF
	and	bl,cl
	or	ch,[ebx+flags]
	mov	cl,bl
%endmacro

%macro instASL 0
	RMWFlag
	and	ch,NNZCF
	add	bl,bl
	adc	ch,0
	or	ch,[ebx+flags]
%endmacro

%macro instBIT 0
	and	ch,0xFF-(SIGN+ZERO+OVERFLOW)
	mov	bh,bl
	and	bl,cl
	jnz	%%byebit
	or	ch,ZERO
%%byebit
	and	bh,(SIGN+OVERFLOW)
	or	ch,bh
%endmacro

%macro instCMP 0
	and	ch,NNZCF
	xor	eax,eax
	mov	al,cl
	sub	al,bl
	cmc
	adc	ch,0
	or	ch,[eax+flags]
%endmacro

%macro instCPX 0
	and	ch,NNZCF
	mov	eax,[edi+xreg]
	sub	al,bl
	cmc
	adc	ch,0
	or	ch,[eax+flags]
%endmacro

%macro instCPY 0
	and	ch,NNZCF
	mov	eax,[edi+yreg]
	sub	al,bl
	cmc
	adc	ch,0
	or	ch,[eax+flags]
%endmacro

%macro instDEC 0
	RMWFlag
	and	ch,NNZF
	dec	bl
	or	ch,[ebx+flags]
	call	writemem
	killRMWFlag
%endmacro

%macro instEOR 0
	and	ch,NNZF
	xor	bl,cl
	or	ch,[ebx+flags]
	mov	cl,bl
%endmacro

%macro instINC 0
	RMWFlag
	and	ch,NNZF
	inc	bl
	or	ch,[ebx+flags]
	call	writemem
	killRMWFlag
%endmacro


%macro instLDA 0
	and	ch,NNZF
	mov	cl,bl
	or	ch,[ebx+flags]
%endmacro

%macro instLDX 0
	and	ebx,0xff
	and	ch,NNZF
	mov	[edi+xreg],ebx
	or	ch,[ebx+flags]
%endmacro

%macro instLDY 0
	and	ebx,0xff
	and	ch,NNZF
	mov	[edi+yreg],ebx
	or	ch,[ebx+flags]
%endmacro

%macro instLSR 0
	RMWFlag
	and	ch,NNZCF
	shr	bl,1
	adc	ch,0
	or	ch,[ebx+flags]
%endmacro

%macro instORA 0
	and	ch,NNZF
	or	bl,cl
	or	ch,[ebx+flags]
	mov	cl,bl
%endmacro

%macro instROL 0
	RMWFlag
	and	ch,NNZF
	shr	ch,1
	adc	bl,bl
	rcl	ch,1
	or	ch,[ebx+flags]
%endmacro

%macro instROR 0
	RMWFlag
	mov	al,ch
	and	ch,NNZCF
	shr	al,1
	rcr	bl,1
	adc	ch,0
	or	ch,[ebx+flags]
%endmacro
	
%macro DoSbc 0
	mov	al,ch
	and	ch,NNZCVF			;get mask out nzcv flags
	shr	al,1
	cmc
	sbb	cl,bl
	cmc
	mov	al,0
	jno	%%nooverflowsbc
	mov	al,OVERFLOW
%%nooverflowsbc
	adc	ch,0
	mov	bl,cl
	or	ch,al
	and	ebx,0xff
	or	ch,[ebx+flags]
%endmacro
	
%macro DoSbcd 0
	push	edx
	xor	eax,eax
	xor	edx,edx
	mov	bh,ch
	mov	dl,ch
	and	ch,NNZCVF
	mov	al,cl
	shr	bh,1
	cmc
	sbb	al,bl
	cmc
	mov	dh,0
	jno	%%nooverflowsbcd
	mov	dh,OVERFLOW
%%nooverflowsbcd
	adc	ch,0
	or	ch,dh
	or	ch,[eax+flags]
	xor	eax,eax
		
	mov	al,cl
	mov	ah,bl
		
	and	ax,0x0f0f
	shr	dl,1
	cmc
	sbb	al,ah
	xor	edx,edx
	test	al,0x10
	jz	%%nosubfix1
	mov	dl,0x1
	sub	al,0x06
	and	al,0x0f
%%nosubfix1
	mov	dh,al
	mov	al,cl
	mov	ah,bl
	shr	eax,4
	and	eax,0x0f0f
	shr	dl,1
	sbb	al,ah
	test	al,0x10
	jz	%%nosubfix2
	sub	al,0x06
%%nosubfix2
	shl	al,4
	or	al,dh
	mov	cl,al
	pop	edx
%endmacro

%macro instSBC 1
	test	ch,DECIMAL
	jnz	near %%sbcd
	DoSbc
	finito %1
%%sbcd
	DoSbcd
	finito %1
%endmacro

%macro instSTA 0
	mov	bl,cl
%endmacro

%macro instSTX 0
	mov	ebx,[edi+xreg]
%endmacro

%macro instSTY 0
	mov	ebx,[edi+yreg]
%endmacro
;-----------------------------------------------------------------------------------
;UNDOCUMENTED opcodes

;DCP -> m-1,cmp m
%macro instDCP 0
	RMWFlag
	dec	bl
	and	ch,NNZCF
	xor	eax,eax
	mov	al,cl
	sub	al,bl
	cmc
	adc	ch,0
	or	ch,[eax+flags]
%endmacro

;ISB -> m+1, sbc m
%macro instISB 1
	RMWFlag
	inc	bl
	push	ebx
	test	ch,DECIMAL
	jnz	near %%isbsbcd
	DoSbc
	pop	ebx
	call	writemem
	finito %1
%%isbsbcd
	DoSbcd	
	pop	ebx
	call	writemem
	finito %1
%endmacro
	
;LAX -> LDA #xxxx,LDX #xxx
%macro instLAX 0
	and	ch,NNZF
	mov	cl,bl
	mov	[edi+xreg],ebx
	or	ch,[ebx+flags]
%endmacro

;RLA -> ROL byte ,AND byte 
%macro instRLA 0
	RMWFlag
	and	ch,NNZF
	shr	ch,1
	adc	bl,bl				;ROL #xxxx
	rcl	ch,1				;get carry flag
	xor	eax,eax
	and	cl,bl				;AND byte
	mov	al,cl
	or	ch,[eax+flags]
	call	writemem
%endmacro

;RRA -> ROR #xxxx, ADC #xxxx
%macro instRRA 1
	RMWFlag
	shr	ch,1
	rcr	bl,1				;ROR #xxxx
	rcl	ch,1
	push	ebx
	and	ebx,0xff
	test	ch,DECIMAL
	jnz near	%%abcdrra
	DoAdc
	pop	ebx
	call	writemem
	finito %1
%%abcdrra
	DoAbcd
	pop	ebx
	call	writemem
	finito %1
%endmacro

;SAX -> m=A&X
%macro instSAX 0
	mov	bl,[edi+xreg]
	and	bl,cl
%endmacro
	 
;SLO -> ASL #xxxx ,ORA
%macro instSLO 0
	RMWFlag
	and	ch,NNZCF
	add	bl,bl				;ASL #xxxx
	adc	ch,0				;get carry
	xor	eax,eax
	or	cl,bl				;ORA
	mov	al,cl
	or	ch,[eax+flags]
%endmacro

;SRE -> LSR #xxxx,EOR
%macro instSRE 0
	RMWFlag
	xor	eax,eax
	and	ch,NNZCF
	shr	bl,1				;LSR #xxxx
	adc	ch,0
	xor	cl,bl
	mov	al,cl				;EOR #xxxx
	or	ch,[eax+flags]
%endmacro
;-----------------------------------------------------------------------------------

future
	mov	edx,[edi+ppc2]
	finito 7
;------------------------------------------------
;ADC
inst69
	ReadImm
	instADC 2
inst65
	ReadZpg
	instADC 3
inst75
	ReadZpgX
	instADC 4
inst6d
	ReadAbs
	instADC 4
inst7d
	ReadAbsX
	instADC 4
inst79
	ReadAbsY
	instADC 4
inst61
	ReadIndX
	instADC 6
inst71
	ReadIndY
	instADC 5
;-----------------------------
;ANC - NKHOSI SEKELELE AFRICA - sorry :-)
inst0b
inst2b
	ReadImm
	and	ch,NNZCF
	and	bl,cl
	or	ch,[ebx+flags]
	mov	cl,bl
	and	bl,0x80
	rol	bl,1
	or	ch,bl
	finito 2

;-----------------------------
;AND
inst29
	ReadImm
	instAND
	finito 2
inst25
	ReadZpg
	instAND
	finito 3
inst35
	ReadZpgX
	instAND
	finito 4
inst2d
	ReadAbs
	instAND
	finito 4
inst3d
	ReadAbsX
	instAND
	finito 4
inst39
	ReadAbsY
	instAND
	finito 4
inst21
	ReadIndX
	instAND
	finito 6
inst31
	ReadIndY
	instAND
	finito 5
;---------------------------
;ANE
inst8b
	ReadImm
	and	ch,NNZF
	mov	al,[edi+xreg]
	or	cl,0xee
	and	cl,al
	and	cl,bl
	mov	bl,cl
	or	ch,[ebx+flags]
	finito 2
;---------------------------
;ASL
inst0a
	inc	edx
	and	ch,NNZCF
	add	cl,cl
	adc	ch,0
	mov	bl,cl
	or	ch,[ebx+flags]
	finito 2
inst06
	StoreZpg
	instASL
	mov	[esi+ebp],bl
	finito 5
inst16
	StoreZpgX
	instASL
	mov	[esi+ebp],bl
	finito 6
inst0e
	StoreAbs
	Readbyte
	instASL
	Writebyte
	finito 6
inst1e
	StoreAbsX
	Readbyte
	instASL
	Writebyte
	finito 7
;---------------------------
;ARR
inst6b
;	jmp	future
	ReadImm
	push	edx
	and	eax,0xff
	and	ebx,0xff
	and	edx,0xff
		
	and	bl,cl
		
	mov	dl,ch
	mov	al,ch
	mov	dh,bl
		
	and	ch,NNZCVF
	shl	al,7
		
	shr	bl,1
	or	bl,al				;accu

	
	test	ch,DECIMAL
	jnz	near DecArr
		
	mov	al,bl
	mov	cl,bl
		
	or	ch,[ebx+flags]		;nz flags
		
	and	al,0x20
	and	bl,0x40
		
	shl	al,1
	xor	al,bl
	and	al,0x40
		
	or	ch,al				;overflow
		
	shr	bl,6
	or	ch,bl
	pop	edx
	finito 2
		
DecArr
	mov	cl,bl
	mov	al,dh

	test	bl,0xff
	jnz	.nono
	or	ch,ZERO
.nono
	and	dl,0x1
	shl	dl,7
	or	ch,dl				;sign
	
	xor	bl,al				;tmp
	and	bl,0x40
	or	ch,bl				;overflow
		
	mov	bl,dh
	and	al,0xf
	and	bl,0x1
		
	add	al,bl
		
	cmp	al,0x6
	jb	DecArr2
	mov	al,cl
	add	al,6
	and	al,0xf
	and	cl,0xf0
	or	cl,al
		
DecArr2
	xor	ebx,ebx
	xor	eax,eax
	mov	bl,dh
	mov	al,dh
	and	bl,0x10
	add	eax,ebx
	and	eax,0x1f0
	cmp	eax,0x60
	jb	DecArr3
	add	cl,0x60
	or	ch,CARRY
DecArr3
	pop	edx
	finito 2
;---------------------------
;ASR
inst4b
	ReadImm
	and	bl,cl
	and	ch,NNZCF
	shr	bl,1
	adc	ch,0
	or	ch,[ebx+flags]
	mov	cl,bl
	finito 2
;---------------------------
%macro brancher 0
	mov	ebx,edx
	and	eax,0xff
	cbw					;if no debug required these 2operations
	add	dx,ax				;can be placed in brancher
	inc	dword[edi+cycles]		;guaranteed extra cycle from here on
	inc	dword[edi+clk]
	cmp	dh,bh
	jz	%%branch2			;branching to same page 
	inc	dword[edi+cycles]
	inc	dword[edi+clk]
%%branch2
	mov	eax,edx			;is xchg quicker than mov?
%endmacro
	
;BCC
inst90
	add	edx,2				;point to next opcode
	test	ch,CARRY
	jnz .bcc
	brancher
.bcc
	finito 2
;---------------------------
;BCS
instb0
	add	edx,2
	test	ch,CARRY
	jz bcs1
	brancher
bcs1
	finito 2

;---------------------------
;BEQ
instf0
	add	edx,2
	test	ch,ZERO
	jz beq1
	brancher
beq1
	finito 2
;---------------------------
;BIT
inst24
	ReadZpg
	instBIT
	finito 3
inst2c
	ReadAbs
	instBIT
	finito 4

;---------------------------
;BMI
inst30
	add	edx,2
	test	ch,SIGN
	jz bmi1
	brancher
bmi1
	finito 2

;---------------------------
;BNE
instd0
	add	edx,2
	test	ch,ZERO
	jnz bne1
	brancher
bne1
	finito 2
;---------------------------
;BPL
inst10
	add	edx,2
	test	ch,SIGN
	jnz bpl1
	brancher
bpl1
	finito 2
;---------------------------
;BVC
inst50
	add	edx,2
	test	ch,OVERFLOW
	jnz bvc1
	brancher
bvc1
	finito 2
;---------------------------
;BVS
inst70
	add	edx,2
	test	ch,OVERFLOW
	jz bvs1
	brancher
bvs1
	finito 2
;---------------------------
;BRK
inst00
	add	edx,2				;note - must push pc+2 to stack
	mov	bl,ch
	mov	eax,[edi+psp]
	mov	[esi+eax],dh		;place high byte
	dec	al
	mov	[esi+eax],dl		;place low byte
	dec	al
	or	bl,UNDEFINED
	mov	[esi+eax],bl
	dec	al
	mov	[edi+psp],eax
	or	ch,INTERUPT+BREAK
	mov	eax,0xfffe
	call	readmem
	mov	edx,[esi+eax]
	and	edx,0xffff
	finito 7
;---------------------------
;CLC
inst18
	inc	edx
	and	ch,0x0ff-CARRY
	finito 2

;---------------------------
;CLD
instd8
	inc	edx
	and	ch,0x0ff-DECIMAL
	finito 2

;---------------------------
;CLI
inst58
	inc	edx
	and	ch,0x0ff-INTERUPT
	finito 2

;---------------------------
;CLV
instb8
	inc	edx
	and	ch,0x0ff-OVERFLOW
	finito 2

;----------------------------
;CMP
instc9
	ReadImm
	instCMP
	finito 2
instc5
	ReadZpg
	instCMP
	finito 3
instd5
	ReadZpgX
	instCMP
	finito 4
instcd
	ReadAbs
	instCMP
	finito 4
instdd
	ReadAbsX
	instCMP
	finito 4
instd9
	ReadAbsY
	instCMP
	finito 4
instc1
	ReadIndX
	instCMP
	finito 6
instd1
	ReadIndY
	instCMP
	finito 5

;---------------------------
;CPX
inste0
	ReadImm
	instCPX
	finito 2
inste4
	ReadZpg
	instCPX
	finito 3
instec
	ReadAbs
	instCPX
	finito 4

;---------------------------
;CPY
instc0
	ReadImm
	instCPY
	finito 2
instc4
	ReadZpg
	instCPY
	finito 3
instcc
	ReadAbs
	instCPY
	finito 4
;---------------------------
;DCP
instc7
	StoreZpg
	instDCP
	mov	[esi+ebp],bl
	finito 5
instd7
	StoreZpgX
	instDCP
	mov	[esi+ebp],bl
	finito 6
instcf
	StoreAbs
	Readbyte
	instDCP
	Writebyte
	finito 6
instdf
	StoreAbsX
	Readbyte
	instDCP
	Writebyte
	finito 7
instdb
	StoreAbsY
	Readbyte
	instDCP
	Writebyte
	finito 7
instc3
	StoreIndX
	Readbyte
	instDCP
	Writebyte
	killRMWFlag
	finito 8
instd3
	StoreIndY
	Readbyte
	instDCP
	Writebyte
	finito 8
;---------------------------
;DEC
instc6
	StoreZpg
	instDEC
	finito 5
instd6
	StoreZpgX
	instDEC
	finito 6
instce
	StoreAbs
	Readbyte
	instDEC
	finito 6
instde
	StoreAbsX
	Readbyte
	instDEC
	finito 7

;---------------------------
;DEX
instca
	dec	byte[edi+xreg]
	and	ch,NNZF
	mov	ebx,[edi+xreg]
	inc	edx
	or	ch,[ebx+flags]
	finito 2

;---------------------------
;DEY
inst88
	dec	byte[edi+yreg]
	and	ch,NNZF
	mov	ebx,[edi+yreg]
	inc	edx
	or	ch,[ebx+flags]
	finito 2

;----------------------------
;EOR
inst49
	ReadImm
	instEOR
	finito 2
inst45
	ReadZpg
	instEOR
	finito 3
inst55
	ReadZpgX
	instEOR
	finito 4
inst4d
	ReadAbs
	instEOR
	finito 4
inst5d
	ReadAbsX
	instEOR
	finito 4
inst59
	ReadAbsY
	instEOR
	finito 4
inst41
	ReadIndX
	instEOR
	finito 6
inst51
	ReadIndY
	instEOR
	finito 5

;---------------------------
;INC
inste6
	StoreZpg
	instINC
	finito 5
instf6
	StoreZpgX
	instINC
	finito 6
instee
	StoreAbs
	Readbyte
	instINC
	finito 6
instfe
	StoreAbsX
	Readbyte
	instINC
	finito 7

;---------------------------
;INX
inste8
	inc	byte[edi+xreg]
	and	ch,NNZF
	mov	ebx,[edi+xreg]
	inc	edx
	or	ch,[ebx+flags]
	finito 2
;---------------------------
;INY
instc8
	inc	byte[edi+yreg]
	and	ch,NNZF
	mov	ebx,[edi+yreg]
	inc	edx
	or	ch,[ebx+flags]
	finito 2

;---------------------------
;ISB
inste7
	StoreZpg
	instISB 5
instf7
	StoreZpgX
	instISB 6
instef
	StoreAbs
	Readbyte
	instISB 6
instff
	StoreAbsX
	Readbyte
	instISB 7
instfb
	StoreAbsY
	Readbyte
	instISB 7
inste3
	StoreIndX
	Readbyte
	instISB 8
instf3
	StoreIndY
	Readbyte
	instISB 8

;---------------------------
;JAM
instjam
	mov	edx,[edi+ppc2]
	finito 7
;---------------------------
;JMP
inst4c
	mov	edx,eax
	finito 3
;---------------------------
; JMP($xxxx) has same boundary constraints as other
; indirect addressing, ie Jmp ($abff) would be
; PCL=peek($abff) PCH=peek($ab00)

inst6c
	call	readmem
	mov	dl,[esi+eax]
	inc	al
	mov	dh,[esi+eax]
	finito 5
;---------------------------
; special note, JSR pushes PC+2 instead of PC+3 on to stack
; RTS adds one to the address from stack.
; the jump table in CBM basic takes account of this ie PRINT
; routine is listed as $aa9f, instead of $aaa0

;JSR
inst20
	add	edx,2
	mov	ebx,[edi+psp]
	mov	[esi+ebx],dh		;place high byte
	dec	bl
	mov	[esi+ebx],dl		;place low byte
	dec	bl
	mov	edx,eax
	mov	[edi+psp],ebx
	finito 6
;----------------------------
;LAS
instbb
	ReadAbsY
	and	ch,NNZF
	mov	al,[edi+psp]
	and	bl,al
	mov	[edi+psp],bl
	mov	[edi+xreg],bl
	mov	cl,bl
	or	ch,[ebx+flags]
	finito 4
;----------------------------
;LAX
insta7
	ReadZpg
	instLAX
	finito 3
instb7
	ReadZpgY
	instLAX
	finito 4
instaf
	ReadAbs
	instLAX
	finito 4
instbf
	ReadAbsY
	instLAX
	finito 4
insta3
	ReadIndX
	instLAX
	finito 6
instb3
	ReadIndY
	instLAX
	finito 5
;----------------------------
;LDA
insta9
	ReadImm
	instLDA
	finito 2
insta5
	ReadZpg
	instLDA
	finito 3
instb5
	ReadZpgX
	instLDA
	finito 4
instad
	ReadAbs
	instLDA
	finito 4
instbd
	ReadAbsX
	instLDA
	finito 4
instb9
	ReadAbsY
	instLDA
	finito 4
insta1
	ReadIndX
	instLDA
	finito 6
instb1
	ReadIndY
	instLDA
	finito 5
;----------------------------
;LDX
insta2
	ReadImm
	instLDX
	finito 2
insta6
	ReadZpg
	instLDX
	finito 3
instb6
	ReadZpgY
	instLDX
	finito 4
instae
	ReadAbs
	instLDX
	finito 4
instbe
	ReadAbsY
	instLDX
	finito 4

;----------------------------
;LDY
insta0
	ReadImm
	instLDY
	finito 2
insta4
	ReadZpg
	instLDY
	finito 3
instb4
	ReadZpgX
	instLDY
	finito 4
instac
	ReadAbs
	instLDY
	finito 4
instbc
	ReadAbsX
	instLDY
	finito 4

;---------------------------
;LSR
inst4a
	inc	edx
	and	ch,NNZCF
	shr	cl,1
	adc	ch,0
	mov	bl,cl
	or	ch,[ebx+flags]
	finito 2
inst46
	StoreZpg
	instLSR
	mov	[esi+ebp],bl
	finito 5
inst56
	StoreZpgX
	instLSR
	mov	[esi+ebp],bl
	finito 6
inst4e
	StoreAbs
	Readbyte
	instLSR
	call	writemem
	finito 6
inst5e
	StoreAbsX
	Readbyte
	instLSR
	call	writemem
	finito 7
;----------------------------
;LXA
instab
	ReadImm
	and	ch,NNZF
	or	cl,0xee
	and	bl,cl
	or	ch,[ebx+flags]
	mov	cl,bl
	mov	[edi+xreg],bl
	finito 2

;----------------------------
;NOP
instea
	inc	edx
	finito 2
;UNDOC
instnopz
	add	edx,2
	finito 3
instnopt
	ReadAbsX
	finito 4
instnopt2
	ReadAbs
	finito 4
;----------------------------
;ORA
inst09
	ReadImm
	instORA
	finito 2
inst05
	ReadZpg
	instORA
	finito 3
inst15
	ReadZpgX
	instORA
	finito 4
inst0d
	ReadAbs
	instORA
	finito 4
inst1d
	ReadAbsX
	instORA
	finito 4
inst19
	ReadAbsY
	instORA
	finito 4
inst01
	ReadIndX
	instORA
	finito 6
inst11
	ReadIndY
	instORA
	finito 5

;---------------------------
;PHA
inst48
	mov	eax,[edi+psp]
	mov	[esi+eax],cl
	dec	al
	inc	edx
	mov	[edi+psp],eax
	finito 3
;---------------------------
;PHP
inst08
	or	ch,UNDEFINED+BREAK
	mov	eax,[edi+psp]

	mov	[esi+eax],ch
	dec	al

	mov	[edi+psp],eax
	inc	edx
	finito 3

;---------------------------
;PLA
inst68
	mov	eax,[edi+psp]
	and	ch,NNZF
	inc	al
	mov	bl,[esi+eax]
	mov	cl,bl
	or	ch,[ebx+flags]
	mov	[edi+psp],eax
	inc	edx
	finito 4

;---------------------------
;PLP
inst28
	mov	eax,[edi+psp]
	inc	al
	mov	ch,[esi+eax]
	or	ch,UNDEFINED
	mov	[edi+psp],eax
	inc	edx
	and	ch,0xff-BREAK
	finito 4

;---------------------------
;RLA
inst27
	StoreZpg
	instRLA
	finito 5
inst37
	StoreZpgX
	instRLA
	finito 6
inst2f
	StoreAbs
	Readbyte
	instRLA
	finito 6
inst3f
	StoreAbsX
	Readbyte
	instRLA
	finito 7
inst3b
	StoreAbsY
	Readbyte
	instRLA
	finito 7
inst23
	StoreIndX
	Readbyte
	instRLA
	finito 8
inst33
	StoreIndY
	Readbyte
	instRLA
	finito 8
		
;---------------------------
;RRA
inst67
	StoreZpg
	instRRA 5
inst77
	StoreZpgX
	instRRA 6
inst6f
	StoreAbs
	Readbyte
	instRRA 6
inst7f
	StoreAbsX
	Readbyte
	instRRA 7
inst7b
	StoreAbsY
	Readbyte
	instRRA 7
inst63
	StoreIndX
	Readbyte
	instRRA 8
inst73
	StoreIndY
	Readbyte
	instRRA 8

;---------------------------
;ROL
inst2a
	mov	bl,ch
	inc	edx
	and	ch,NNZCF
	shr	bl,1
	adc	cl,cl
	adc	ch,0
	mov	bl,cl
	or	ch,[ebx+flags]
	finito 2
inst26
	StoreZpg
	instROL
	mov	[esi+ebp],bl
	finito 5
inst36
	StoreZpgX
	instROL
	mov	[esi+ebp],bl
	finito 6
inst2e
	StoreAbs
	Readbyte
	instROL
	Writebyte
	finito 6
inst3e
	StoreAbsX
	Readbyte
	instROL
	Writebyte
	finito 7

;---------------------------
;ROR
inst6a
	mov	bl,ch
	inc	edx
	and	ch,NNZCF
	shr	bl,1
	rcr	cl,1
	adc	ch,0
	mov	bl,cl
	or	ch,[ebx+flags]
	finito 2
inst66
	StoreZpg
	instROR
	mov	[esi+ebp],bl
	finito 5
inst76
	StoreZpgX
	instROR
	mov	[esi+ebp],bl
	finito 6
inst6e
	StoreAbs
	Readbyte
	instROR
	Writebyte
	finito 6
inst7e
	StoreAbsX
	Readbyte
	instROR
	Writebyte
	finito 7
;---------------------------
;RTI
inst40
	mov	eax,[edi+psp]
	inc	al
	mov	ch,[esi+eax]	;get psr
	or	ch,UNDEFINED
	inc	al
	mov	dl,[esi+eax]	;low byte
	inc	al
	mov	dh,[esi+eax]	;high byte
	mov	[edi+psp],eax
	and	ch,0xff-BREAK
	finito 6

;---------------------------
; RTS adds one to the return address see JSR

;RTS
inst60
	mov	eax,[edi+psp]
	inc	al
	mov	dl,[esi+eax]	;low byte
	inc	al
	mov	dh,[esi+eax]	;high byte
	mov	[edi+psp],eax
	inc	edx			;account for bug
	finito 6
;---------------------------
;SAX
inst87
	StoreZpg
	instSAX
	mov	[esi+ebp],bl
	finito 3
inst97
	StoreZpgY
	instSAX
	mov	[esi+ebp],bl
	finito 4
inst8f
	StoreAbs
	instSAX
	Writebyte
	finito 4
inst83
	StoreIndX
	RMWFlag
	instSAX
	Writebyte
	finito 6
;----------------------------
;SBC
inste9
	ReadImm
	instSBC 2
inste5
	ReadZpg
	instSBC 3
instf5
	ReadZpgX
	instSBC 4
insted
	ReadAbs
	instSBC 4
instfd
	ReadAbsX
	instSBC 4
instf9
	ReadAbsY
	instSBC 4
inste1
	ReadIndX
	instSBC 6
instf1
	ReadIndY
	instSBC 5
;----------------------------
;SBX
instcb
	ReadImm
	xor	eax,eax
	and	ch,NNZCF
	mov	al,[edi+xreg]
	and	al,cl
	sub	al,bl
	cmc
	adc	ch,0
	or	ch,[eax+flags]
	mov	[edi+xreg],al
	finito 2
;---------------------------
;SEC
inst38
	inc	edx
	or	ch,CARRY
	finito 2

;---------------------------
;SED
instf8
	inc	edx
	or	ch,DECIMAL
	finito 2

;---------------------------
;SEI
inst78
	inc	edx
	or	ch,INTERUPT
	finito 2
;---------------------------
;SHA
inst93
;	StoreIndY 

	and	eax,0xff
	xor	ebx,ebx
	add	edx,2
	push	edx
	push	eax
	mov	bl,[esi+eax]		;(zpg) get addr from zpg
	inc	al
	mov	bh,[esi+eax]
	mov	eax,ebx			;ax=(zpg)
	add	al,[edi+yreg]
	add	bl,[edi+yreg]
	adc	bh,0				;(zpg)+y
	mov	ebp,eax
		
	push	ebx
	Readbyte				;do unecessary read
	pop	eax				;correct address
	pop	edx				;original byte pcl
	
	xor	ebx,ebx
	inc	dl				;pch
	
		
	mov	bl,[esi+edx]		;
	inc	bl
	and	bl,cl
	and	bl,[edi+xreg]		

		
	mov	ebp,eax
	pop	edx
	Writebyte
	finito 6	
inst9f
	add	edx,3
	push	edx
	mov	ebx,[edi+xreg]
	mov	edx,eax
	inc	dh
	add	eax,[edi+yreg]
	and	bl,cl
	and	bl,dh
	mov	ebp,eax
	pop	edx
	Writebyte
	finito 5
;---------------------------
;SHS
inst9b
	add	edx,3
	push	edx
	mov	edx,eax
	mov	ebx,[edi+xreg]
	add	eax,[edi+yreg]
	and	bl,cl
	mov	[edi+psp],bl
	inc	dh
	and	bl,dh
	mov	ebp,eax
	pop	edx
	Writebyte
	finito 5
;---------------------------
;SHX
inst9e
	add	edx,3
	push	edx
	mov	edx,eax
	mov	ebx,[edi+xreg]
	add	eax,[edi+yreg]
	inc	dh
	and	bl,dh
	mov	ebp,eax
	pop	edx
	Writebyte
	finito 5
;---------------------------
;SHY
inst9c
	add	edx,3
	push	edx
	mov	edx,eax
	mov	ebx,[edi+yreg]
	add	eax,[edi+xreg]
	inc	dh
	and	bl,dh
	pop	edx
	mov	ebp,eax
	Writebyte
	finito 5
;---------------------------
;SLO
inst07
	StoreZpg
	instSLO
	mov	[esi+ebp],bl
	killRMWFlag
	finito 5
inst17
	StoreZpgX
	instSLO
	mov	[esi+ebp],bl
	finito 6
inst0f
	StoreAbs
	Readbyte
	instSLO
	Writebyte
	finito 6
inst1f
	StoreAbsX
	Readbyte
	instSLO
	Writebyte
	finito 7
inst1b
	StoreAbsY
	Readbyte
	instSLO
	Writebyte
	finito 7
inst03
	StoreIndX
	Readbyte
	instSLO
	Writebyte
	finito 8
inst13
	StoreIndY
	Readbyte
	instSLO
	Writebyte
	finito 8
;---------------------------
;SRE
inst47
	StoreZpg
	instSRE
	mov	[esi+ebp],bl
	finito 5
inst57
	StoreZpgX
	instSRE
	mov	[esi+ebp],bl
	finito 6
inst4f
	StoreAbs
	Readbyte
	instSRE
	Writebyte
	finito 6
inst5f
	StoreAbsX
	Readbyte
	instSRE
	Writebyte
	finito 7
inst5b
	StoreAbsY
	Readbyte
	instSRE
	Writebyte
	finito 7
inst43
	StoreIndX
	Readbyte
	instSRE
	Writebyte
	finito 8
inst53
	StoreIndY
	Readbyte
	instSRE
	finito 8
;----------------------------
;STA
inst85
	StoreZpg
	instSTA
	mov	[esi+ebp],bl
	finito 3
inst95
	StoreZpgX
	instSTA
	mov	[esi+ebp],bl	
	finito 4
inst8d
	StoreAbs 
	instSTA
	Writebyte
	finito 4
inst9d
	StoreAbsX 
	RMWFlag
	instSTA
	Writebyte
	finito 5
inst99
	StoreAbsY 
	RMWFlag
	instSTA
	Writebyte
	finito 5
inst81
	StoreIndX 
	RMWFlag
	instSTA
	Writebyte
	finito 6
inst91
	StoreIndY 
	RMWFlag
	instSTA
	Writebyte
	finito 6

;----------------------------
;STX
inst86
	StoreZpg
	instSTX
	mov	[esi+ebp],bl
	finito 3
inst96
	StoreZpgY
	instSTX
	mov	[esi+ebp],bl
	finito 4
inst8e
	StoreAbs 
	instSTX
	Writebyte
	finito 4
		
;----------------------------
;STY
inst84
	StoreZpg
	instSTY
	mov	[esi+ebp],bl
	finito 3
inst94
	StoreZpgX
	instSTY
	mov	[esi+ebp],bl
	finito 4
inst8c
	StoreAbs 
	instSTY
	Writebyte
	finito 4
		
;---------------------------
;TAX
instaa
	mov	bl,cl
	and	ch,NNZF
	inc	edx
	mov	[edi+xreg],ebx
	or	ch,[ebx+flags]
	finito 2

;---------------------------
;TAY
insta8
	mov	bl,cl
	and	ch,NNZF
	inc	edx
	mov	[edi+yreg],ebx
	or	ch,[ebx+flags]
	finito 2
;---------------------------
;TSX
instba
	and	ch,NNZF
	mov	bl,[edi+psp]
	inc	edx
	mov	[edi+xreg],bl
	or	ch,[ebx+flags]
	finito 2

;---------------------------
;TXA
inst8a
	and	ch,NNZF
	mov	bl,[edi+xreg]
	inc	edx
	mov	cl,bl
	or	ch,[ebx+flags]
	finito 2

;---------------------------
;TXS
inst9a
	mov	al,[edi+xreg]
	inc	edx
	mov	[edi+psp],al
	finito 2
;---------------------------
;TYA
inst98
	and	ch,NNZF
	mov	bl,[edi+yreg]
	inc	edx
	mov	cl,bl
	or	ch,[ebx+flags]
	finito 2

;===================================================================================
