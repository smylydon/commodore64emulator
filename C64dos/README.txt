------------------------------------------------------------------------------------------
DISCLAIMER
----------
THIS EMULATOR AND ALL ITS SOURCE CODE IS FREEWARE. NO PART OF THIS EMULATOR, HEREBY
REFERED TO AS EMU OR MNS64, MAY BE DISTRIBUTED FOR ANY FINANCIAL GAIN OR VENTURE,
EXCEPT ON MAGAZINE COVER DISKS. MNS64 PACKAGE MUST BE DISTRIBUTED IN ITS ENTIRITY,
INCLUDING THIS DOCUMENT IN ITS ENTIRITY OR ANY THAT SUPERCEED IT.

YOUR FREE TO ALTER AND USE THE SOURCE IN YOUR OWN FREEWARE PRODUCTIONS, YOU'RE NOT
EXPECTED TO PRINT ANY ACKNOLEDGEMENT OF ITS USE, HOWEVER A MENTION IS ALWAYS
APPRECIATED.( AN EMAIL OR A FREE COPY OF YOUR WORK IS ALWAYS WELCOME )

ALL COPYWRITES,TRADEMAKES AND PATENTS, EXPLICTLY OR IMPLICITLY REFERED TO BY
THIS PACKAGE, IS FULLY ACKNOLEDGED.

YOU ARE REMINDED THAT THE USE OF ANY PART OF THIS PACKAGE IS DONE SO AT YOUR OWN
RISK, AND THE POSSESION OF THIS PACKAGE IS AN ACCEPTENCE OF THE TERMS AND CONDITIONS.

------------------------------------------------------------------------------------------
INTRODUCTION
------------

Welcome to Midnight (Chopper) Surfers C64 emulator ver. 2.5. As you can see this
emulator is still in the WIP stage. So there are still quite a few things lacking,
such as a 1541 floppy emulation. Most games run fine though, of course if a game refuses
to run you should try and change the cpu and cia cycles from the chips menu.

Why another C64 emulator? Well I originally wanted to do an Amiga emulator, but common
sense told me that you dont pick an Amiga for your first emulator. So I thought I'd
do a C64 emulator (Oh dear!), then use the tools and experience gained from that to do
an Amiga.

I'm undecided whether to start an Amiga version of this, or work on a Genisys or Amiga
emulator. However work shall continue on this project, so some input and advice would
be nice guys.

usage		 	:	double click Cbm64.exe from windows explorer, or 
				you can type Cbm64 at dos prompt. Emu runs in Win95
				Dos box, or in pure Dos with dos-extender.
				MNS64 (sound off ) runs between 150%-160% with verticle
				blank turned on and between 190%-240% with no vblanking.
				By default FM sound is turned on, unless you dont
				have an Adlib compatable card.
		
				F8	filerequestor, you must type RUN afterwards.
				F9	Sprite dump,press F9 again to resume EMU
				F10	GUI
				F11	Sound on/off
	     			F12	for Hard Reset of C64. the is no Soft Reset.
	      			Esc	to quit emulator (anytime).

Keyboard		:	use keypad for cursors(2&6), exponent(4) and home(7).

Files (main)		:	M6510.asm,opcodes.inc,admodes.inc --- cpu emulation
				M6526.asm                         --- cia1&2  emulation
				M6569.asm+Vicky.c                 --- VIC2 gfx emulation.

				Requires Nasm and DJGPP to compile.

------------------------------------------------------------------------------------------
COMPATIBILITY
-------------
The section shows which games that I have tested. The comments are a simply guide to
getting the games to run, or know bugs. Of course theres always going to to be some
gfx glitches; don't forget this is a line base emulation so collision detection is only
reported once every rasterline.

A point to note is that I suspect my SHA SHS SHY and SHX implementation for alot of the 
failures and crashes; but some games are just very sensitive and require exact settings
for the cia and cpu cycling. 

key	RUNS ALWAYS means a well behaved game, that will run with any settings
	RUNS DEFAULT stick to default settings for best results
	

720grad		- RUNS ALWAYS, ingame Y scroll is buggy
Battle Field	- RUNS ALWAYS
Bubble Bobble	- RUNS DEFAULT
Buck Rodgers	- RUNS ALWAYS
Commando	- RUNS DEFAULT
Congo Bongo	- RUNS DEFAULT
Frogger		- runs decruncher sometimes, very tempremental game. HAS NEVER RUN!!
Green Beret	- RUNS ALWAYS slight raster interrupt problem on title screen
Ikari Warriors	- RUNS ALWAYS 
Int Karate +	- RUNS DEFAUT sprite glitches. minor raster interrupt glitches
Int Karate 2	- RUNS ALWAYS sprite glitches. minor raster interrupt glitches
IO		- RUNS DEFAULT - intro is colours are wrong way round.
Jet Set Willy	- need 1541 to test it
Jet Set Willy2	- RUNS ALWAYS
Manic Miner	- RUNS ALWAYS
Pitstop 1	- RUNS ALWAYS note change to joyport 1
Pitstop 2	- RUNS ALWAYS occasional trivial sprite glitches
Polespos	- UNEXPLAINED FAULT used to be well behaved game 
Rygar		- RUNS DEFAULT
Spellbound	- RUNS ALWAYS
Spindizy	- RUNS ALWAYS

------------------------------------------------------------------------------------------
GUI
---
The gui automatically pauses the emulatior, simply use cursor keys and enter to navigate
around menus. Hopefully I should have implemented mouse control before I finish this text.
Not all menu options have been implemented.

