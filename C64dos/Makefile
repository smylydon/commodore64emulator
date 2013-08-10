CC = gcc
LD = gcc

DEFS = 
CFLAGS = -fomit-frame-pointer -m486 -Wall 

OBJS = CBM64.o gui.o vicky.o monam.o pcio.o Sidney.o modes.o M6569.o M6526.o M6510.o tape.o joystick.o

all: CBM64.exe

CBM64.exe: $(OBJS) 
	$(LD)  -s -o CBM64.exe $(OBJS) 

CBM64.o: CBM64.c
	$(CC) $(DEFS) $(CFLAGS) -o CBM64.o -c CBM64.c
		
gui.o: gui.c
	$(CC) $(DEFS) $(CFLAGS) -o gui.o -c gui.c

vicky.o: vicky.c
	$(CC) $(DEFS) $(CFLAGS) -o vicky.o -c vicky.c
	
monam.o: monam.c
	$(CC) $(DEFS) $(CFLAGS) -o monam.o -c monam.c
		
pcio.o: gui.c
	$(CC) $(DEFS) $(CFLAGS) -o pcio.o -c pcio.c
		
Sidney.o: Sidney.c
	$(CC) $(DEFS) $(CFLAGS) -o Sidney.o -c Sidney.c

modes.o: modes.c
	$(CC) $(DEFS) $(CFLAGS) -o modes.o -c modes.c
		
tape.o: tape.c
	$(CC) $(DEFS) $(CFLAGS) -o tape.o -c tape.c
		
joystick.o: joystick.c
	$(CC) $(DEFS) $(CFLAGS) -o joystick.o -c joystick.c
	
M6569.o: M6569.asm
	nasm -f coff M6569.asm
		
M6526.o: M6526.asm
	nasm -f coff M6526.asm
		
M6510.o: M6510.asm
	nasm -f coff M6510.asm


