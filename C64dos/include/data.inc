
;===================================================================================
[SECTION .data]
;-----------------------------------------------------------------------------------
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
M6510
_cpu
times 80	dd	0
	ALIGN 32
_RAM
memory
times 0x010000 db	 0			;64k ram
basicstart	incbin "include\basic.bin"	;basic (rom)=8k
	ALIGN 32
_PixelRom
charstart	incbin "include\char.bin"		;char (rom)=4k
	ALIGN 32
kernalstart	incbin "include\kernal.bin"	;kernal 8
	ALIGN 32
_SIDCHIP
times 64	db	0
	ALIGN 32
%include "include\equates.inc"

	ALIGN 32
;------------
_gatez	dd	0
_cputimer	dd	0
_ciatimer	dd	0
_victimer	dd	0
_mnsvline	dd	0
_rasterlatch dd	0
crap1		dd	0
crap2		dd	0x0ffffffff
tmpdata	dd	0
UNDEFINED1	equ	crap1-memory
UNDEFINED2	equ	crap2-memory
	ALIGN 32

;-----------------------------------------------------------------------------------
;	   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
clock

	db 7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6
  	db 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
  	db 6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6
  	db 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7

  	db 6, 6, 0, 8, 3, 3, 7, 5, 3, 2, 2, 2, 3, 4, 6, 6
  	db 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 0, 7
  	db 6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6
  	db 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7

  	db 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4
  	db 2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5
  	db 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4
  	db 2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4

  	db 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6
  	db 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
  	db 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6
  	db 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
	ALIGN 32
;		0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
flags	db	0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 ;0
	db	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 ;1
	db	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 ;2
	db	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 ;3
	db	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 ;4
	db	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 ;5
	db	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 ;6
	db	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 ;7

	db	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 ;8
	db	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 ;9
	db	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 ;A
	db	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 ;B
	db	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 ;C
	db	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 ;D
	db	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 ;E
	db	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 ;F

	ALIGN 32
arrflags
	db	CARRY,OVERFLOW,CARRY+OVERFLOW,0,0
	ALIGN 32
;-----------------------------------------------------------------------------------

