
char ReadSID[0x20],WriteSID[0x20];

void M6581HardReset(void) {
    int i=0;
    while(i<0x20) {
        ReadSID[i]=0;
        WriteSID[i]=0;
        i++;
    }
}

void M6581Initialize(void) {
    ReadSID[0]=0;
}

char M6581ReadSID(char address) {
    return ReadSID[address&0x1f];
}

void M6581WriteSID(char byte,char address) {
    WriteSID[address&0x1f]=byte;
}

void M6581Execute(void){
     return;
}
