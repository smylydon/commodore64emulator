extern void RenderBuffer(unsigned int x, unsigned int y);
extern void anIntToBuffer(unsigned int aNumber, unsigned char aMode,unsigned char aBase);
extern void aStringToBuffer(char* aString);
extern void anIntToBuffer(unsigned int aNumber,unsigned char aMode,unsigned char aBase);
extern void Render(unsigned char *gfxchar,unsigned int vline,unsigned int bc);
extern unsigned char GetKeyArr(unsigned char data);
extern unsigned char GetInvKeyArr(unsigned char data);
extern unsigned char GetJoy1(void);
extern unsigned char GetJoy2(void);

#define cWhite 0xffffff
#define cBlack 0x000000

