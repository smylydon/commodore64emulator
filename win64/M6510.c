
#include "CBM64.h"
#include "AddressBus.h"
#include "Disa.h"
#define Nflag 0x80
#define Vflag 0x40
#define Uflag 0x20
#define Bflag 0x10
#define Dflag 0x08
#define Iflag 0x04
#define Zflag 0x02
#define Cflag 0x01
#define NNZCVF 0xff-(Nflag|Zflag|Cflag|Vflag)
#define NNZCF 0xff-(Nflag|Zflag|Cflag)
#define NNZF 0xff-(Nflag|Zflag)

extern void M6526DumpCiaChip(unsigned int id);
extern void M6569Dump(void);

struct BYTES {
    unsigned char lo;
    unsigned char hi;
    unsigned char dm1;
    unsigned char dm2;
};

union REGISTER
{
    struct BYTES r;
    unsigned int value;
};

unsigned char flags[]={
                          0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0
                          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //1
                          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //2
                          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //3
                          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //4
                          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //5
                          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //6
                          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //7

                          0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, //8
                          0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, //9
                          0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, //A
                          0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, //B
                          0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, //C
                          0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, //D
                          0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, //E
                          0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80  //F
                      };

void M6510HardReset(void);
void M6510SetCycles(unsigned int mycycles);
unsigned int M6510ReadCycles(void);
void M6510Execute(void);
void M6510Process(void);
void M6510Dump(void);

union REGISTER psp,areg,xreg,yreg,pc1,pc2,pc3,psr;
union REGISTER operand,address,aux1,aux2;
unsigned int inst,clock,cycles,count,pause;
unsigned char nmemonic,trace;

void M6510HardReset(void) {
    areg.value=0;
    xreg.value=0;
    yreg.value=0;      //address registers
    operand.value=0;
    address.value=0;
    aux1.value=aux2.value=0;
    pc1.value=BusReadWord(0xfffc);
    pc2.value=pc1.value;   //program counters
    psp.value=0xff;        //stack pointer
    psr.value=0x00;         //status register
    psr.r.lo|=Uflag;
    inst+=1;          //instructions executed
    clock+=7;          //total number of cycles
    cycles=0;          //cycles to execute
    count=0;           //count of cycles executed
    pause=0;
    trace=0;
}

int M6510trace(void) {
    return trace;
}

void M6510SetCycles(unsigned int mycycles) {
    cycles=mycycles;
}

unsigned int M6510ReadCycles(void) {
    return cycles;
}

void DoFlags(unsigned int test) {
    if(psr.value&test) {
        aStringToBuffer("1 ");
    }
    else {
        aStringToBuffer("- ");
    }
}

void M6510Dump(void) {
    aStringToBuffer("M6510 Status \n\n");
    aStringToBuffer("N V - B D I Z C\n");
    DoFlags(Nflag);
    DoFlags(Vflag);
    DoFlags(Uflag);
    DoFlags(Bflag);
    DoFlags(Dflag);
    DoFlags(Iflag);
    DoFlags(Zflag);
    DoFlags(Cflag);
    aStringToBuffer("\n\nPC1    :");
    anIntToBuffer(pc1.value,2,1);
    aStringToBuffer("\nPC2    :");
    anIntToBuffer(pc2.value,2,1);
    aStringToBuffer("\nPC3    :");
    anIntToBuffer(pc3.value,2,1);
    aStringToBuffer("\nSR     :");
    anIntToBuffer(psp.value,2,1);
    aStringToBuffer("\nAR     :");
    anIntToBuffer(areg.value,1,1);
    aStringToBuffer("\nXR     :");
    anIntToBuffer(xreg.value,1,1);
    aStringToBuffer("\nYR     :");
    anIntToBuffer(yreg.value,1,1);
    aStringToBuffer("\ncycles  :");
    anIntToBuffer(clock,0,0);
    aStringToBuffer("\n\n0038 0037  :");
    anIntToBuffer(BusReadByte(0x037),1,1);
    anIntToBuffer(BusReadWord(0x038),1,1);
    aStringToBuffer("\n\n002c 002b  :");
    anIntToBuffer(BusReadByte(0x07a),1,1);
    anIntToBuffer(BusReadWord(0x07b),1,1);
    aStringToBuffer("\n\n");
};