optable
	dd	inst00	;00	BRK
	dd	inst01	;01	OR	($00,X)
	dd	instjam	;02	JAM
	dd	inst03	;03	SLO	($00,X)
	dd	instnopz	;04	NOP	$00
	dd	inst05	;05	ORA	$00
	dd	inst06	;06	ASL	$00
	dd	inst07	;07	SLO	$00
	dd	inst08	;08	PHP
	dd	inst09	;09	ORA	#$00
	dd	inst0a	;0A	ASL	A
	dd	inst0b	;0B	ANC	#$00
	dd	instnopt2	;0C	NOP	$0000
	dd	inst0d	;0D	OR	$0000
	dd	inst0e	;0E	ASL	$0000
	dd	inst0f	;0F	SLO	$0000

	dd	inst10	;10	BPL	$00
	dd	inst11	;11	OR	($00),Y
	dd	instjam	;12	JAM
	dd	inst13	;13	SLO	($00),Y
	dd	instnopz	;14	NOP	$00,X
	dd	inst15	;15	OR	$00,X
	dd	inst16	;16	ASL	$00,X
	dd	inst17	;17	SLO	$00,X
	dd	inst18	;18	CLC
	dd	inst19	;19	OR	$0000,Y
	dd	instea	;1A	NOP
	dd	inst1b	;1B	SLO	$0000,Y
	dd	instnopt	;1C	NOP	$0000,X
	dd	inst1d	;1D	OR	$0000,X
	dd	inst1e	;1E	ASL	$0000,X
	dd	inst1f	;1F	SLO	$0000,X

	dd	inst20	;20	JSR	#$0000
	dd	inst21	;21	AND	($00,X)
	dd	instjam	;22	JAM
	dd	inst23	;23	RLA	($00,X)
	dd	inst24	;24	BIT	$00
	dd	inst25	;25	AND	$00
	dd	inst26	;26	ROL	$00
	dd	inst27	;27	RLA	$00
	dd	inst28	;28	PLP
	dd	inst29	;29	AND	#$00
	dd	inst2a	;2A	ROL	A
	dd	inst2b	;2B	ANC	#$00
	dd	inst2c	;2C	BIT	$0000
	dd	inst2d	;2D	AND	$0000
	dd	inst2e	;2E	ROL	$0000
	dd	inst2f	;2F	RLA	$0000

	dd	inst30	;30	BMI	#$00
	dd	inst31	;31	AND	($00),Y
	dd	instjam	;32	JAM
	dd	inst33	;33	RLA	($00),Y
	dd	instnopz	;34	NOP	$00,X
	dd	inst35	;35	AND	$00,X
	dd	inst36	;36	ROL	$00,X
	dd	inst37	;37	RLA	$00,X
	dd	inst38	;38	SEC
	dd	inst39	;39	AND	$0000,Y
	dd	instea	;3A	NOP
	dd	inst3b	;3B	RLA	$0000,Y
	dd	instnopt	;3C	NOP	$0000,X
	dd	inst3d	;3D	AND	$0000,X
	dd	inst3e	;3E	ROL	$0000,X
	dd	inst3f	;3F	RLA	$0000,X

	dd	inst40	;40	RTI
	dd	inst41	;41	EOR	($00,X)
	dd	instjam	;42	JAM
	dd	inst43	;43	SRE	($00,X)
	dd	instnopz	;44	NOP	$00
	dd	inst45	;45	EOR	$00
	dd	inst46	;46	LSR	$00
	dd	inst47	;47	SRE	$00
	dd	inst48	;48	PHA
	dd	inst49	;49	EOR	#$00
	dd	inst4a	;4A	LSR	A
	dd	inst4b	;4B	ASR	#$00
	dd	inst4c	;4C	JMP	$0000
	dd	inst4d	;4D	EOR	$0000
	dd	inst4e	;4E	LSR	$0000
	dd	inst4f	;4F	SRE	$0000

	dd	inst50	;50	BVC	$00
	dd	inst51	;51	EOR	($00),Y
	dd	instjam	;52	JAM
	dd	inst53	;53	SRE	($00),Y
	dd	instnopz	;54	NOP	$00,X
	dd	inst55	;55	EOR	$00,X
	dd	inst56	;56	LSR	$00,X
	dd	inst57	;57	SRE	$00,X
	dd	inst58	;58	CLI
	dd	inst59	;59	EOR	$0000,Y
	dd	instea	;5A	NOP
	dd	inst5b	;5B	SRE	$0000,Y
	dd	instnopt	;5C	NOP	$0000,X
	dd	inst5d	;5D	EOR	$0000,X
	dd	inst5e	;5E	LSR	$0000,X
	dd	inst5f	;5F	SRE	$0000,X

	dd	inst60	;60	RTS
	dd	inst61	;61	ADC	($00,X)
	dd	instjam	;62	JAM
	dd	inst63	;63	RRA	($00,X)
	dd	instnopz	;64	NOP	$00
	dd	inst65	;65	ADC	$00
	dd	inst66	;66	ROR	$00
	dd	inst67	;67	RRA	$00
	dd	inst68	;68	PLA
	dd	inst69	;69	ADC	#00
	dd	inst6a	;6A	ROR	A
	dd	inst6b	;6B	ARR	#$00
	dd	inst6c	;6C	JMP	($0000)
	dd	inst6d	;6D	ADC	$0000
	dd	inst6e	;6E	ROR	$0000
	dd	inst6f	;6F	RRA	$0000

	dd	inst70	;70	BVS	$00
	dd	inst71	;71	ADC	($00),Y
	dd	instjam	;72	JAM
	dd	inst73	;73	RRA	($00),Y
	dd	instnopz	;74	NOP	$00,X
	dd	inst75	;75	ADC	$00,X
	dd	inst76	;76	ROR	$00,X
	dd	inst77	;77	RRA	$00,X
	dd	inst78	;78	SEI
	dd	inst79	;79	ADC	$0000,Y
	dd	instea	;7A	NOP
	dd	inst7b	;7B	RRA	$0000,Y
	dd	instnopt	;7C	NOP	$0000,X
	dd	inst7d	;7D	ADC	$0000,X
	dd	inst7e	;7E	ROR	$0000,X
	dd	inst7f	;7F	RRA	$0000,X

	dd	instnopz	;80	NOP	#$00
	dd	inst81	;81	STA	($00,X)
	dd	instnopz	;82	NOP	#$00
	dd	inst83	;83	SAX	($00,X)
	dd	inst84	;84	STY	$00
	dd	inst85	;85	STA	$00
	dd	inst86	;86	STX	$00
	dd	inst87	;87	SAX	$00
	dd	inst88	;88	DEY
	dd	instnopz	;89	NOP	#$00
	dd	inst8a	;8A	TXA
	dd	inst8b	;8B	ANE	#$00
	dd	inst8c	;8C	STY	$0000
	dd	inst8d	;8D	STA	$0000
	dd	inst8e	;8E	STX	$0000
	dd	inst8f	;8F	SAX	$0000

	dd	inst90	;90	BCC	$00
	dd	inst91	;91	STA	($00),Y
	dd	instjam	;92	JAM
	dd	inst93	;93	SHA	($00),Y
	dd	inst94	;94	STY	$00,X
	dd	inst95	;95	STA	$00,X
	dd	inst96	;96	STX	$00,Y
	dd	inst97	;97	SAX	$00,Y
	dd	inst98	;98	TYA
	dd	inst99	;99	STA	$0000,Y
	dd	inst9a	;9A	TXS
	dd	inst9b	;9B	SHS	$0000,Y
	dd	inst9c	;9C	SHY	$0000,X
	dd	inst9d	;9D	STA	$0000,X
	dd	inst9e	;9E	SHX	$0000,Y
	dd	inst9f	;9F	SHA	$0000,Y

	dd	insta0	;A0	LDY	#00
	dd	insta1	;A1	LDA	($00,X)
	dd	insta2	;A2	LDX	#00
	dd	insta3	;A3	LAX	($00,X)
	dd	insta4	;A4	LDY	$00
	dd	insta5	;A5	LDA	$00
	dd	insta6	;A6	LDX	$00
	dd	insta7	;A7	LAX	$00
	dd	insta8	;A8	TAY
	dd	insta9	;A9	LDA	#00
	dd	instaa	;AA	TAX
	dd	instab	;AB	LXA
	dd	instac	;AC	LDY	$0000
	dd	instad	;AD	LDA	$0000
	dd	instae	;AE	LDX	$0000
	dd	instaf	;AF	LAX	$0000

	dd	instb0	;B0	BCS	$00
	dd	instb1	;B1	LDA	($00),Y
	dd	instjam	;B2	JAM
	dd	instb3	;B3	LAX	($00),Y
	dd	instb4	;B4	LDY	$00,X
	dd	instb5	;B5	LDA	$00,X
	dd	instb6	;B6	LDX	$00,Y
	dd	instb7	;B7	LAX	$00,Y
	dd	instb8	;B8	CLV
	dd	instb9	;B9	LDA	$0000,Y
	dd	instba	;BA	TSX
	dd	instbb	;BB	LAS	$0000,Y
	dd	instbc	;BC	LDY	$0000,X
	dd	instbd	;BD	LDA	$0000,X
	dd	instbe	;BE	LDX	$0000,Y
	dd	instbf	;BF	LAX	$0000,Y

	dd	instc0	;C0	CPY	#$00
	dd	instc1	;C1	CMP	($00,X)
	dd	instnopz	;C2	NOP	#$00
	dd	instc3	;C3	DCP	($00,X)
	dd	instc4	;C4	CPY	$00
	dd	instc5	;C5	CMP	$00
	dd	instc6	;C6	DEC	$00
	dd	instc7	;C7	DCP	$00
	dd	instc8	;C8	INY
	dd	instc9	;C9	CMP	#$00
	dd	instca	;CA	DEX
	dd	instcb	;CB	SBX	#$00
	dd	instcc	;CC	CPY	$0000
	dd	instcd	;CD	CMP	$0000
	dd	instce	;CE	DEC	$0000
	dd	instcf	;CF	DCP	$0000

	dd	instd0	;D0	BNE	$00
	dd	instd1	;D1	CMP	($00),Y
	dd	instjam	;D2	JAM
	dd	instd3	;D3	DCP	($00),Y
	dd	instnopz	;D4	NOP	$00,X
	dd	instd5	;D5	CMP	$00,X
	dd	instd6	;D6	DEC	$00,X
	dd	instd7	;D7	DCP	$00,X
	dd	instd8	;D8	CLD
	dd	instd9	;D9	CMP	$0000,Y
	dd	instea	;DA	NOP
	dd	instdb	;DB	DCP	$0000,Y
	dd	instnopt	;DC	NOP	$0000,X
	dd	instdd	;DD	CMP	$0000,X
	dd	instde	;DE	DEC	$0000,X
	dd	instdf	;DF	DCP	$0000,X

	dd	inste0	;E0	CPX	#00
	dd	inste1	;E1	SBC	($00,X)
	dd	instnopz	;E2	NOP	#$00
	dd	inste3	;E3	ISB	($00,X)
	dd	inste4	;E4	CPX	$00
	dd	inste5	;E5	SBC	$00
	dd	inste6	;E6	INC	$00
	dd	inste7	;E7	ISB	$00
	dd	inste8	;E8	INX
	dd	inste9	;E9	SBC	#$00
	dd	instea	;EA	NOP
	dd	inste9	;EB	USB	#$00
	dd	instec	;EC	CPX	$0000
	dd	insted	;ED	SBC	$0000
	dd	instee	;EE	INC	$0000
	dd	instef	;EF	ISB	$0000

	dd	instf0	;F0	BEQ	$00
	dd	instf1	;F1	SBC	($00),Y
	dd	instjam	;F2	JAM
	dd	instf3	;F3	ISB	($00),y
	dd	instnopz	;F4	NOP	$00,X
	dd	instf5	;F5	SBC	$00,X
	dd	instf6	;F6	INC	$00,X
	dd	instf7	;F7	ISB	$00,X
	dd	instf8	;F8	SED
	dd	instf9	;F9	SBC	$0000,Y
	dd	instea	;FA	NOP
	dd	instfb	;FB	ISB	$0000,Y
	dd	instnopt	;FC	NOP	$0000,X
	dd	instfd	;FD	SBC	$0000,X
	dd	instfe	;FE	INC	$0000,X
	dd	instff	;FF	ISB	$0000,X
		

	ALIGN 32


