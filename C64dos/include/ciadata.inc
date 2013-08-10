

;===================================================================================
[SECTION .data]
;-----------------------------------------------------------------------------------
_CIACHIP1
times 64	db	0
	ALIGN 32
_CIACHIP2
times 64	db	0
	ALIGN 32
TODperiod	dd	102272
	ALIGN 32
;===================================================================================

;-----------------------------------------------------------------------------------
CIA1writetable
	dd	write2CIA1pra
	dd	write2CIAprb
	dd	write2CIAddra
	dd	write2CIAddrb
	dd	write2CIAddrb
	dd	write2TAhigh
	dd	write2CIAprb
	dd	write2TBhigh

	dd	write2TODtens
	dd	write2TODsecs
	dd	write2TODmins
	dd	write2TODhours
	dd	write2CIAddrb
	dd	write2cia1icr
	dd	write2ciacra
	dd	write2ciacrb
	ALIGN 32
CIA2writetable
	dd	write2CIA2pra
	dd	write2CIAprb
	dd	write2CIAddra
	dd	write2CIAddrb
	dd	write2CIAddrb
	dd	write2TAhigh
	dd	write2CIAddrb
	dd	write2TBhigh

	dd	write2TODtens
	dd	write2TODsecs
	dd	write2TODmins
	dd	write2TODhours
	dd	write2CIAsdr
	dd	write2cia2icr
	dd	write2ciacra
	dd	write2ciacrb
	ALIGN 32	

;===================================================================================


CIA1readtable
	dd	readCIA1pra
	dd	readCIA1prb
	dd	readCIAddra
	dd	readCIAddrb
	dd	readCIAtal
	dd	readCIAtah
	dd	readCIAtbl
	dd	readCIAtbh

	dd	readCIATODtens
	dd	readCIATODsecs
	dd	readCIATODmins
	dd	readCIATODhour
	dd	readCIAddra
	dd	readCIA1icr
	dd	readCIAddra
	dd	readCIAddra
	ALIGN 32

;-----------------------------------------------------------------------------------
	ALIGN 32
CIA2readtable
	dd	readCIA2pra
	dd	readCIA2prb
	dd	readCIAddra
	dd	readCIAddrb
	dd	readCIAtal
	dd	readCIAtah
	dd	readCIAtbl
	dd	readCIAtbh

	dd	readCIATODtens
	dd	readCIATODsecs
	dd	readCIATODmins
	dd	readCIATODhour
	dd	readCIAddra
	dd	readCIA2icr
	dd	readCIAddra
	dd	readCIAddra
	ALIGN 32
_keyarr
times 0x1ff	db	0
	ALIGN 32
_invkeyarr
times 0x1ff db 0
	ALIGN 32	
	