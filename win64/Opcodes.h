void ADC(void) {
    aux1.value=psr.value&Cflag;
    areg.value&=0xff;
    operand.value&=0xff;
    aux2.value=areg.value+operand.value+aux1.value;
    psr.value&=NNZCVF;
    if(!((areg.r.lo^operand.r.lo)&0x80)
            && ((areg.r.lo^aux2.r.lo)&0x80)) {
        psr.r.lo|=Vflag;
    }
    if((psr.r.lo&Dflag)==0) {
        if(aux2.value>0xff)
            psr.r.lo|=Cflag;
        areg.value=aux2.value&0xff;
        psr.r.lo|=flags[areg.r.lo];
    }
    else {
        aux2.value&=0xff;
        if(aux2.r.lo==0)
            psr.r.lo|=Zflag;
        aux2.r.lo=(areg.r.lo&0xf)+(operand.r.lo&0xf)+aux1.r.lo;
        aux1.value=0;
        if(aux2.r.lo>=0xa) {
            aux2.r.lo=(aux2.r.lo+6)&0xf;
            aux1.r.lo++;
        }
        aux1.r.lo+=(areg.r.lo>>4)+(operand.r.lo>>4);
        if((aux1.r.lo<<4)&0x80)
            psr.r.lo|=Nflag;
        if(aux1.r.lo>=0xa) {
            psr.r.lo|=Cflag;
            aux1.r.lo=(aux1.r.lo+6)&0xf;
        }
        areg.r.lo=(aux1.r.lo<<4)|aux2.r.lo;
    }
}

void AND(void) {
    psr.r.lo&=NNZF;
    areg.value&=operand.value;
    psr.r.lo|=flags[areg.r.lo];
}

void ASLA(void) {
    clock+=2;
    count+=2;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    psr.r.lo&=NNZCF;
    if(areg.value&0x80)
        psr.r.lo|=Cflag;
    areg.value<<=1;
    areg.value&=0xff;
    psr.r.lo|=flags[areg.value];
}

void ASLM(void) {
    psr.value&=NNZCF;
    if(operand.value&0x80)
        psr.r.lo|=Cflag;
    operand.value<<=1;
    operand.value&=0xff;
    psr.r.lo|=flags[operand.value];
    BusWriteByte(operand.r.lo,address.value);
}

void BIT(void) {
    psr.r.lo&=0xff-(Nflag|Zflag|Vflag);
    aux1.value=operand.value&areg.value&0xff;
    psr.r.lo|=(operand.r.lo&(Nflag|Vflag));
    if((aux1.r.lo)==0) {
        psr.r.lo|=Zflag;
    }
}

void Brancher(unsigned char test) {
    clock+=3;
    count+=3;
    operand.value=BusReadByte(pc1.value)&0xff;
    pc1.value=(pc1.value+1)&0xffff;
    aux1.r.lo=BusReadByte(pc1.value);
    if(test) {
        aux1.value=pc1.value;
        if(operand.r.lo&0x80)
            operand.r.hi=0xff;
        pc1.value=(pc1.value+operand.value)&0xffff;
        if(aux1.r.hi==pc1.r.hi) {
            count++;
            clock++;
        }
        else {
            clock+=2;
            count+=2;
        }
    }
}

void BEQ(void) {
    Brancher(psr.r.lo&Zflag);
}

void BNE(void) {
    Brancher(!(psr.r.lo&Zflag));
}

void BCS(void) {
    Brancher(psr.r.lo&Cflag);
}

void BCC(void) {
    Brancher(!(psr.r.lo&Cflag));
}

void BMI(void) {
    Brancher(psr.r.lo&Nflag);
}

void BPL(void) {
    Brancher(!(psr.r.lo&Nflag));
}

void BVS(void) {
    Brancher(psr.r.lo&Vflag);
}

void BVC(void) {
    Brancher(!(psr.r.lo&Vflag));
}

void BRK(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    pc1.value++;
    clock+=7;
    count+=7;
    BusPushWordStack(pc1.value,psp.r.lo);//w 1+2
    psp.r.lo-=2;
    psr.r.lo|=(Iflag|Uflag|Bflag);
    BusPushByteStack(psr.r.lo,psp.r.lo);//w 3
    psp.r.lo--;
    pc1.value=BusReadWord(0xfffe); // r 4
}

