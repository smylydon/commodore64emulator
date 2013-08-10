/*
 * CPU.java
 *
 * Created on February 2, 2006, 10:14 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

/**
 *
 * @author Owner
 */
public class CPU {
    private static final int Nflag=0x80;
    private static final int Vflag=0x40;
    private static final int Uflag=0x20;
    private static final int Bflag=0x10;
    private static final int Dflag=0x08;
    private static final int Iflag=0x04;
    private static final int Zflag=0x02;
    private static final int Cflag=0x01;
    private static final int NNZCVF=0xff-(Nflag|Zflag|Cflag|Vflag);
    private static final int NNZCF=0xff-(Nflag|Zflag|Cflag);
    private static final int NNZF=0xff-(Nflag|Zflag);

    private AddressBus Bus;
    private Timing t;
    private Flags f;
    private int areg,xreg,yreg,psr,psp,pc1,pc2;
    private int cycles,count,clock,inst,nmemonic,trace;
    private int operand,address,aux1,aux2,aux3,aux4;
    
    private void ReadImm() {
    clock+=2;
    count+=2;
    operand=Bus.readByte(pc1);
    pc1++;
    }

    private void ReadZpg() {
    clock+=3;
    count+=3;
    address=Bus.readByte(pc1);
    operand=Bus.readByte(address);
    pc1++;
    }

 private void ReadZpgX() {
    clock+=4;
    count+=4;
    address=(Bus.readByte(pc1)+xreg)&0xff;
    operand=Bus.readByte(address);
    pc1++;
}

private void ReadZpgY() {
    clock+=4;
    count+=4;
    address=(Bus.readByte(pc1)+yreg)&0xff;
    operand=Bus.readByte(address);
    pc1++;
}

private void ReadAbs() {
    clock+=4;
    count+=4;
    address=Bus.readWord(pc1);
    operand=Bus.readByte(address);
    pc1+=2;
}

private void ReadAbsX() {
    clock+=4;
    count+=4;
    xreg&=0xff;
    address=Bus.readWord(pc1);
    aux1=address&0xff00;
    address+=xreg;
    aux2=address&0xff00;
    if(aux2!=aux1) {
        count++;
        clock++;
        aux1|=(address&0xff);
        operand=Bus.readByte(aux1);
    }
    operand=Bus.readByte(address);
    pc1+=2;
}

private void ReadAbsY() {
    clock+=4;
    count+=4;
    yreg&=0xff;
    address=Bus.readWord(pc1);
    aux1=address&0xff00;
    address+=yreg;
    aux2=address&0xff00;
    if(aux2!=aux1) {
        count++;
        clock++;
        aux1|=(address&0xff);
        operand=Bus.readByte(aux1);
    }
    operand=Bus.readByte(address);
    pc1+=2;
}

private void ReadIndX() {
    clock+=6;
    count+=6;
    xreg&=0xff;
    address=(Bus.readByte(pc1)+xreg)&0xff;
    address=Bus.readBounds(address);
    operand=Bus.readByte(address);
    pc1++;
}

private void ReadIndY() {
    clock+=5;
    count+=5;
    yreg&=0xff;
    address=Bus.readByte(pc1);
    address=Bus.readBounds(address);
    aux1=address&0xff00;
    address=(address+yreg)&0xffff;
    aux2=address&0xff00;
    if(aux2!=aux1) {
        count++;
        clock++;
        aux1|=(address&0xff);
        operand=Bus.readByte(aux1);
    }
    operand=Bus.readByte(address);
    pc1++;
}

//////////

private void StoreZpg() {
    clock+=3;
    count+=3;
    address=Bus.readByte(pc1);
    pc1++;
}

private void StoreZpgX() {
    clock+=4;
    count+=4;
    address=(Bus.readByte(pc1)+xreg)&0xff;
    pc1++;
}

private void StoreZpgY() {
    clock+=4;
    count+=4;
    address=(Bus.readByte(pc1)+yreg)&0xff;
    pc1++;
}

private void StoreAbs() {
    clock+=4;
    count+=4;
    address=Bus.readWord(pc1);           //r 2+3
    pc1+=2;
}

private void StoreAbsX() {
    clock+=5;
    count+=5;
    xreg&=0xff;
    address=Bus.readWord(pc1);
    aux1=address&0xff00;
    address=(address+xreg)&0xffff;
    aux1|=(address&0xff);
    aux2=Bus.readByte(aux1);
    pc1+=2;
}

private void StoreAbsY() {
    clock+=5;
    count+=5;
    yreg&=0xff;
    address=Bus.readWord(pc1);
    aux1=address&0xff00;
    address=(address+yreg)&0xffff;
    aux1|=(address&0xff);
    aux2=Bus.readByte(aux1);
    pc1+=2;
}

private void StoreIndX() {
    clock+=6;
    count+=6;
    aux1=(Bus.readByte(pc1)+xreg)&0xff;
    address=Bus.readBounds(aux1);
    pc1++;
}

private void StoreIndY() {
    clock+=6;
    count+=6;
    yreg&=0xff;
    aux1=Bus.readByte(pc1);
    address=Bus.readBounds(aux1);
    aux1=address&0xff00;
    address=(address+yreg)&0xffff;
    aux1|=(address&0xff);
    aux2=Bus.readByte(aux1);
    pc1++;
};
    
