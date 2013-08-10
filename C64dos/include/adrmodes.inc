;===================================================================================
;-----------------------------------------------------------------------------------

%macro Readbyte 0
	call	readmem
%endmacro
	
%macro ReadImm 0
	add	edx,2
	xor	ah,ah
	mov	ebx,eax
	mov	ebp,eax
%endmacro

%macro ReadZpg 0
	add	edx,2
	xor	ah,ah
	mov	bl,[esi+eax]
	mov	ebp,eax
%endmacro

%macro ReadZpgX 0
	add	edx,2
	xor	ah,ah
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
	call	readmem
	mov	bl,[esi+eax]
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
	mov	ebp,eax
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
	mov	ebp,eax
	Readbyte
%%indynoxtra
	pop	eax
	mov	ebp,eax
	Readbyte
%endmacro
	
;-----------------------------------------------------------------------------------
%macro RMWFlag 0
	mov	byte[edi+RMW],0xff
%endmacro
	
%macro StoreZpg 0
	add	edx,2
	xor	ah,ah
	mov	ebp,eax
%endmacro
	
%macro StoreZpgX 0
	xor	ah,ah
	add	edx,2
	add	al,[edi+xreg]
	mov	ebp,eax
%endmacro

%macro StoreZpgY 0
	xor	ah,ah
	add	edx,2
	add	al,[edi+yreg]
	mov	ebp,eax
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
	
;===================================================================================
;-----------------------------------------------------------------------------------