void CLC(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    psr.r.lo&=0xff-Cflag;
}

void CLD(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    psr.r.lo&=0xff-Dflag;
}

void CLI(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    psr.r.lo&=0xff-Iflag;
}

void CLV(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    psr.r.lo&=0xff-Vflag;
}

void CMP(void) {
    psr.r.lo&=NNZCF;
    areg.value&=0xff;
    operand.value&=0xff;
    operand.value=areg.value-operand.value;
    if(operand.value<0x100) {
        psr.r.lo|=Cflag;
    }
    operand.value&=0xff;
    psr.r.lo|=flags[operand.value];
}

void CPX(void) {
    psr.r.lo&=NNZCF;
    xreg.value&=0xff;
    operand.value&=0xff;
    operand.value=xreg.value-operand.value;
    if(operand.value<0x100) {
        psr.r.lo|=Cflag;
    }
    operand.value&=0xff;
    psr.r.lo|=flags[operand.value];
}

void CPY(void) {
    psr.r.lo&=NNZCF;
    yreg.value&=0xff;
    operand.value&=0xff;
    operand.value=yreg.value-operand.value;
    if(operand.value<0x100) {
        psr.r.lo|=Cflag;
    }
    operand.value&=0xff;
    psr.r.lo|=flags[operand.value];
}

void DEC(void) {
    psr.r.lo&=NNZF;
    operand.r.lo--;
    psr.r.lo|=flags[operand.r.lo];
    BusWriteByte(operand.r.lo,address.value);
}

void DEX(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    psr.r.lo&=NNZF;
    xreg.r.lo--;
    psr.r.lo|=flags[xreg.r.lo];
}

void DEY(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    psr.r.lo&=NNZF;
    yreg.r.lo--;
    psr.r.lo|=flags[yreg.r.lo];
}

void EORA(void) {
    psr.r.lo&=NNZF;
    areg.r.lo=areg.r.lo^operand.r.lo;
    psr.r.lo|=flags[areg.r.lo];
}

void INC(void) {
    psr.r.lo&=NNZF;
    operand.r.lo++;
    psr.r.lo|=flags[operand.r.lo];
    BusWriteByte(operand.r.lo,address.value);
}

void INX(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    psr.r.lo&=NNZF;
    xreg.r.lo++;
    psr.r.lo|=flags[xreg.r.lo];
}

void INY(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    psr.r.lo&=NNZF;
    yreg.r.lo++;
    psr.r.lo|=flags[yreg.r.lo];
}

void JMPAbs(void) {
    clock+=3;
    count+=3;
    pc1.value=BusReadWord(pc1.value);
}

void JMPInd(void) {
    clock+=5;
    count+=5;
    pc1.value=BusReadWord(pc1.value);
    pc1.value=BusReadBounds(pc1.value);
}

void JSR(void) {
    clock+=6;
    count+=6;
    aux1.value=BusReadWord(pc1.value);
    pc1.value++;
    BusPushWordStack(pc1.value,psp.r.lo);
    psp.r.lo-=2;
    pc1.value=aux1.value;
}

void LDA(void) {
    psr.r.lo&=NNZF;
    areg.r.lo=operand.r.lo;
    psr.r.lo|=flags[areg.r.lo];
}

void LDX(void) {
    psr.r.lo&=NNZF;
    xreg.r.lo=operand.r.lo;
    psr.r.lo|=flags[xreg.r.lo];
}

void LDY(void) {
    psr.r.lo&=NNZF;
    yreg.r.lo=operand.r.lo;
    psr.r.lo|=flags[yreg.r.lo];
}

void LSRA(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    psr.r.lo&=NNZCF;
    psr.r.lo|=(areg.r.lo&Cflag);
    areg.r.lo>>=1;
    psr.r.lo|=flags[areg.r.lo];
}

void LSRM(void) {
    psr.r.lo&=NNZCF;
    psr.r.lo|=(operand.r.lo&Cflag);
    operand.r.lo>>=1;
    psr.r.lo|=flags[operand.r.lo];
    BusWriteByte(operand.r.lo,address.value);
}