  private void StoreRWMZpg() {
    clock+=5;
    count+=5;
    address=Bus.readByte(pc1);
    operand=Bus.readByte(address);
    pc1++;
};

    private void StoreRWMZpgX(){
    clock+=6;
    count+=6;
    address=Bus.readByte(pc1);
    address=(address+xreg)&0xff;
    operand=Bus.readByte(address);
    pc1++;
}

    private void StoreRWMZpgY() {
    clock+=6;
    count+=6;
    address=Bus.readByte(pc1);
    address=(address+yreg)&0xff;
    operand=Bus.readByte(address);
    pc1++;
}

private void StoreRWMAbs() {
    clock+=6;
    count+=6;
    address=Bus.readWord(pc1);           //r 2+3
    operand=Bus.readByte(address);  //r4
    Bus.writeByte(operand,address);       //w1
    pc1+=2;
}

private void StoreRWMAbsX() {
    clock+=7;
    count+=7;
    xreg&=0xff;
    address=Bus.readWord(pc1);           //r 2+3
    aux1=address&0xff00;
    address=(address+xreg)&0xffff;
    aux1|=(address&0xff);
    aux2=Bus.readByte(aux1);              //r4
    operand=Bus.readByte(address);  //r5
    Bus.writeByte(operand,address);       //w1
    pc1+=2;
}

private void StoreRWMAbsY() {
    clock+=7;
    count+=7;
    yreg&=0xff;
    address=Bus.readWord(pc1);           //r 2+3
    aux1=address&0xff00;
    address=(address+yreg)&0xffff;
    aux1|=(address&0xff);
    aux2=Bus.readByte(aux1);              //r4
    operand=Bus.readByte(address);  //r5
    Bus.writeByte(operand,address);       //w1
    pc1+=2;
}

private void StoreRWMIndX() {
    clock+=8;
    count+=8;
    xreg&=0xff;
    aux1=(Bus.readByte(pc1)+xreg)&0xff;
    address=Bus.readBounds(aux1);
    operand=Bus.readByte(address);
    Bus.writeByte(operand,address);
    pc1++;
}

private void StoreRWMIndY() {
    clock+=8;
    count+=8;
    yreg&=0xff;
    aux1=Bus.readByte(pc1);
    address=Bus.readBounds(aux1);
    aux1=address&0xff00;
    aux1|=((address+yreg)&0xff);
    address=(address+yreg)&0xffff;
    aux2=Bus.readByte(aux1);
    operand=Bus.readByte(address);
    pc1++;
}

void ADC() {
    aux1=psr&Cflag;
    areg&=0xff;
    operand&=0xff;
    aux2=areg+operand+aux1;
    psr&=NNZCVF;
    if((((~(areg^operand)&0x80))!=0)&& (((areg^aux2)&0x80)!=0)) {
        psr|=Vflag;
    }
    if((~(psr&Dflag))!=0) {
        if((aux2&0xff00)!=0)
            psr|=Cflag;
        areg=aux2&0xff;
        psr|=f.readFlags(areg);
    }
    else {
        aux2&=0xff;
        if(aux2==0)
            psr|=Zflag;
        aux2=((areg&0xf)+(operand&0xf)+aux1)&0xff;
        aux1=0;
        if(aux2>=0xa) {
            aux2=(aux2+6)&0xf;
            aux1++;
        }
        aux1=(aux1+(areg>>4)+(operand>>4))&0xff;
        if(((aux1<<4)&0x80)!=0)
            psr|=Nflag;
        if(aux1>=0xa) {
            psr|=Cflag;
            aux1=(aux1+6)&0xf;
        }
        areg=((aux1<<4)|aux2)&0xff;
    };
}

private void AND() {
    psr&=NNZF;
    areg&=0xff;
    areg&=operand;
    psr|=f.readFlags(areg);
}

void ASLA() {
    clock+=2;
    count+=2;
    aux1=Bus.readByte(pc1); // r2        
    psr&=NNZCF;
    if((areg&0x80)!=0)
        psr|=Cflag;
    areg<<=1;        
    areg&=0xff;
    psr|=f.readFlags(areg);
}

void ASLM() {
    psr&=NNZCF;
    if((operand&0x80)!=0)
        psr|=Cflag;
    operand<<=1;        
    operand&=0xff;
    psr|=f.readFlags(operand);
    Bus.writeByte(operand,address);
}

void BIT() {
    psr&=0xff-(Nflag|Zflag|Vflag);
    aux1=operand&areg&0xff;
    psr|=(operand&(Nflag|Vflag));
    if((aux1)==0) {
        psr|=Zflag;
    }
}

void Brancher(int test,int cond) {
    clock+=3;
    count+=3;
    operand=Bus.readByte(pc1);
    pc1=(pc1+1)&0xffff;
    aux1=Bus.readByte(pc1);        
    if(test==cond) {
        aux1=pc1&0xff00;
        if((operand&0x80)!=0)
            operand|=0xff00;
        pc1=(pc1+operand)&0xffff;
        aux2=pc1&0xff00;
        if(aux1==aux2) {
            count++;
            clock++;
        }
        else {
            clock+=2;
            count+=2;
        }
    }
}

void BEQ() {
    Brancher(psr&Zflag,1);
}

void BNE() {
    Brancher(psr&Zflag,0);
}

void BCS() {
    Brancher(psr&Cflag,1);
}

void BCC() {
    Brancher(psr&Cflag,0);
}

void BMI() {
    Brancher(psr&Nflag,1);
}

void BPL() {
    Brancher(psr&Nflag,0);
}

void BVS() {
    Brancher(psr&Vflag,1);
}

void BVC() {
    Brancher(psr&Vflag,0);
}

void BRK() {
    aux1=Bus.readByte(pc1); // r2   
    pc1++;
    clock+=7;
    count+=7;
    Bus.pushWord(pc1,psp);//w 1+2
    psp-=2;
    psr|=(Iflag|Uflag|Bflag);    
    Bus.pushByte(psr,psp);//w 3
    psp--;
    pc1=Bus.readWord(0xfffe); // r 4
    psp&=0xff;
}

void CLC() {
    aux1=Bus.readByte(pc1); // r2   
    clock+=2;
    count+=2;
    psr&=0xff-Cflag;
}

void CLD() {
    aux1=Bus.readByte(pc1); // r2    
    clock+=2;
    count+=2;
    psr&=0xff-Dflag;
}

void CLI() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=2;
    count+=2;
    psr&=0xff-Iflag;
}

