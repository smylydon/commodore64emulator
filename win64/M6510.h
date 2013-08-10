// CPU Defines and Structures
extern void M6510HardReset(void);
extern void M6510SetCycles(unsigned int mycycles);
extern int M6510ReadCycles(void);
extern void M6510Execute(void);
extern void M6510Dump(void);
extern void M6510Pause(void);
extern unsigned char M6510trace(void);
extern int M6510Tracer(void);
extern void M6510LoadCpu(unsigned int *fileimage);
extern void M6510SaveCpu(unsigned int *fileimage);