#include "Admodes.h"
#include "Opcodes.h"

void M6510Process(void) {
    pc1.value&=0xffff;
    if(BusNMI()) {
        BusClearNMI();
        pc2.value=pc1.value;
        psr.r.lo&=(0xff-Bflag);
        psr.r.lo|=Uflag;
        BusPushWordStack(pc1.value,psp.r.lo);
        psp.r.lo-=2;
        psr.r.lo|=Uflag;
        BusPushByteStack(psr.r.lo,psp.r.lo);
        psr.r.lo|=Iflag;
        psp.r.lo--;
        pc1.value=BusReadWord(0xfffa);
        clock+=7;
        count+=7;
        inst++;
    }

    if((BusIRQ())&&((psr.r.lo&Iflag)==0)) {
        pc2.value=pc1.value;
        psr.r.lo&=(0xff-Bflag);
        BusPushWordStack(pc1.value,psp.r.lo);
        psp.r.lo-=2;
        psr.r.lo|=Uflag;
        BusPushByteStack(psr.r.lo,psp.r.lo);
        psp.r.lo--;
        psr.r.lo|=Iflag;
        pc1.value=BusReadWord(0xfffe);
        clock+=7;
        count+=7;
        inst++;
    }

    nmemonic=BusReadByte(pc1.value);
    pc3.value=pc2.value;
    pc2.value=pc1.value;
    inst++;
    pc1.value++;
    switch(nmemonic) {
        ///////ADC
        case 0x69:
        ReadImm();
        ADC();
        break;
        case 0x65:
        ReadZpg();
        ADC();
        break;
        case 0x75:
        ReadZpgX();
        ADC();
        break;
        case 0x6d:
        ReadAbs();
        ADC();
        break;
        case 0x7d:
        ReadAbsX();
        ADC();
        break;
        case 0x79:
        ReadAbsY();
        ADC();
        break;
        case 0x61:
        ReadIndX();
        ADC();
        break;
        case 0x71:
        ReadIndY();
        ADC();
        break;
        ///////ANC
        case 0x0b:
        case 0x2b:
        //ANC - NKHOSI SEKELELE AFRICA - sorry :-)
        ReadImm();
        ANC();
        break;
        ///////AND
        case 0x29:
        ReadImm();
        AND();
        break;
        case 0x25:
        ReadZpg();
        AND();
        break;
        case 0x35:
        ReadZpgX();
        AND();
        break;
        case 0x2d:
        ReadAbs();
        AND();
        break;
        case 0x3d:
        ReadAbsX();
        AND();
        break;
        case 0x39:
        ReadAbsY();
        AND();
        break;
        case 0x21:
        ReadIndX();
        AND();
        break;
        case 0x31:
        ReadIndY();
        AND();
        break;
        ///////ANE
        case 0x8b:
        ReadImm();
        ANE();
        break;
        ///////ASL
        case 0x0A:
        ASLA();
        break;
        case 0x06:
        StoreRWMZpg();
        ASLM();
        break;
        case 0x16:
        StoreRWMZpgX();
        ASLM();
        break;
        case 0x0e:
        StoreRWMAbs();
        ASLM();
        break;
        case 0x1e:
        StoreRWMAbsX();
        ASLM();
        break;
        ///////ARR
        case 0x6b:
        ReadImm();
        ARR();
        break;
        ///////ASR
        case 0x4b:
        ReadImm();
        ASR();
        break;
        ///////BRANCH INSTRUCTIONS

        case 0x90:
        BCC();
        break;
        case 0xb0:
        BCS();
        break;
        case 0xf0:
        BEQ();
        break;
        case 0x30:
        BMI();
        break;
        case 0xd0:
        BNE();
        break;
        case 0x10:
        BPL();
        break;
        case 0x50:
        BVC();
        break;
        case 0x70:
        BVS();
        break;
        ///////BIT
        case 0x24:
        ReadZpg();
        BIT();
        break;
        case 0x2c:
        ReadAbs();
        BIT();
        break;
        ///////BRK
        case 0x00: //00 BRK
        BRK();
        break;

        ///////CLEAR FLAGS
        case 0x18:
        CLC();
        break;
        case 0xd8:
        CLD();
        break;
        case 0x58:
        CLI();
        break;
        case 0xb8:
        CLV();
        break;
        ///////CMP
        case 0xc9:
        ReadImm();
        CMP();
        break;
        case 0xc5:
        ReadZpg();
        CMP();
        break;
        case 0xd5:
        ReadZpgX();
        CMP();
        break;
        case 0xcd:
        ReadAbs();
        CMP();
        break;
        case 0xdd:
        ReadAbsX();
        CMP();
        break;
        case 0xd9:
        ReadAbsY();
        CMP();
        break;
        case 0xc1:
        ReadIndX();
        CMP();
        break;
        case 0xd1:
        ReadIndY();
        CMP();
        break;

        ///////CPX
        case 0xe0:
        ReadImm();
        CPX();
        break;
        case 0xe4:
        ReadZpg();
        CPX();
        break;
        case 0xec:
        ReadAbs();
        CPX();
        break;
        ///////CPY
        case 0xc0:
        ReadImm();
        CPY();
        break;
        case 0xc4:
        ReadZpg();
        CPY();
        break;
        case 0xcc:
        ReadAbs();
        CPY();
        break;
        ///////DCP
        case 0xc7:
        StoreRWMZpg();
        DCP();
        break;
        case 0xd7:
        StoreRWMZpgX();
        DCP();
        break;
        case 0xcf:
        StoreRWMAbs();
        DCP();
        break;
        case 0xdf:
        StoreRWMAbsX();
        DCP();
        break;
        case 0xdb:
        StoreRWMAbsY();
        DCP();
        break;
        case 0xc3:
        StoreIndX();
        DCP();
        break;
        case 0xd3:
        StoreIndY();
        DCP();
        break;
        ///////DEC
        case 0xc6:
        StoreRWMZpg();
        DEC();
        break;
        case 0xd6:
        StoreRWMZpgX();
        DEC();
        break;
        case 0xce:
        StoreRWMAbs();
        DEC();
        break;
        case 0xde:
        StoreRWMAbsX();
        DEC();
        break;
        ///////DEX
        case 0xca:
        DEX();
        break;
        ///////DEY
        case 0x88:
        DEY();
        break;
        ///////EORA
        case 0x49:
        ReadImm();
        EORA();
        break;
        case 0x45:
        ReadZpg();
        EORA();
        break;
        case 0x55:
        ReadZpgX();
        EORA();
        break;
        case 0x4d:
        ReadAbs();
        EORA();
        break;
        case 0x5d:
        ReadAbsX();
        EORA();
        break;
        case 0x59:
        ReadAbsY();
        EORA();
        break;
        case 0x41:
        ReadIndX();
        EORA();
        break;
        case 0x51:
        ReadIndY();
        EORA();
        break;
        ///////INC
        case 0xe6:
        StoreRWMZpg();
        INC();
        break;
        case 0xf6:
        StoreRWMZpgX();
        INC();
        break;
        case 0xee:
        StoreRWMAbs();
        INC();
        break;
        case 0xfe:
        StoreRWMAbsX();
        INC();
        break;
        ///////INX
        case 0xe8:
        INX();
        break;
        ///////INY
        case 0xc8:
        INY();
        break;
        ///////ISB
        case 0xe7:
        StoreRWMZpg();
        ISB();
        break;
        case 0xf7:
        StoreRWMZpgX();
        ISB();
        break;
        case 0xef:
        StoreRWMAbs();
        ISB();
        break;
        case 0xff:
        StoreRWMAbsX();
        ISB();
        break;
        case 0xfb:
        StoreRWMAbsY();
        ISB();
        break;
        case 0xe3:
        StoreIndX();
        ISB();
        break;
        case 0xf3:
        StoreIndY();
        ISB();
        break;
        ///////JMP
        case 0x4c:
        JMPAbs();
        break;
        case 0x6c:
        JMPInd();
        break;
        ///////JSR
        case 0x20:
        JSR();
        break;
        ///////LAS
        case 0xbb:
        ReadAbsY();
        LAS();
        break;
        ///////LAX
        case 0xa7:
        ReadZpg();
        LAX();
        break;
        case 0xb7:
        ReadZpgY();
        LAX();
        break;
        case 0xaf:
        ReadAbs();
        LAX();
        break;
        case 0xbf:
        ReadAbsY();
        LAX();
        break;
        case 0xa3:
        ReadIndX();
        LAX();
        break;
        case 0xb3:
        ReadIndY();
        LAX();
        break;
        ///////LDA
        case 0xa9:
        ReadImm();
        LDA();
        break;
        case 0xa5:
        ReadZpg();
        LDA();
        break;
        case 0xb5:
        ReadZpgX();
        LDA();
        break;
        case 0xad:
        ReadAbs();
        LDA();
        break;
        case 0xbd:
        ReadAbsX();
        LDA();
        break;
        case 0xb9:
        ReadAbsY();
        LDA();
        break;
        case 0xa1:
        ReadIndX();
        LDA();
        break;
        case 0xb1:
        ReadIndY();
        LDA();
        break;
        ///////LDX
        case 0xa2:
        ReadImm();
        LDX();
        break;
        case 0xa6:
        ReadZpg();
        LDX();
        break;
        case 0xb6:
        ReadZpgY();
        LDX();
        break;
        case 0xae:
        ReadAbs();
        LDX();
        break;
        case 0xbe:
        ReadAbsY();
        LDX();
        break;
        ///////LDY
        case 0xa0:
        ReadImm();
        LDY();
        break;
        case 0xa4:
        ReadZpg();
        LDY();
        break;
        case 0xb4:
        ReadZpgX();
        LDY();
        break;
        case 0xac:
        ReadAbs();
        LDY();
        break;
        case 0xbc:
        ReadAbsX();
        LDY();
        break;
        ///////LSR
        case 0x4a:
        LSRA();
        break;
        case 0x46:
        StoreRWMZpg();
        LSRM();
        break;
        case 0x56:
        StoreRWMZpgX();
        LSRM();
        break;
        case 0x4e:
        StoreRWMAbs();
        LSRM();
        break;
        case 0x5e:
        StoreRWMAbsX();
        LSRM();
        break;
        ///////LXA
        case 0xab:
        ReadImm();
        LXA();
        break;
        ///////NOPS
        case 0x80:
        case 0x82:
        case 0x89:
        case 0xc2:
        case 0xe2:
        ReadImm();
        NOPund();
        case 0x1a:
        case 0x3a:
        case 0x5a:
        case 0x7a:
        case 0xda:
        case 0xea:
        case 0xfa:
        NOP();
        break;
        case 0x04:
        case 0x44:
        case 0x64:
        case 0xd4:
        case 0xf4:
        ReadZpg();
        NOPund();
        break;
        case 0x14:
        case 0x34:
        case 0x54:
        case 0x74:
        ReadZpgX();
        NOPund();
        break;
        //          case 0x0c:
        case 0x1c:
        case 0x3c:
        case 0x5c:
        case 0x7c:
        case 0xdc:
        case 0xfc:
        ReadAbsX();
        NOPund();
        break;
        case 0x0c:
        ReadAbs();
        NOPund();
        break;
        ///////ORA
        case 0x09:
        ReadImm();
        ORA();
        break;
        case 0x05:
        ReadZpg();
        ORA();
        break;
        case 0x15:
        ReadZpgX();
        ORA();
        break;
        case 0x0d:
        ReadAbs();
        ORA();
        break;
        case 0x1d:
        ReadAbsX();
        ORA();
        break;
        case 0x19:
        ReadAbsY();
        ORA();
        break;
        case 0x01:
        ReadIndX();
        ORA();
        break;
        case 0x11:
        ReadIndY();
        ORA();
        break;
        ///////PUSHS AND POPS
        case 0x48:
        PHA();
        break;
        case 0x08:
        PHP();
        break;
        case 0x68:
        PLA();
        break;
        case 0x28:
        PLP();
        break;
        ///////RLA
        case 0x27:
        StoreRWMZpg();
        RLA();
        break;
        case 0x37:
        StoreRWMZpgX();
        RLA();
        break;
        case 0x2f:
        StoreRWMAbs();
        RLA();
        break;
        case 0x3f:
        StoreRWMAbsX();
        RLA();
        break;
        case 0x3b:
        StoreRWMAbsY();
        RLA();
        break;
        case 0x23:
        StoreIndX();
        RLA();
        break;
        case 0x33:
        StoreIndY();
        RLA();
        break;
        ///////RRA
        case 0x67:
        StoreRWMZpg();
        RRA();
        break;
        case 0x77:
        StoreRWMZpgX();
        RRA();
        break;
        case 0x6f:
        StoreRWMAbs();
        RRA();
        break;
        case 0x7f:
        StoreRWMAbsX();
        RRA();
        break;
        case 0x7b:
        StoreRWMAbsY();
        RRA();
        break;
        case 0x63:
        StoreIndX();
        RRA();
        break;
        case 0x73:
        StoreIndY();
        RRA();
        break;
        ///////ROL
        case 0x2a:
        ROLA();
        break;
        case 0x26:
        StoreRWMZpg();
        ROLM();
        break;
        case 0x36:
        StoreRWMZpgX();
        ROLM();
        break;
        case 0x2e:
        StoreRWMAbs();
        ROLM();
        break;
        case 0x3e:
        StoreRWMAbsX();
        ROLM();
        break;
        ///////ROR
        case 0x6a:
        RORA();
        break;
        case 0x66:
        StoreRWMZpg();
        RORM();
        break;
        case 0x76:
        StoreRWMZpgX();
        RORM();
        break;
        case 0x6e:
        StoreRWMAbs();
        RORM();
        break;
        case 0x7e:
        StoreRWMAbsX();
        RORM();
        break;
        ///////RTI AND RTS
        case 0x40:
        RTI();
        break;
        case 0x60:
        RTS();
        break;
        ///////SAX
        case 0x87:
        StoreZpg();
        SAX();
        break;
        case 0x97:
        StoreZpgX();
        SAX();
        break;
        case 0x8f:
        StoreAbs();
        SAX();
        break;
        case 0x83:
        StoreIndX();
        SAX();
        break;
        ///////SBC
        case 0xe9:
        ReadImm();
        SBC();
        break;
        case 0xe5:
        ReadZpg();
        SBC();
        break;
        case 0xf5:
        ReadZpgX();
        SBC();
        break;
        case 0xed:
        ReadAbs();
        SBC();
        break;
        case 0xfd:
        ReadAbsX();
        SBC();
        break;
        case 0xf9:
        ReadAbsY();
        SBC();
        break;
        case 0xe1:
        ReadIndX();
        SBC();
        break;
        case 0xf1:
        ReadIndY();
        SBC();
        break;
        ///////SBX
        case 0xcb:
        ReadImm();
        SBX();
        break;
        ///////SET FLAGS
        case 0x38:
        SEC();
        break;
        case 0xf8:
        SED();
        break;
        case 0x78:
        SEI();
        break;
        ///////SHA-SHY
        case 0x93:
        SHAa();
        break;
        case 0x9f:
        SHAb();
        break;
        case 0x9b:
        SHS();
        break;
        case 0x9e:
        SHX();
        break;
        case 0x9c:
        SHY();
        break;
        ///////SLO
        case 0x07:
        StoreRWMZpg();
        SLO();
        break;
        case 0x17:
        StoreRWMZpgX();
        SLO();
        break;
        case 0x0f:
        StoreRWMAbs();
        SLO();
        break;
        case 0x1f:
        StoreRWMAbsX();
        SLO();
        break;
        case 0x1b:
        StoreRWMAbsY();
        SLO();
        break;
        case 0x03:
        StoreIndX();
        SLO();
        break;
        case 0x13:
        StoreIndY();
        SLO();
        break;
        ///////SRE
        case 0x47:
        StoreRWMZpg();
        SRE();
        break;
        case 0x57:
        StoreRWMZpgX();
        SRE();
        break;
        case 0x4f:
        StoreRWMAbs();
        SRE();
        break;
        case 0x5f:
        StoreRWMAbsX();
        SRE();
        break;
        case 0x5b:
        StoreRWMAbsY();
        SRE();
        break;
        case 0x43:
        StoreIndX();
        SRE();
        break;
        case 0x53:
        StoreIndY();
        SRE();
        break;
        ///////STA
        case 0x85:
        StoreZpg();
        STA();
        break;
        case 0x95:
        StoreZpgX();
        STA();
        break;
        case 0x8d:
        StoreAbs();
        STA();
        break;
        case 0x9d:
        StoreAbsX();
        STA();
        break;
        case 0x99:
        StoreAbsY();
        STA();
        break;
        case 0x81:
        StoreIndX();
        STA();
        break;
        case 0x91:
        StoreIndY();
        STA();
        break;
        ///////STX
        case 0x86:
        StoreZpg();
        STX();
        break;
        case 0x96:
        StoreZpgY();
        STX();
        break;
        case 0x8e:
        StoreAbs();
        STX();
        break;
        ///////STY
        case 0x84:
        StoreZpg();
        STY();
        break;
        case 0x94:
        StoreZpgX();
        STY();
        break;
        case 0x8c:
        StoreAbs();
        STY();
        break;
        ///////TRANSFER
        case 0xaa:
        TAX();
        break;
        case 0xa8:
        TAY();
        break;
        case 0xba:
        TSX();
        break;
        case 0x8a:
        TXA();
        break;
        case 0x9a:
        TXS();
        break;
        case 0x98:
        TYA();
        break;
        default:
        break;
    }
}