void CLV() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=2;
    count+=2;
    psr&=0xff-Vflag;
}

void CMP() {
    psr&=NNZCF;
    areg&=0xff;
    operand&=0xff;
    if(areg>=operand) {
        psr|=Cflag;
    }
    operand=areg-operand;
    psr|=f.readFlags(operand);
}

void CPX() {
    psr&=NNZCF;
    xreg&=0xff;
    operand&=0xff;
    if(xreg>=operand) {
        psr|=Cflag;
    }
    operand=xreg-operand;
    psr|=f.readFlags(operand);
}

void CPY() {
    psr&=NNZCF;
    yreg&=0xff;
    operand&=0xff;
    if(yreg>=operand) {
        psr|=Cflag;
    }
    operand=yreg-operand;
    psr|=f.readFlags(operand);
}

void DEC() {
    psr&=NNZF;
    operand=(operand-1)&0xff;
    psr|=f.readFlags(operand);
    Bus.writeByte(operand,address);
}

void DEX() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=2;
    count+=2;
    psr&=NNZF;
    xreg=(xreg-1)&0xff;
    psr|=f.readFlags(xreg);
}

void DEY() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=2;
    count+=2;
    psr&=NNZF;
    yreg=(yreg-1)&0xff;
    psr|=f.readFlags(yreg);
}