;-----------------------------------------------------------------------------------
; when a read is made from VIC2 the byte here is ORed with the byte thats read 
; from VIC2 this insures that the undefined bits are always set.
	
	ALIGN 32
;VIC2oringtable
	db	0		;00	sprite 0 x cord
	db	0		;01	sprite 0 y cord
	db	0		;02
	db	0		;03
	db	0		;04
	db	0		;05
	db	0		;06
	db	0		;07
	db	0		;08
	db	0		;09
	db	0		;0A
	db	0		;0B
	db	0		;0C
	db	0		;0D
	db	0		;0E	sprite 7 x cord
	db	0		;0F	sprite 7 y cord

	db	0		;10	bit 8 of sprites x cords
	db	0		;11	only bit 7,0-6 is copied from $d011
	db	0		;12	read latch 4 raster @ $d012
	db	0		;13
	db	0		;14
	db	0		;15	sprite enable
	db	0xc0		;16	0x0c0 vic control register 2
	db	0		;17	sprite y expansion
	db	0x01		;18	0x001 vic memory pointer
	db	0x70		;19	read latch 4 vicirq @ $d019
	db	0xf0		;1a	0x0f0 vic interupt enable
	db	0		;1b	sprite data priority
	db	0		;1c	sprite multicolor
	db	0		;1d	sprite x expantion
	db	0		;1e	sprite-sprite collision
	db	0		;1f	sprite-data collision

	db	0xf0		;20	0x0f0 border color
	db	0xf0		;21	0x0f0 background color 0
	db	0xf0		;22	0x0f0
	db	0xf0		;23	0x0f0
	db	0xf0		;24	0x0f0
	db	0xf0		;25	0x0f0 sprite multicolor 1
	db	0xf0		;26	0x0f0 sprite multicolor 2
	db	0xf0		;27	0x0f0 sprite 0 color
	db	0xf0		;28	0x0f0
	db	0xf0		;29	0x0f0
	db	0xf0		;2a	0x0f0
	db	0xf0		;2b	0x0f0
	db	0xf0		;2c	0x0f0
	db	0xf0		;2d	0x0f0
	db	0xf0		;2e	0x0f0 sprite 7 color
	db	0xff		;2f	points to a byte with 0xff

	db	0xff		;30	points to a byte with 0xff
	db	0xff		;31	points to a byte with 0xff
	db	0xff		;32	points to a byte with 0xff
	db	0xff		;33	points to a byte with 0xff
	db	0xff		;34	points to a byte with 0xff
	db	0xff		;35	points to a byte with 0xff
	db	0xff		;36	points to a byte with 0xff
	db	0xff		;38	points to a byte with 0xff
	db	0xff		;39	points to a byte with 0xff
	db	0xff		;3a	points to a byte with 0xff
	db	0xff		;3b	points to a byte with 0xff
	db	0xff		;3c	points to a byte with 0xff
	db	0xff		;3d	points to a byte with 0xff
	db	0xff		;3e	points to a byte with 0xff
	db	0xff		;3f	points to a byte with 0xff