Pressing F10 will descend the GUI tree. WARNING ESC WILL KILL EMU ON ALL GUI's.

games		This allows the user to bring up filerequesters for different tape images.
		By default pressing f8, filerequester searches for t64 images. If you want
		to load a p00 files, you have to select GAMES menu, then p00. After this
		the default image type will be p00, so pressing f8 from now on until you
		change it, will bring up p00 images. likewise the same is true for the 
		C64 format and my own "SAVE STATE" format ( see chip menu ).

joyport		This allows you to change which joyport to use on the C64, by simply 
		pressing enter. To calibrate your joystick, just wiggle (swirl) it about 
		whilest the emu is running. Its a good idea to calibrate the joystick
		when you on basic prompt ("CBM64 BASIC...READY."), this will stop it
		from interferring with the keyboard emulation.

chips		Selecting this menu brings up the chips menu, from which you can go to the
		the machine code monitor, tweak cpu and cia timings.
		Pressing enter on cpu or cia, will cycle 70..69..61..60..70.. etc thus
		setting the required cycles per rasterline.
		You'll also find the LOAD STATE and SAVE STATE options here. You must have
		at least one file named something.st in the emu directory before you can
		save. Just select "something.st" in filerequester to save. You should
		find 10 slots already saved for you.
		SAVE STATE, saves the state of machine ( chips & memory ) verbatum ie
		word for word, byte for byte. Thus everything is captured to file, so
		you can resume EXACTLY where you left off.
		Obviously LOAD STATE, is the opposite of SAVE STATE.

screen		The screen gui allows you to change palettes aswell as the gamma setting.
		You can always return to origin settings by selecting default on this gui.
		This is also where you change the PC Screen Resolution.

		NOTE: default on this menu does not default the entire system, just the
		colour.

default		This will will restore palette,cpu and cia cycling to 63 cycles/rasterline.

help		simple help.

about		is about, press F10 when u finised reading the blurb.

------------------------------------------------------------------------------------------
HARDWARE
--------
The following details are what has been done:

M6510 (cpu)		:	(src 100% asm)
					official and undocumented opcodes DONE
					checked with test suit2.14

M6526 (i/o)		:	(src 100% asm)
					Timer A&B count cpu cyles
					No TB counts TA.
					improved joystick keyboard handling

M6567 (gfx)		:	(src 60% asm, 40% C)
					Standard Text Mode (STD)
					Extended Text Mode (ECM)
					Multicolor Text Mode (MCM)
					Standard Bitmap Mode (BMM)
					Multicolor Bitmap Mode (MCBMM)
					
					Invalid Text Mode (INV)
					Invalid Bitmap Mode 1 (IBMM1)
					Invalid Bitmap Mode 2 (IBMM2)
					Idle mode--INV is used for now
					
					Sprites :
						Single Color Mode
						Multicolor Mode

						Sprite X-expansion
						Sprite Y-expansion
						
						sprite2sprite priority
						Sp2Gfx priority
						
						sprite2sprite collision detection
						sprite2gfx collion detection
						
M6581 (snd)		:	SidFM emulation
				By Paul TOTH 97-98 <tothpaul@multimania.com>
				http://www.multimania.com/tothpaul (fr & us)

				I simply converted it verbatum from PASCAL
				to C and a function to ASM.

------------------------------------------------------------------------------------------
HISTORY
-------
version 2.5	20 AUG 01. Extensive work done, completed X-Mode code which
		slowed the Emu speed to between 50-70% of a C64. Fixed by
		using inline assembly for actual rendering, and by re-arranging
		VIC 2 code. The rest of work centred on polishing up the code.
		For the moment you cant switch modes from 360x256, this will
		be a feature in the next update.
		M6510 99%, VIC2 98%, CIA1&2 75%, SID 50%, GUI 95%,MONITOR 99%.

version 2.0	major improvements, all mnemonics checked with test suite 2.14
		SidFM emulation added.Very primitive X-Mode code added but not
		used.New keyboard and joystick emulation method, now Commando
		and Manic Manic accepts joystick signals.
		M6510 99%, VIC2 98%, CIA1&2 75%, SID 50%, GUI 95%,MONITOR 99%.

version 1.5	same story as version 1.4, Miner Willy jumps for no reason on
		Manic Miner, making game unplayable. Commando still wont start.
		M6510 99%, VIC2 95%, CIA1&2 75%, SID 0%, GUI 95%,MONITOR 99%.

version 1.4	variation of v1.3, no real improvement, Commando wont start,
		because game wont accept joystick fire button signal.
		M6510 99%, VIC2 95%, CIA1&2 75%, SID 0%, GUI 95%,MONITOR 99%.

version 1.3	major rewrite of gui, am much happier with the code. Removed some
		hidden bugs from Monam. Begun converting VIC2 functions into
		inline ASM. Removed some bugs from M6510, Bubble now runs again :-).
		M6510 99%, VIC2 95%, CIA1&2 75%, SID 0%, GUI 95%,MONITOR 99%.

version 1.2	WIP release, runs 90% of games, improved VIC2 with X&Y scrolling
		split screen games work. Bubble don't run now :-(.
		M6510 99%, VIC2 90%, CIA1&2 75%, SID 0%, GUI 70%,MONITOR 99%.


version 1.0	private WIP release to friends for playtesting, runs Bubble bobble and
		80% of games tested. Has regular sprite&background glitches on split
		screen games. No Y scrolling!!.
		M6510 99%, VIC2 50, CIA 1&2 75%, SID 0%, GUI 0%,MONITOR 90%.
------------------------------------------------------------------------------------------