void EORA() {

    psr&=NNZF;
    areg=(areg^operand)&0xff;
    psr|=f.readFlags(areg);
}

void INC() {
    psr&=NNZF;
    operand=(operand+1)&0xff;
    psr|=f.readFlags(operand);
    Bus.writeByte(operand,address);
}

void INX() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=2;
    count+=2;
    psr&=NNZF;
    xreg=(xreg+1)&0xff;
    psr|=f.readFlags(xreg);
}

void INY() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=2;
    count+=2;
    psr&=NNZF;
    yreg=(yreg+1)&0xff;
    psr|=f.readFlags(yreg);
}

void JMPAbs() {
    clock+=3;
    count+=3;
    pc1=Bus.readWord(pc1);
}

void JMPInd() {
    clock+=5;
    count+=5;
    pc1=Bus.readWord(pc1);
    pc1=Bus.readBounds(pc1);
}

void JSR() {
    clock+=6;
    count+=6;
    aux1=Bus.readWord(pc1);
    pc1++;
    Bus.pushWord(pc1,psp);
    psp=(psp-2)&0xff;
    pc1=aux1;
}

void LDA() {
    psr&=NNZF;
    areg=operand&0xff;
    psr|=f.readFlags(areg);
}

void LDX() {
    psr&=NNZF;
    xreg=operand&0xff;
    psr|=f.readFlags(xreg);
}

void LDY() {
    psr&=NNZF;
    yreg=operand&0xff;
    psr|=f.readFlags(yreg);
}

void LSRA() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=2;
    count+=2;
    psr&=NNZCF;
    psr|=(areg&Cflag);
    areg=(areg>>1)&0xff;
    psr|=f.readFlags(areg);
}

void LSRM() {
    psr&=NNZCF;
    psr|=(operand&Cflag);
    operand=(operand>>1)&0xff;
    psr|=f.readFlags(operand);
    Bus.writeByte(operand,address);
}

void NOP() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=2;
    count+=2;
    areg&=0xff;
    xreg&=0xff;
    yreg&=0xff;
    psp&=0x1ff;
    pc1&=0xffff;
    operand=0;
    address=0;
    aux1=0;
    aux2=0;
}

void NOPund() {
    areg&=0xff;
    xreg&=0xff;
    yreg&=0xff;
    psp&=0x1ff;
    pc1&=0xffff;
    operand=0;
    address=0;
    aux1=0;
    aux2=0;
}

void ORA() {
    psr&=NNZF;
    areg=(areg|operand)&0xff;
    psr|=f.readFlags(areg);
}

void PHA() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=3;
    count+=3;
    aux1=Bus.readByte(pc1); // r2    
    Bus.pushByte(areg,psp);
    psp--;
}

void PHP() {
    aux1=Bus.readByte(pc1); // r2     
    clock+=3;
    count+=3;
    aux1=Bus.readByte(pc1); // r2    
    Bus.pushByte(psr|Uflag|Bflag,psp);
    psp--;
}