;===================================================================================
	ALIGN 32
SIDNEY	equ	(_SIDCHIP-memory)

SIDwritetable

	dd	sid1freqlo+SIDNEY		;00
	dd	sid1freqhi+SIDNEY		;01
	dd	sid1pulselo+SIDNEY	;02
	dd	sidipulsehi+SIDNEY	;03	
	dd	sid1conreg+SIDNEY		;04
	dd	sid1attdec+SIDNEY		;05
	dd	sid1susrel+SIDNEY		;06

	dd	sid2freqlo+SIDNEY		;07
	dd	sid2freqhi+SIDNEY		;08
	dd	sid2pulselo+SIDNEY	;09
	dd	sid2pulsehi+SIDNEY	;0a	
	dd	sid2conreg+SIDNEY		;0b
	dd	sid2attdec+SIDNEY		;0c
	dd	sid2susrel+SIDNEY		;0d

	dd	sid3freqlo+SIDNEY		;0e
	dd	sid3freqhi+SIDNEY		;0f
	dd	sid3pulselo+SIDNEY	;10
	dd	sid3pulsehi+SIDNEY	;11	
	dd	sid3conreg+SIDNEY		;12
	dd	sid3attdec+SIDNEY		;13
	dd	sid3susrel+SIDNEY		;14

	dd	sidfclo+SIDNEY		;15
	dd	sidfchi+SIDNEY		;16
	dd	sidresfilt+SIDNEY		;17
	dd	sidmodevol+SIDNEY		;18

	dd	UNDEFINED1			;19
	dd	UNDEFINED1			;1a
	dd	UNDEFINED1			;1b
	dd	UNDEFINED1			;1c

	dd	UNDEFINED1			;1d
	dd	UNDEFINED1			;1e
	dd	UNDEFINED1			;1f