void NOP(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=2;
    count+=2;
    areg.value&=0xff;
    xreg.value&=0xff;
    yreg.value&=0xff;
    psp.value&=0x1ff;
    pc1.value&=0xffff;
    operand.value=0;
    aux1.value=0;
    aux2.value=0;
}

void NOPund(void) {
    areg.value&=0xff;
    xreg.value&=0xff;
    yreg.value&=0xff;
    psp.value&=0x1ff;
    pc1.value&=0xffff;
    operand.value=0;
    aux1.value=0;
    aux2.value=0;
}

void ORA(void) {
    psr.r.lo&=NNZF;
    areg.r.lo|=operand.r.lo;
    psr.r.lo|=flags[areg.r.lo];
}

void PHA(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=3;
    count+=3;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    BusPushByteStack(areg.r.lo,psp.r.lo);
    psp.r.lo--;
}

void PHP(void) {
    aux1.r.lo=BusReadByte(pc1.value); // r2
    clock+=3;
    count+=3;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    BusPushByteStack(psr.r.lo|Uflag|Bflag,psp.r.lo);
    psp.r.lo--;
}

void PLA(void) {
    clock+=4;
    count+=4;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    psp.r.lo++;
    areg.r.lo=BusPopByteStack(psp.r.lo);
    areg.value&=0xff;
    psr.r.lo|=flags[areg.r.lo];
}

void PLP(void) {
    clock+=4;
    count+=4;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    psp.r.lo++;
    psr.r.lo=BusPopByteStack(psp.r.lo);
    psr.r.lo|=Uflag;
    psr.r.lo&=0xff-Bflag;
}

void ROLA(void) {
    clock+=2;
    count+=2;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    aux1.value=psr.value&Cflag;
    areg.value&=0xff;
    psr.r.lo&=NNZCF;
    if(areg.value&0x80)
        psr.r.lo|=Cflag;
    areg.value=(areg.value+areg.value)|aux1.value;
    areg.value&=0xff;
    psr.r.lo|=flags[areg.r.lo];
}

void ROLM(void) {
    aux1.value=psr.value&Cflag;
    operand.value&=0xff;
    psr.value&=NNZCF;
    if(operand.value&0x80)
        psr.r.lo|=Cflag;
    operand.value=(operand.value+operand.value)|aux1.value;
    operand.value&=0xff;
    psr.r.lo|=flags[operand.r.lo];
    BusWriteByte(operand.r.lo,address.value);
}

void RORA(void) {
    clock+=2;
    count+=2;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    areg.r.hi=psr.r.lo&Cflag;
    aux2.r.lo=areg.r.lo&Cflag;
    psr.r.lo&=NNZCF;
    areg.value=(areg.value>>1)&0xff;
    psr.r.lo|=flags[areg.r.lo]|aux2.r.lo;
}

void RORM(void) {
    operand.r.hi=psr.r.lo&Cflag;
    aux2.r.lo=operand.r.lo&Cflag;
    psr.value&=NNZCF;
    operand.value=(operand.value>>1)&0xff;
    psr.r.lo|=flags[operand.r.lo]|aux2.r.lo;
    BusWriteByte(operand.r.lo,address.value);
}

void RTI(void) {
    clock+=6;
    count+=6;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    psp.r.lo++;
    psr.r.lo=BusPopByteStack(psp.r.lo);
    psr.r.lo|=Uflag;
    psp.r.lo++;
    pc1.value=BusPopWordStack(psp.r.lo);
    psr.r.lo&=0xff-Bflag;
    psp.r.lo++;
}

void RTS(void) {
    clock+=6;
    count+=6;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    psp.r.lo++;
    pc1.value=BusPopWordStack(psp.r.lo)+1;
    psp.r.lo++;
    pc1.value&=0xffff;
}

void STA(void) {
    BusWriteByte(areg.r.lo,address.value);
}

void STX(void) {
    BusWriteByte(xreg.r.lo,address.value);
}

void STY(void) {
    BusWriteByte(yreg.r.lo,address.value);
}