void PLA() {
    clock+=4;
    count+=4;
    aux1=Bus.readByte(pc1); // r2    
    psp++;
    areg=Bus.popByte(psp);
    areg&=0xff;
    psr|=f.readFlags(areg);    
}

void PLP() {
    clock+=4;
    count+=4;
    aux1=Bus.readByte(pc1); // r2    
    psp++;
    psr=Bus.popByte(psp);
    psr|=Uflag;
    psr&=0xff-Bflag;
}

void ROLA() {
    clock+=2;
    count+=2;
    aux1=Bus.readByte(pc1); // r2    
    aux1=psr&Cflag;
    areg&=0xff;
    psr&=NNZCF;
    if((areg&0x80)!=0)
        psr|=Cflag;    
    areg=(areg+areg)|aux1;
    areg&=0xff;
    psr|=f.readFlags(areg);
}

void ROLM() {
    aux1=psr&Cflag;
    operand&=0xff;
    psr&=NNZCF;
    if((operand&0x80)!=0)
        psr|=Cflag;    
    operand=(operand+operand)|aux1;
    operand&=0xff;
    psr|=f.readFlags(operand);
    Bus.writeByte(operand,address);
}

void RORA() {
    clock+=2;
    count+=2;
    areg&=0xff;
    aux1=Bus.readByte(pc1); // r2    
    areg|=((psr&Cflag)<<8);
    aux2=areg&Cflag;
    psr&=NNZCF;
    areg=(areg>>1)&0xff;
    psr|=f.readFlags(areg)|aux2;
}

void RORM() {
    operand&=0xff;
    operand|=((psr&Cflag)<<8);
    aux2=operand&Cflag;
    psr&=NNZCF;
    operand=(operand>>1)&0xff;
    psr|=f.readFlags(operand)|aux2;
    Bus.writeByte(operand,address);
}

void RTI() {
    clock+=6;
    count+=6;
    aux1=Bus.readByte(pc1); // r2    
    psp++;
    psr=Bus.popByte(psp);
    psr|=Uflag;
    psp++;
    pc1=Bus.popWord(psp);
    psr&=(0xff-Bflag);
    psp=(psp+1)&0xff;
}

void RTS() {
    clock+=6;
    count+=6;
    aux1=Bus.readByte(pc1); // r2    
    psp++;
    pc1=Bus.popWord(psp)+1;
    psp=(psp+1)&0xff;
    pc1&=0xffff;
}

void STA() {
    Bus.writeByte(areg,address);
}

void STX() {
    Bus.writeByte(xreg,address);
}

void STY() {
    Bus.writeByte(yreg,address);
}

void SBC() {
    areg&=0xff;
    operand&=0xff;
    aux1=(~psr)&0x01;
    aux2=areg-(operand+aux1);
    psr&=NNZCVF;
    if((((areg^operand)&0x80)!=0)
             &&(((areg^aux2)&0x80))!=0) {
        psr|=Vflag;
    }
    
    if((psr&Dflag)==0) {
        if(areg>=operand)
            psr|=Cflag;
        areg=aux2&0xff;
        psr|=f.readFlags(areg);
    }
    else {
        aux2&=0xff;
        if(aux2==0)
            psr|=Zflag;
        aux2=(areg&0xf)-(operand&0xf)-aux1;
        aux1=0;
        if(aux2>=0xa) {
            aux2=(aux2-6)&0xf;
            aux1=0x01;
        }
        aux1=((areg>>4)-(operand>>4)-aux1)&0xff;
        operand=aux1<<4;
        if((operand&0x80)!=0)
            psr|=Nflag;
        if(aux1>=0xa) {
            aux1=(aux1-0x6)&0xf;
        }
        areg=((aux1<<4)|aux2)&0xff;
    }
}

void SEC() {
    clock+=2;
    count+=2;
    aux1=Bus.readByte(pc1); // r2    
    psr|=Cflag;
}

void SED() {
    clock+=2;
    count+=2;
    aux1=Bus.readByte(pc1); // r2    
    psr|=Dflag;
}