;-----------------------------------------------------------------------------------
	ALIGN 32
SIDreadtable
	dd	UNDEFINED1			;00
	dd	UNDEFINED1			;01
	dd	UNDEFINED1			;02
	dd	UNDEFINED1			;03
	dd	UNDEFINED1			;04
	dd	UNDEFINED1			;05
	dd	UNDEFINED1			;06
	dd	UNDEFINED1			;07

	dd	UNDEFINED1			;08
	dd	UNDEFINED1			;09
	dd	UNDEFINED1			;0a
	dd	UNDEFINED1			;0b
	dd	UNDEFINED1			;0c
	dd	UNDEFINED1			;0d
	dd	UNDEFINED1			;0e
	dd	UNDEFINED1			;0f

	dd	UNDEFINED1			;10
	dd	UNDEFINED1			;11
	dd	UNDEFINED1			;12
	dd	UNDEFINED1			;13
	dd	UNDEFINED1			;14
	dd	UNDEFINED1			;15
	dd	UNDEFINED1			;16
	dd	UNDEFINED1			;17

	dd	UNDEFINED1			;18
	dd	sidpotx+SIDNEY		;19
	dd	sidpoty+SIDNEY		;1a	read only
	dd	sidocs3+SIDNEY		;1b	read only
	dd	sidenv3+SIDNEY		;1c	read only
	dd	UNDEFINED1			;1d
	dd	UNDEFINED1			;1e
	dd	UNDEFINED1			;1f
;----------------------------------------------------------------------------------
; Q) why am I using a jump table?
; A) 1st because this jump table would translate to 4x compare+branch pair, this 
;    would be testing for reverse case. Because all conditional branches are short
;    jumps, this would not be enuf to reach. This messing around would eat up
;    (Intel) clock cycles.
;    2nd if I manage to implement a diskdrive and-or a cart in the spare I/O slots
;    this would obviously add another 1/2 cmp+jcc+jmp .
;    3rd Each "jcc" combination would cause pipe stalls
;    4th this is more readable ( pun intended ) there4 easier to maintain.
;----------------------------------------------------------------------------------
	ALIGN 32
readtable
	dd	readVIC2			;0
	dd	readVIC2			;1
	dd	readVIC2			;2
	dd	readVIC2			;3

	dd	readSID			;4
	dd	readSID			;5
	dd	readSID			;6
	dd	readSID			;7

	dd	readCOLORRAM		;8
	dd	readCOLORRAM		;9
	dd	readCOLORRAM		;a
	dd	readCOLORRAM		;b

	dd	readCIA1			;c
	dd	readCIA2			;d

	dd	readRAM			;e
	dd	readRAM			;f
;------------------------------------------------------------------------------
;same reasoning as readtable... ie cut out "IFs" 
	ALIGN 32
writetable

	dd	writeVIC2			;0
	dd	writeVIC2			;1
	dd	writeVIC2			;2
	dd	writeVIC2			;3

	dd	writeSID			;4
	dd	writeSID			;5
	dd	writeSID			;6
	dd	writeSID			;7

	dd	writeCOLORRAM		;8
	dd	writeCOLORRAM		;9
	dd	writeCOLORRAM		;a
	dd	writeCOLORRAM		;b

	dd	writeCIA1			;c

	dd	writeCIA2			;d

	dd	readRAM			;e
	dd	readRAM			;f
	ALIGN 32
