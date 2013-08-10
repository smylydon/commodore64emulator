struct ioState {
    unsigned char mem[0x10000];
    unsigned char pokevic[0x40];
    unsigned char cia1[0x40];
    unsigned char cia2[0x40];
    unsigned char sid[0x40];
    unsigned char colram[0x800];
    unsigned int cpu[0xff];
    unsigned int peekvic[0xff];
};

extern void BusResetBus(void);
extern unsigned char BusReadCiaB(void);
extern void BusUpDateVicII(void);
extern void BusCPUnoBadline(void);
extern void BusCPUyesBadline(void);
extern unsigned int BusReadCPUCycles(void);
extern void BusSetCPUCycles(unsigned int cycles);
extern unsigned int BusNMI(void);
extern unsigned int BusIRQ(void);
extern void BusClearIRQ(unsigned int MIRQ);
extern void BusSetIRQ(unsigned int MIRQ);
extern void BusSetNMI(void);
extern void BusClearNMI(void);
extern unsigned char BusReadByte(unsigned int address);
extern unsigned int BusReadWord(unsigned int address);
extern void BusWriteByte(unsigned char data,unsigned int address);
extern unsigned char BusPopByteStack(unsigned char address);
extern unsigned int BusPopWordStack(unsigned char address);
extern void BusPushByteStack(unsigned char data,unsigned char address);
extern void BusPushWordStack(unsigned int word,unsigned char address);
extern void BusDump(unsigned int address,unsigned int size);
extern unsigned int BusReadInst(unsigned int address);
extern unsigned int BusReadBounds(unsigned int address);
extern void BusWriteRam(unsigned char data,unsigned int address);
extern unsigned char BusReadRam(unsigned int address);
extern unsigned char BusReadColorRam(unsigned char data);
extern unsigned char BusReadChar(unsigned int data);
extern void BusLoadState(struct ioState *p);
extern void BusSaveState(struct ioState *p);