void SBC(void) {
    areg.value&=0xff;
    operand.value&=0xff;
    aux1.value=(~psr.r.lo)&0x01;
    aux2.value=areg.value-(operand.value+aux1.value);
    psr.value&=NNZCVF;
    if(((areg.r.lo^operand.r.lo)&0x80)
             &&((areg.r.lo^aux2.r.lo)&0x80)) {
        psr.r.lo|=Vflag;
    }

    if((psr.r.lo&Dflag)==0) {
        if(aux2.value<0x100)
            psr.r.lo|=Cflag;
        areg.value=aux2.value&0xff;
        psr.r.lo|=flags[areg.r.lo];
    }
    else {
        aux2.value&=0xff;
        if(aux2.value==0)
            psr.r.lo|=Zflag;
        aux2.r.lo=(areg.r.lo&0xf)-(operand.r.lo&0xf)-aux1.r.lo;
        aux1.value=0;
        if(aux2.r.lo>=0xa) {
            aux2.value=(aux2.r.lo-6)&0xf;
            aux1.value=0x01;
        }
        aux1.value=((areg.r.lo>>4)-(operand.r.lo>>4)-aux1.r.lo)&0xff;
        operand.value=aux1.value<<4;
        if(operand.r.lo&0x80)
            psr.r.lo|=Nflag;
        if(aux1.value>=0xa) {
            aux1.value=(aux1.value-0x6)&0xf;
        }
        areg.value=((aux1.value<<4)|aux2.value)&0xff;
    }
}

void SEC(void) {
    clock+=2;
    count+=2;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    psr.r.lo|=Cflag;
}

void SED(void) {
    clock+=2;
    count+=2;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    psr.r.lo|=Dflag;
}

void SEI(void) {
    clock+=2;
    count+=2;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    psr.r.lo|=Iflag;
}

void TAX(void) {
    clock+=2;
    count+=2;
    psr.r.lo&=NNZF;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    xreg.value=areg.value&0xff;
    psr.r.lo|=flags[xreg.r.lo];
}

void TXA(void) {
    clock+=2;
    count+=2;
    psr.r.lo&=NNZF;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    areg.value=xreg.value&0xff;
    psr.r.lo|=flags[areg.value];
}

void TAY(void) {
    clock+=2;
    count+=2;
    psr.r.lo&=NNZF;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    yreg.value=areg.value&0xff;
    psr.r.lo|=flags[yreg.value];
}

void TYA(void) {
    clock+=2;
    count+=2;
    psr.r.lo&=NNZF;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    areg.value=yreg.value&0xff;
    psr.r.lo|=flags[areg.value];
}

void TSX(void) {
    clock+=2;
    count+=2;
    psr.r.lo&=NNZF;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    xreg.value=psp.value&0xff;
    psr.r.lo|=flags[xreg.value];
}

void TXS(void) {
    clock+=2;
    count+=2;
    aux1.r.lo=BusReadByte(pc1.value); // r2
    psp.value=xreg.value&0xff;
}

//////Undocumented instructions..
void ANC(void) {
    psr.r.lo&=NNZCF;
    areg.r.lo&=operand.r.lo;
    psr.r.lo|=flags[areg.r.lo];
    operand.r.lo=areg.r.lo;
    operand.r.lo=((operand.r.lo&0x80)>>7);
    psr.r.lo|=operand.r.lo;
}

void ANE(void) {
    psr.value&=NNZF;
    areg.r.lo|=0xee;
    areg.value=areg.value&xreg.value&operand.value&0xff;
    psr.r.lo|=flags[areg.value];
}

void ARR(void) {
    if(psr.r.lo&Dflag) {
        aux2.r.lo=operand.r.lo;
        operand.r.lo&=areg.r.lo;
        operand.r.lo>>=1;
        if(psr.r.lo&Cflag)
            operand.r.lo|=0x80;
        psr.r.lo&=NNZCVF;
        psr.r.lo|=flags[operand.r.lo];
        aux1.r.lo=(operand.r.lo^areg.r.lo);
    }
    else {
        psr.r.lo&=NNZCVF;
        operand.value&=0xff;
        aux2.value=operand.value;
        aux1.value=0x00;
        if(operand.value&0x1)
            aux1.value=0x80;
        operand.value&=areg.value;
        operand.r.lo>>=1;
        operand.r.lo|=aux1.r.lo;
        aux1.r.lo=(operand.r.lo&Uflag)<<1;
        aux2.r.lo=operand.r.lo&Vflag;
        areg.r.lo=operand.r.lo;
        psr.r.lo|=flags[areg.r.lo]|((aux1.r.lo^aux2.r.lo)&Vflag);
        psr.r.lo|=(aux2.value>>6)&0xff;
    }
}

