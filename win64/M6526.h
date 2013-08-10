extern void M6526HardReset(void);
extern void M6526SetCycles(unsigned int period);
extern void M6526DumpCiaChip(unsigned char id);
extern char M6526ReadCIA(unsigned char address,unsigned char id);
extern void M6526WriteCIA(unsigned char data,unsigned char address, unsigned char id);
extern void M6526Execute(void);
extern void M6526LoadState(struct ioState *p);
extern void M6526SaveState(struct ioState *p);
