
;===================================================================================

SIGN			equ	0x80
OVERFLOW		equ	0x40
UNDEFINED		equ	0x20
BREAK			equ	0x10
DECIMAL		equ	0x08
INTERUPT		equ	0x04
ZERO			equ	0x02
CARRY			equ	0x01

IMP			equ	0
ACC			equ	1
IMM			equ	2
ZPG			equ	3
ZPGX			equ	4
ZPGY			equ	5
ABS			equ	6
ABSX			equ	7
ABSY			equ	8
INDX			equ	9
INDY			equ	10
INDI			equ	11
REL			equ	12

;-----------------------------------------------------------------------------------
;Memory management equates...
LORAMB		equ	1
HIRAMB		equ	2
CHARENB		equ	4

COLORRAMbase	equ	0xd800

;-----------------------------------------------------------------------------------
;note sid1/2/3 means sid voice x, v1/vc1 etc would be confused with vic1xxx etc 

SIDbase		equ	0x0d400

;NB sid1freq - sidmodevol is write only registers

sid1freqlo		equ	0
sid1freqhi		equ	1
sid1pulselo		equ	2
sidipulsehi		equ	3		
sid1conreg		equ	4		
sid1attdec		equ	5
sid1susrel		equ	6

sid2freqlo		equ	7
sid2freqhi		equ	8
sid2pulselo		equ	9
sid2pulsehi		equ	10
sid2conreg		equ	11		
sid2attdec		equ	12
sid2susrel		equ	13

sid3freqlo		equ	14
sid3freqhi		equ	15
sid3pulselo		equ	16
sid3pulsehi		equ	17
sid3conreg		equ	18		
sid3attdec		equ	19
sid3susrel		equ	20

sidfclo		equ	21
sidfchi		equ	22
sidresfilt		equ	23
sidmodevol		equ	24

sidpotx		equ	25			;read only
sidpoty		equ	26			;read only
sidocs3		equ	27			;read only
sidenv3		equ	28			;read only

