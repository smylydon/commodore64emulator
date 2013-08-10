

;===================================================================================
[SECTION .data]
;-----------------------------------------------------------------------------------
	ALIGN 32
_VIC2CHIP
times 128	db	0			;mirroring is done by read/write routine
	ALIGN 32
_COLORRAM
times	2048	db	0
	ALIGN 32
;===================================================================================
VIC2writetable
	dd	writeSPRX0				;00	sprite 0 x cord
	dd	writeSPRY0				;01	sprite 0 y cord
	dd	writeSPRX1				;02
	dd	writeSPRY1				;03
	dd	writeSPRX2				;04
	dd	writeSPRY2				;05
	dd	writeSPRX3				;06
	dd	writeSPRY3				;07
	dd	writeSPRX4				;08
	dd	writeSPRY4				;09
	dd	writeSPRX5				;0A
	dd	writeSPRY5				;0B
	dd	writeSPRX6				;0C
	dd	writeSPRY6				;0D
	dd	writeSPRX7				;0E	sprite 7 x cord
	dd	writeSPRY7				;0F	sprite 7 y cord

	dd	writeSPRXX				;10	bit 8 of sprites x cords
	dd	writevicrst8			;11	vic control register 1 write/read latch
	dd	write2raster			;12	vic raster write latch
	dd	writeVICKY				;13
	dd	writeVICKY				;14
	dd	writeVICKY				;15	sprite enable
	dd	writeVICCON2			;16	0x0c0 vic control register 2
	dd	writeVICKY				;17	sprite y expansion
	dd	writeVICKY				;18	0x001 vic memory pointer
	dd	writevicflag			;19	0x070 vic interupt register write latch
	dd	writevicmask			;1a	0x0f0 vic interupt enable
	dd	writeVICKY				;1b	sprite data priority
	dd	writeVICKY				;1c	sprite multicolor
	dd	writeVICKY				;1d	sprite x expantion
	dd	writeviccollS			;1e	sprite-sprite collision
	dd	writeviccollG			;1f	sprite-data collision

	dd	writeVICBDCOL			;20	0x0f0 border color
	dd	writeVICPPRCOL			;21	0x0f0 background color 0
	dd	writeVICKY				;22	0x0f0
	dd	writeVICKY				;23	0x0f0
	dd	writeVICKY				;24	0x0f0
	dd	writeVICKY				;25	0x0f0 sprite multicolor 1
	dd	writeVICKY				;26	0x0f0 sprite multicolor 2
	dd	writeVICKY				;27	0x0f0 sprite 0 color
	dd	writeVICKY				;28	0x0f0
	dd	writeVICKY				;29	0x0f0
	dd	writeVICKY				;2a	0x0f0
	dd	writeVICKY				;2b	0x0f0
	dd	writeVICKY				;2c	0x0f0
	dd	writeVICKY				;2d	0x0f0
	dd	writeVICKY				;2e	0x0f0 sprite 7 color
	dd	writeundefVICKY			;2f	points to a byte with 0xff

	dd	writeundefVICKY			;30	points to a byte with 0xff
	dd	writeundefVICKY			;31	points to a byte with 0xff
	dd	writeundefVICKY			;32	points to a byte with 0xff
	dd	writeundefVICKY			;33	points to a byte with 0xff
	dd	writeundefVICKY			;34	points to a byte with 0xff
	dd	writeundefVICKY			;35	points to a byte with 0xff
	dd	writeundefVICKY			;36	points to a byte with 0xff
	dd	writeundefVICKY			;37	points to a byte with 0xff
	dd	writeundefVICKY			;38	points to a byte with 0xff
	dd	writeundefVICKY			;39	points to a byte with 0xff
	dd	writeundefVICKY			;3a	points to a byte with 0xff
	dd	writeundefVICKY			;3b	points to a byte with 0xff
	dd	writeundefVICKY			;3c	points to a byte with 0xff
	dd	writeundefVICKY			;3d	points to a byte with 0xff
	dd	writeundefVICKY			;3e	points to a byte with 0xff
	dd	writeundefVICKY			;3f	points to a byte with 0xff