void M6510Pause(void) {
    if(pc2.value!=0xfd53)
        pause=0xff;
}

void M6510Execute(void) {
    count=0;
    if(trace==0x0) {
        while(count<cycles) {
            pc1.value&=0xffff;
            if(pc1.value==0x0) {
                trace=0xff;
                break;
            }
            else {
                M6510Process();
            }
        }
    }
}

int M6510Tracer(void) {
    M6510Process();
    M6510Dump();
    Disa(5,pc2.value);
    RenderBuffer(400,50);
    if(count>cycles)
        count=0;
    return count;
}

void M6510SaveCpu(unsigned int *fileimage) {
    fileimage[0x00]=areg.value&0xff;		/* copy cpu status to temp buffer */
    fileimage[0x01]=xreg.value&0xff;
    fileimage[0x02]=yreg.value&0xff;
    fileimage[0x03]=psr.value&0xff;
    fileimage[0x04]=psp.value&0xff;
    fileimage[0x05]=pc1.value&0xffff;
    fileimage[0x06]=pc2.value&0xffff;
    fileimage[0x07]=BusIRQ();
    fileimage[0x08]=BusNMI();
}

void M6510LoadCpu(unsigned int *fileimage) {
    areg.value=fileimage[0x00]&0xff;				/* copy temp buffer to cpu structure */
    xreg.value=fileimage[0x01]&0xff;
    yreg.value=fileimage[0x02]&0xff;
    psr.value=fileimage[0x03]&0xff;
    psp.value=fileimage[0x04]&0xff;
    pc1.value=fileimage[0x05]&0xffff;
    pc2.value=fileimage[0x06]&0xffff;
}
