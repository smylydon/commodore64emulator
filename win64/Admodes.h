void ReadImm(void) {
    clock+=2;
    count+=2;
    operand.value=BusReadByte(pc1.value)&0xff;
    pc1.value++;
}

void ReadZpg(void) {
    clock+=3;
    count+=3;
    address.value=BusReadByte(pc1.value)&0xff;
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value++;
}

void ReadZpgX(void) {
    clock+=4;
    count+=4;
    address.value=(BusReadByte(pc1.value)+xreg.r.lo)&0xff;
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value++;
}

void ReadZpgY(void) {
    clock+=4;
    count+=4;
    address.value=(BusReadByte(pc1.value)+yreg.r.lo)&0xff;
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value++;
}

void ReadAbs(void) {
    clock+=4;
    count+=4;
    address.value=BusReadWord(pc1.value);
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value+=2;
}

void ReadAbsX(void) {
    clock+=4;
    count+=4;
    xreg.value&=0xff;
    address.value=BusReadWord(pc1.value);
    aux1.r.hi=address.r.hi;
    address.value+=xreg.value;
    if(address.r.hi!=aux1.r.hi) {
        count++;
        clock++;
        aux1.r.lo=address.r.lo;
        operand.r.lo=BusReadByte(aux1.value);
    }
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value+=2;
}

void ReadAbsY(void) {
    clock+=4;
    count+=4;
    yreg.value&=0xff;
    address.value=BusReadWord(pc1.value);
    aux1.r.hi=address.r.hi;
    address.value+=yreg.value;
    if(address.r.hi!=aux1.r.hi) {
        count++;
        clock++;
        aux1.r.lo=address.r.lo;
        operand.r.lo=BusReadByte(aux1.value);
    }
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value+=2;
}

void ReadIndX(void) {
    clock+=6;
    count+=6;
    xreg.value&=0xff;
    address.value=BusReadByte(pc1.value)&0xff;
    address.r.lo+=xreg.r.lo;
    address.value=BusReadBounds(address.value);
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value++;
}

void ReadIndY(void) {
    clock+=5;
    count+=5;
    yreg.value&=0xff;
    address.value=BusReadByte(pc1.value)&0xff;
    address.value=BusReadBounds(address.value);
    aux1.r.hi=address.r.hi;
    address.value=(address.value+yreg.value)&0xffff;
    if(aux1.r.hi!=address.r.hi) {
        count++;
        clock++;
        aux1.r.lo=address.r.lo;
        operand.r.lo=BusReadByte(aux1.value);
    }
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value++;
}

//////////

void StoreZpg(void) {
    clock+=3;
    count+=3;
    address.value=BusReadByte(pc1.value)&0xff;
    pc1.value++;
}

void StoreZpgX(void) {
    clock+=4;
    count+=4;
    address.value=BusReadByte(pc1.value)&0xff;
    address.r.lo+=xreg.r.lo;
    pc1.value++;
}

void StoreZpgY(void) {
    clock+=4;
    count+=4;
    address.value=BusReadByte(pc1.value)&0xff;
    address.r.lo+=yreg.r.lo;
    pc1.value++;
}

void StoreAbs(void) {
    clock+=4;
    count+=4;
    address.value=BusReadWord(pc1.value);           //r 2+3
    pc1.value+=2;
}

void StoreAbsX(void) {
    clock+=5;
    count+=5;
    xreg.value&=0xff;
    address.value=BusReadWord(pc1.value);
    aux1.r.hi=address.r.hi;
    address.value=(address.value+xreg.value)&0xffff;
    aux1.r.lo=address.r.lo;
    aux2.r.lo=BusReadByte(aux1.value);
    pc1.value+=2;
}

void StoreAbsY(void) {
    clock+=5;
    count+=5;
    yreg.value&=0xff;
    address.value=BusReadWord(pc1.value);
    aux1.r.hi=address.r.hi;
    address.value=(address.value+yreg.value)&0xffff;
    aux1.r.lo=address.r.lo;
    aux2.r.lo=BusReadByte(aux1.value);
    pc1.value+=2;
}

void StoreIndX(void) {
    clock+=6;
    count+=6;
    aux1.value=(BusReadByte(pc1.value)+xreg.r.lo)&0xff;
    address.value=BusReadBounds(aux1.value);
    pc1.value++;
}

void StoreIndY(void) {
    clock+=6;
    count+=6;
    yreg.value&=0xff;
    aux1.value=BusReadByte(pc1.value)&0xff;
    address.value=BusReadBounds(aux1.value);
    aux1.value=address.value;
    aux1.r.lo=aux1.r.lo+yreg.r.lo;
    address.value=(address.value+yreg.value)&0xffff;
    aux2.value=BusReadByte(aux1.value);
    pc1.value++;
}

void StoreRWMZpg(void) {
    clock+=5;
    count+=5;
    address.value=BusReadByte(pc1.value)&0xff;
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value++;
}

void StoreRWMZpgX(void) {
    clock+=6;
    count+=6;
    address.value=BusReadByte(pc1.value)&0xff;
    address.r.lo+=xreg.r.lo;
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value++;
}

void StoreRWMZpgY(void) {
    clock+=6;
    count+=6;
    address.value=BusReadByte(pc1.value)&0xff;
    address.r.lo+=yreg.r.lo;
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value++;
}

void StoreRWMAbs(void) {
    clock+=6;
    count+=6;
    address.value=BusReadWord(pc1.value);           //r 2+3
    operand.value=BusReadByte(address.value)&0xff;  //r4
    BusWriteByte(operand.r.lo,address.value);       //w1
    pc1.value+=2;
}

void StoreRWMAbsX(void) {
    clock+=7;
    count+=7;
    xreg.value&=0xff;
    address.value=BusReadWord(pc1.value);           //r 2+3
    aux1.r.hi=address.r.hi;
    address.value=(address.value+xreg.value)&0xffff;
    aux1.r.lo=address.r.lo;
    aux2.r.lo=BusReadByte(aux1.value);              //r4
    operand.value=BusReadByte(address.value)&0xff;  //r5
    BusWriteByte(operand.r.lo,address.value);       //w1
    pc1.value+=2;
}

void StoreRWMAbsY(void) {
    clock+=7;
    count+=7;
    yreg.value&=0xff;
    address.value=BusReadWord(pc1.value);           //r 2+3
    aux1.r.hi=address.r.hi;
    address.value=(address.value+yreg.value)&0xffff;
    aux1.r.lo=address.r.lo;
    aux2.r.lo=BusReadByte(aux1.value);              //r4
    operand.value=BusReadByte(address.value)&0xff;  //r5
    BusWriteByte(operand.r.lo,address.value);       //w1
    pc1.value+=2;
}

void StoreRWMIndX(void) {
    clock+=8;
    count+=8;
    aux1.value=(BusReadByte(pc1.value)+xreg.r.lo)&0xff;
    address.value=BusReadBounds(aux1.value);
    operand.r.lo=BusReadByte(address.value);
    BusWriteByte(operand.r.lo,address.value);
    pc1.value++;
}

void StoreRWMIndY(void) {
    clock+=8;
    count+=8;
    yreg.value&=0xff;
    aux1.value=BusReadByte(pc1.value)&0xff;
    address.value=BusReadBounds(aux1.value);
    aux1.value=address.value;
    aux1.r.lo=aux1.r.lo+yreg.r.lo;
    address.value=(address.value+yreg.value)&0xffff;
    aux2.value=BusReadByte(aux1.value);
    operand.value=BusReadByte(address.value)&0xff;
    pc1.value++;
}