void ASR(void) {
    AND();
    LSRA();
}

void DCP(void) {
    DEC();
    CMP();
}

void ISB(void) {
    INC();
    SBC();
}

void LAX(void) {
    psr.value&=NNZF;
    xreg.value=areg.value=operand.value&0xff;
    psr.r.lo|=flags[areg.value];
}

void LAS(void) {
    psr.value&=NNZF;
    psp.r.lo&=operand.r.lo;
    xreg.r.lo=areg.r.lo=psp.r.lo;
    psr.r.lo|=flags[areg.r.lo];
    psp.value|=0x100;
}

void LXA(void) {
    psr.value&=NNZF;
    areg.r.lo|=0xee;
    areg.value&=operand.value&0xff;
    psr.r.lo|=flags[areg.value];
    xreg.value=areg.value;
}

void SBX(void) {
    psr.value&=NNZCF;
    operand.value&=0xff;
    xreg.value&=areg.value&0xff;
    if(xreg.value>operand.value) {
        psr.r.lo|=Cflag;
    }
    xreg.r.lo=xreg.r.lo-operand.r.lo;
    psr.r.lo|=flags[xreg.r.lo];
}

void RLA(void) {
    ROLM();
    AND();
}

void RRA(void) {
    RORM();
    ADC();
}

void SAX(void) {
    operand.value=areg.value&xreg.value;
    BusWriteByte(operand.r.lo,address.value);
}

void SHAa(void) {
    address.value=BusReadByte(pc1.value)&0xff;
    aux2.r.lo=address.r.lo;
    pc1.value++;
    operand.r.lo=BusReadByte(address.value);
    address.r.lo++;
    operand.r.hi=BusReadByte(address.value);
    address.value=operand.value&0xffff;
    aux1.r.hi=address.r.hi;
    address.value+=(yreg.value&0xff);
    aux1.r.lo=address.r.lo;
    operand.r.lo=BusReadByte(aux1.value);
    operand.r.lo&=(aux1.r.hi+1)&xreg.r.lo&areg.r.lo;
    BusWriteByte(operand.r.lo,address.value);
}

void SHAb(void) {
    address.value=BusReadWord(pc1.value);
    aux1.r.hi=address.r.hi;
    pc1.value+=2;
    address.value+=yreg.value;
    aux1.r.lo=address.r.lo;
    operand.r.lo=BusReadByte(aux1.r.lo);
    operand.r.lo=areg.r.lo&xreg.r.lo&aux1.r.hi;
    BusWriteByte(operand.r.lo,address.value);
}
void SHS(void) {
    address.value=BusReadWord(pc1.value);
    aux1.r.lo=address.r.hi;
    pc1.value+=2;
    address.value+=yreg.value;
    operand.r.lo=areg.r.lo&xreg.r.lo;
    psp.r.lo=operand.r.lo;
    psp.value|=0x100;
    aux1.r.lo++;
    operand.r.lo&=aux1.r.lo;
    BusWriteByte(operand.r.lo,address.value);
}

void SHX(void) {
    address.value=BusReadWord(pc1.value);
    operand.r.lo=address.r.hi+1;
    pc1.value+=2;
    address.value+=yreg.value;
    operand.r.lo&=xreg.r.lo;
    BusWriteByte(operand.r.lo,address.value);
}

void SHY(void) {
    address.value=BusReadWord(pc1.value);
    operand.r.lo=address.r.hi+1;
    pc1.value+=2;
    address.value+=xreg.value;
    operand.r.lo&=yreg.r.lo;
    BusWriteByte(operand.r.lo,address.value);
}

void SLO(void) {
    ASLM();
    ORA();
}

void SRE(void) {
    LSRM();
    EORA();
}

void JAM(void) {
    pc1.value=pc2.value;
    inst--;
}