void SEI() {
    clock+=2;
    count+=2;
    aux1=Bus.readByte(pc1); // r2    
    psr|=Iflag;
}

void TAX() {
    clock+=2;
    count+=2;
    psr&=NNZF;
    aux1=Bus.readByte(pc1); // r2    
    xreg=areg&0xff;
    psr|=f.readFlags(xreg);
}

void TXA() {
    clock+=2;
    count+=2;
    psr&=NNZF;    
    aux1=Bus.readByte(pc1); // r2    
    areg=xreg&0xff;
    psr|=f.readFlags(areg);
}

void TAY() {
    clock+=2;
    count+=2;
    psr&=NNZF;    
    aux1=Bus.readByte(pc1); // r2    
    yreg=areg&0xff;
    psr|=f.readFlags(yreg);
}

void TYA() {
    clock+=2;
    count+=2;
    psr&=NNZF;    
    aux1=Bus.readByte(pc1); // r2    
    areg=yreg&0xff;
    psr|=f.readFlags(areg);
}

void TSX() {
    clock+=2;
    count+=2;
    psr&=NNZF;    
    aux1=Bus.readByte(pc1); // r2    
    xreg=psp&0xff;
    psr|=f.readFlags(xreg);
}

void TXS() {
    clock+=2;
    count+=2;
    aux1=Bus.readByte(pc1); // r2    
    psp=xreg&0xff;
}

//////Undocumented instructions..
void ANC() {
    psr&=NNZCF;
    operand&=0xff;
    areg&=operand;
    psr|=f.readFlags(areg);
    operand=areg;
    operand=((operand&0x80)>>7);
    psr|=operand;
}

void ANE() {
    psr&=NNZF;
    areg|=0xee;
    areg=areg&xreg&operand&0xff;
    psr|=f.readFlags(areg);
}

void ARR() {
    if((psr&Dflag)!=0) {
        aux2=operand;
        operand&=areg;
        operand>>=1;
        if((psr&Cflag)!=0)
            operand|=0x80;
        psr&=NNZCVF;
        psr|=f.readFlags(operand);
        aux1=(operand^areg);
    }
    else {
        psr&=NNZCVF;
        operand&=0xff;
        aux2=operand;
        aux1=0x00;
        if((operand&0x1)==1)
            aux1=0x80;
        operand&=areg;
        operand>>=1;
        operand|=aux1;
        aux1=(operand&Uflag)<<1;
        aux2=operand&Vflag;
        areg=operand;
        psr|=f.readFlags(areg)|((aux1^aux2)&Vflag);
        psr|=(aux2>>6)&0xff;
    }
}

void ASR() {
    AND();
    LSRA();
}

void DCP() {
    DEC();
    CMP();
}

void ISB() {
    INC();
    SBC();
}

void LAX() {
    psr&=NNZF;
    xreg=areg=operand&0xff;
    psr|=f.readFlags(xreg);
}

void LAS() {
    psr&=NNZF;
    psp&=operand&0xff;
    xreg=areg=psp;
    psr|=f.readFlags(xreg);
}

void LXA() {
    psr&=NNZF;
    areg|=0xee;
    areg&=operand&0xff;
    psr|=f.readFlags(areg);
    xreg=areg;
}

void SBX() {
    psr&=NNZCF;
    operand&=0xff;
    xreg&=areg&0xff;
    if(xreg>operand) {
        psr|=Cflag;
    }
    xreg=(xreg-operand)&0xff;
    psr|=f.readFlags(xreg);
}

void RLA() {
    ROLM();
    AND();
}

void RRA() {
    RORM();
    ADC();
}

void SAX() {
    operand=areg&xreg;
    Bus.writeByte(operand,address);
}

void SHAa() {
    address=Bus.readByte(pc1)&0xff;
    aux2=address;
    pc1++;
    operand=Bus.readByte(address);
    address++;
    operand=Bus.readByte(address);
    address=operand&0xffff;
    aux2=aux1=(address&0xff00);
    address+=(yreg&0xff);
    aux2>>=8;
    aux1|=(address&0xff);
    operand=Bus.readByte(aux1);
    operand&=(aux2+1)&xreg&areg;
    Bus.writeByte(operand,address);
}