;-----------------------------------------------------------------------------------
	ALIGN 32
VIC2readtable
	dd	readSPRX0		;00	sprite 0 x cord
	dd	readSPRY0		;01	sprite 0 y cord
	dd	readSPRX1		;02
	dd	readSPRY1		;03
	dd	readSPRX2		;04
	dd	readSPRY2		;05
	dd	readSPRX3		;06
	dd	readSPRY3		;07
	dd	readSPRX4		;08
	dd	readSPRY4		;09
	dd	readSPRX5		;0A
	dd	readSPRY5		;0B
	dd	readSPRX6		;0C
	dd	readSPRY6		;0D
	dd	readSPRX7		;0E	sprite 7 x cord
	dd	readSPRY7		;0F	sprite 7 y cord	
	

	dd	readVICnorm		;10	bit 8 of sprites x cords
	dd	readVICrast8	;11	only bit 7,0-6 is copied from $d011
	dd	readVICraster	;12	read latch 4 raster @ $d012
	dd	readVICnorm		;13
	dd	readVICnorm		;14
	dd	readVICnorm		;15	sprite enable
	dd	readVICcon2		;16	0x0c0 vic control register 2
	dd	readVICnorm		;17	sprite y expansion
	dd	readVICmempt	;18	0x001 vic memory pointer
	dd	readVICflag		;19	read latch 4 vicirq @ $d019
	dd	readVICmask		;1a	0x0f0 vic interupt enable
	dd	readVICnorm		;1b	sprite data priority
	dd	readVICnorm		;1c	sprite multicolor
	dd	readVICnorm		;1d	sprite x expantion
	dd	readVICcollide	;1e	sprite-sprite collision
	dd	readVICcollide	;1f	sprite-data collision

	dd	readVICcolor	;20	0x0f0 border color
	dd	readVICcolor	;21	0x0f0 background color 0
	dd	readVICcolor	;22	0x0f0
	dd	readVICcolor	;23	0x0f0
	dd	readVICcolor	;24	0x0f0
	dd	readVICcolor	;25	0x0f0 sprite multicolor 1
	dd	readVICcolor	;26	0x0f0 sprite multicolor 2
	dd	readVICcolor	;27	0x0f0 sprite 0 color
	dd	readVICcolor	;28	0x0f0
	dd	readVICcolor	;29	0x0f0
	dd	readVICcolor	;2a	0x0f0
	dd	readVICcolor	;2b	0x0f0
	dd	readVICcolor	;2c	0x0f0
	dd	readVICcolor	;2d	0x0f0
	dd	readVICcolor	;2e	0x0f0 sprite 7 color
	dd	readVICundef	;2f	points to a byte with 0xff
		
	dd	readVICundef	;30	points to a byte with 0xff
	dd	readVICundef	;31	points to a byte with 0xff
	dd	readVICundef	;32	points to a byte with 0xff
	dd	readVICundef	;33	points to a byte with 0xff
	dd	readVICundef	;34	points to a byte with 0xff
	dd	readVICundef	;35	points to a byte with 0xff
	dd	readVICundef	;36	points to a byte with 0xff
	dd	readVICundef	;37	points to a byte with 0xff
	dd	readVICundef	;38	points to a byte with 0xff
	dd	readVICundef	;39	points to a byte with 0xff
	dd	readVICundef	;3a	points to a byte with 0xff
	dd	readVICundef	;3b	points to a byte with 0xff
	dd	readVICundef	;3c	points to a byte with 0xff
	dd	readVICundef	;3d	points to a byte with 0xff
	dd	readVICundef	;3e	points to a byte with 0xff
	dd	readVICundef	;3f	points to a byte with 0xff
	dd	readVICundef
	ALIGN 32