void SHAb() {
    address=Bus.readWord(pc1);
    aux2=aux1=(address&0xff00);
    pc1+=2;
    aux2>>=8;
    address+=yreg;
    aux1|=(address&0xff);
    operand=Bus.readByte(aux1);
    operand=areg&xreg&aux2;
    Bus.writeByte(operand,address);
}
void SHS() {
    address=Bus.readWord(pc1);
    aux1=(address>>8)&0xff;
    pc1+=2;
    address+=yreg;
    operand=areg&xreg;
    psp=operand&0xff;
    aux1++;
    operand&=aux1;
    Bus.writeByte(operand,address);
}

void SHX() {
    address=Bus.readWord(pc1);
    operand=((address+0x100)>>8)&0xff;
    pc1+=2;
    address+=yreg;
    operand&=xreg;
    Bus.writeByte(operand,address);
}

void SHY() {
    address=Bus.readWord(pc1);
    operand=((address+0x100)>>8)&0xff;
    pc1+=2;
    address+=xreg;
    operand&=yreg;
    Bus.writeByte(operand,address);
}

void SLO() {
    ASLM();
    ORA();
}

void SRE() {
    LSRM();
    EORA();
}

void JAM() {
    pc1=pc2;
    inst--;
}

    
    private void process(){
        pc1&=0xffff;
        pc2=pc1;
        if(Bus.readNMI()){
            Bus.clearNMI();
            psr&=(0xff-Bflag);
            psr|=Uflag;
            Bus.pushWord(pc1,psp);
            psp=(psp-2)&0xff;
            psr|=Uflag;
            Bus.pushByte(psr,psp);
            psr|=Iflag;
            psp=(psp-1)&0xff;
            pc1=Bus.readWord(0xfffa);
            clock+=7;
            count+=7;
            inst+=1;
        }

        if((Bus.readIRQ())&&((psr&Iflag)==0)){
            psr&=(0xff-Bflag);
            Bus.pushWord(pc1,psp);
            psp=(psp-2)&0xff;
            psr|=Uflag;
            Bus.pushByte(psr,psp);
            psp=(psp-1)&0xff;
            psr|=Iflag;
            pc1=Bus.readWord(0xfffe);
            clock+=7;
            count+=7;
            inst+=1;
        }

    nmemonic=Bus.readByte(pc1);
    pc1=(pc1+1)&0xffff;
    inst++;
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
    };

    public void execute(){
    count=0;
    if(trace==0x0) {
        while(count<cycles) {
            pc1&=0xffff;
            if(pc1==0x00) {
                trace=0xff;
                break;
            }
            else {
                process();
            }
        }
    }           
    };
    
    public void hardReset(){
        areg=0;
        xreg=0;
        yreg=0;      //address registers
        operand=0;
        address=0;
        aux1=aux2=trace=0;
        pc1=Bus.readWord(0xfffc);
        pc2=pc1;   //program counters
        psp=0xff;        //stack pointer
        psr=0x00;         //status register
        psr|=Uflag;      
        inst+=1;          //instructions executed
        clock+=7;          //total number of cycles
        cycles=0;          //cycles to execute
        count=0;           //count of cycles executed       
    };
    
    public void setCycles(int newCycles){
        cycles=newCycles;
    }; 
    
    public int flags(){
        return (psr&0xff);
    };
    
    public String readPC1(){
        return (Integer.toString(pc1,16));
    };
    
    public String readPC2(){
        return (Integer.toString(pc2,16));
    };
    
    public String readSPreg(){
        return (Integer.toString(psp,16));
    }; 
    
    public String readAreg(){
        return (Integer.toString(areg,16));
    };
    
    public String readXreg(){
        return (Integer.toString(xreg,16));
    };    
    
    public String readYreg(){
        return (Integer.toString(yreg,16));
    };    
    
    /** Creates a new instance of CPU */
    public CPU(AddressBus b) {
        Bus=b;
        t=new Timing();
        f=new Flags();
    };
}
