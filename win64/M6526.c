
#include "AddressBus.h"
#include "CBM64.h"
#define CIAPRA   0x0
#define CIAPRB   0x1
#define CIADDRA   0x2
#define CIADDRB   0x3
#define CIATALO         0x4
#define CIATAHI         0x5
#define CIATBLO         0x6
#define CIATBHI         0x7
#define CIATODTENS      0x8
#define CIATODSECS   0x9
#define CIATODMINS      0xa
#define CIATODHOUR      0xb
#define CIASDR   0xc
#define CIAICR   0xd
#define CIACRA   0xe
#define CIACRB   0xf

#define STARTT  0x01
#define PBON  0x02
#define OUTMODE  0x04
#define RUNMODE  0x08
#define FORCE  0x10
#define INMODEA  0x20
#define SPMODE  0x40
#define TODALARM 0x80
//#define STOPTMR     0xfe

#define OUTPRB  0x02
#define TOGGLE  0x04
#define PRB6  0x40
#define PRB7  0x80

#define TAINT  0x01
#define TBINT  0x02
#define TODINT  0x04
#define INTER         0x80
#define CIAAIRQ     0x00ff

struct TODDLER {
    unsigned char tenths;
    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
};

union TODtime
{
    struct TODDLER t;
    unsigned int time;
};

struct TODCLOCK {
    union TODtime count;    //actual time
    union TODtime latch;    //latch
    unsigned int   period;           //period for cycles
    unsigned int   cycles;           //number of cycles executed
    unsigned char  start;            //flags
    unsigned char  update;
    unsigned char  show;
};

struct TIME {
    unsigned char lo;
    unsigned char hi;
    unsigned char dm1;
    unsigned char dm2;
};

union CIATIME
{
    struct TIME t;
    unsigned int value;
};

struct CIATIMER {
    union CIATIME count;
    union CIATIME latch;
    unsigned char CNT,PHI2,CTA,Pulse;
};


unsigned char aux1,aux2,aux3,aux4;

struct CIACHIP {
    unsigned char id;
    unsigned char CIAMASK,CIAFLAG;
    unsigned int cycles;
    unsigned int TODperiod;
    unsigned char CiaChip[0x10];
    struct CIATIMER TA;
    struct CIATIMER TB;
    struct TODCLOCK TOD;
};

struct CIACHIP CiaChipA,CiaChipB;
struct CIACHIP* C;

void M6526LoadCia(struct CIACHIP *C,unsigned char *p) {
    unsigned int i;
    for(i=0;i<0x10;i++) {
        C->CiaChip[i]=p[i];
    }

    C->TB.latch.t.lo=p[40];
    C->TB.latch.t.hi=p[41];
    C->TB.count.t.lo=p[44];
    C->TB.count.t.lo=p[45];
    C->TB.CNT=p[16];
    C->TB.CTA=p[17];
    C->TB.PHI2=p[18];
    C->TB.Pulse=p[52];

    C->CIAMASK=p[29];

    C->TA.latch.t.lo=p[32];
    C->TA.latch.t.hi=p[33];
    C->TA.count.t.lo=p[36];
    C->TA.count.t.lo=p[37];
    C->TA.CNT=p[49];
    C->TA.PHI2=p[48];
    C->TA.Pulse=p[51];

    C->TOD.start=p[53];
    C->TOD.update=p[54];

    C->TOD.latch.t.tenths=p[56];
    C->TOD.latch.t.seconds=p[57];
    C->TOD.latch.t.minutes=p[58];
    C->TOD.latch.t.hours=p[59];

    C->TOD.count.t.tenths=p[60];
    C->TOD.count.t.seconds=p[61];
    C->TOD.count.t.minutes=p[62];
    C->TOD.count.t.hours=p[63];

}

void M6526SaveCia(struct CIACHIP *C,unsigned char *p) {
    unsigned int i;
    for(i=0;i<0x10;i++) {
        p[i]=C->CiaChip[i];
    }
    p[40]=C->TB.latch.t.lo;
    p[41]=C->TB.latch.t.hi;
    p[44]=C->TB.count.t.lo;
    p[45]=C->TB.count.t.lo;
    p[16]=C->TB.CNT;
    p[17]=C->TB.CTA;
    p[18]=C->TB.PHI2;
    p[52]=C->TB.Pulse;

    p[29]=C->CIAMASK;

    p[32]=C->TA.latch.t.lo;
    p[33]=C->TA.latch.t.hi;
    p[36]=C->TA.count.t.lo;
    p[37]=C->TA.count.t.lo;
    p[49]=C->TA.CNT;
    p[48]=C->TA.PHI2;
    p[51]=C->TA.Pulse;

    p[53]=C->TOD.start;
    p[54]=C->TOD.update;

    p[56]=C->TOD.latch.t.tenths;
    p[57]=C->TOD.latch.t.seconds;
    p[58]=C->TOD.latch.t.minutes;
    p[59]=C->TOD.latch.t.hours;

    p[60]=C->TOD.count.t.tenths;
    p[61]=C->TOD.count.t.seconds;
    p[62]=C->TOD.count.t.minutes;
    p[63]=C->TOD.count.t.hours;
}

void M6526LoadState(struct ioState *p) {
    M6526LoadCia(&CiaChipA,p->cia1);
    M6526LoadCia(&CiaChipB,p->cia2);
}

void M6526SaveState(struct ioState *p) {
    M6526SaveCia(&CiaChipA,p->cia1);
    M6526SaveCia(&CiaChipB,p->cia2);
}

void M6526HardReset(void) {
    unsigned int i=0;
    while(i<0x10) {
        CiaChipA.CiaChip[i]=0;
        CiaChipB.CiaChip[i]=0;
        i++;
    }
    CiaChipA.CIAMASK=CiaChipB.CIAMASK=0;
    CiaChipA.CIAFLAG=CiaChipB.CIAFLAG=0;
    CiaChipA.id=0;
    CiaChipA.TOD.count.time=CiaChipA.TOD.latch.time=0;
    CiaChipA.TA.count.value=CiaChipA.TA.latch.value=0xffff;
    CiaChipA.TB.count.value=CiaChipA.TB.latch.value=0xffff;
    CiaChipB.id=1;
    CiaChipB.TOD.count.time=CiaChipB.TOD.latch.time=0;
    CiaChipB.TA.count.value=CiaChipB.TA.latch.value=0xffff;
    CiaChipB.TB.count.value=CiaChipB.TB.latch.value=0xffff;
}

void M6526SetCycles(unsigned int period) {
    CiaChipA.cycles=period&0xffff;
    CiaChipB.cycles=period&0xffff;
}

/*-----------------------------------------------------------------*/
////////////////////////     CIA      //////////////////////////////

void M6526DumpCiaChip(unsigned char id) {
    unsigned char temp;
    if(id) {
        C=&CiaChipB;
    }
    else {
        C=&CiaChipA;
    }
    temp=C->CiaChip[CIACRA];
    aStringToBuffer("\n");
    if(temp&1) {
        aStringToBuffer("Timer A : ON\n");
    }
    else {
        aStringToBuffer("Timer A : OFF\n");
    }
    aStringToBuffer(" latch  : ");
    anIntToBuffer(C->TA.latch.value,2,1);
    aStringToBuffer("\n count  : ");
    anIntToBuffer(C->TA.count.value,2,1);
    aStringToBuffer("\n\n");
    temp=C->CiaChip[CIACRB];
    if(temp&1) {
        aStringToBuffer("Timer B : ON\n");
    }
    else {
        aStringToBuffer("Timer B : OFF\n");
    }
    aStringToBuffer(" latch  : ");
    anIntToBuffer(C->TB.latch.value,2,1);
    aStringToBuffer("\n count  : ");
    anIntToBuffer(C->TB.count.value,2,1);
    aStringToBuffer("\n\n");

    aStringToBuffer("interrupts\n");
    temp=C->CiaChip[CIAICR];
    if(temp&0x1f) {
        aStringToBuffer(" Pending :");
        if(temp&1)
            aStringToBuffer(" TA");
        if(temp&2)
            aStringToBuffer(" TB");
        aStringToBuffer("\n");
    }
    else {
        aStringToBuffer(" Pending : None\n");
    }
    temp=C->CIAMASK;
    if(temp&0x1f) {
        aStringToBuffer(" Enabled :");
        if(temp&1)
            aStringToBuffer(" TA");
        if(temp&2)
            aStringToBuffer(" TB");
        aStringToBuffer("\n");
    }
    else {
        aStringToBuffer(" Enabled : None\n");
    }
    aStringToBuffer("\n");
}

unsigned char M6526ReadCIA(unsigned char address,unsigned char id) {
    address&=0xf;
    if(id) {
        C=&CiaChipB;
    }
    else {
        C=&CiaChipA;
    }
    switch(address) {
        case 0x00: //pra
        if(C->id) {
            aux1=C->CiaChip[CIAPRA];
            aux1=(aux1|(~C->CiaChip[CIADDRA]))&0x3f;
        }
        else {
            aux1=C->CiaChip[CIAPRA]|(!C->CiaChip[CIADDRA]);
            aux2=C->CiaChip[CIAPRB]|(!C->CiaChip[CIADDRB]);
            aux2&=GetJoy1();
            aux3=0x01;
            aux4=0;
            while(aux3!=0) {
                if(!(aux2&aux3)) {
                    aux1&=GetInvKeyArr(aux4);
                }
                aux3<<=1;
                aux4++;
            }
            return (aux1&GetJoy2());
        }
        return aux1;
        case 0x01: //prb
        if(C->id) {
            aux1=C->CiaChip[CIAPRB];
            aux1=(aux1|(~C->CiaChip[CIADDRB]));
        }
        else {
            aux1=~C->CiaChip[CIADDRB];
            aux2=C->CiaChip[CIAPRA]|(~C->CiaChip[CIADDRA]);
            aux2&=GetJoy2();
            aux3=0x01;
            aux4=0;
            while(aux3!=0) {
                if(!(aux2&aux3)) {
                    aux1&=GetKeyArr(aux4);
                }
                aux3<<=1;
                aux4++;
            }
            aux1|=(C->CiaChip[CIAPRB]&C->CiaChip[CIADDRB]);
            aux1=(aux1&GetJoy1());
        }
        return aux1;
        break;
        case 0x02: //ddra
        case 0x03: //ddrb
        return C->CiaChip[address];
        case 0x04: //timerAlo
        return C->TA.count.t.lo;
        case 0x05: //timerAhi
        return C->TA.count.t.hi;
        case 0x06: //timerBlo
        return C->TB.count.t.lo;
        case 0x07: //timerBhi
        return C->TB.count.t.hi;
        case 0x08: //tens
        aux1=C->CiaChip[address];
        C->TOD.update=0x00;
        return aux1;
        case 0x09: //TODsecs
        case 0x0A: //TODmins
        return C->CiaChip[address];
        case 0x0B: //TODhour
        aux1=C->CiaChip[address];
        C->TOD.update=0xff;
        return aux1;
        case 0x0C: //ciasdr
        return C->CiaChip[address];
        case 0x0D: //ciaicr
        aux1=C->CiaChip[CIAICR];
        C->CiaChip[CIAICR]=0;
        if(C->id) {
            //            BusClearNMI();
        }
        else {
            BusClearIRQ(CIAAIRQ);
        }
        return (aux1&0x9f);
        case 0x0E: //cra
        case 0x0F: //crb
        return C->CiaChip[address];
    }
    return(0);
}


void M6526WriteCIA(unsigned char byte,unsigned char address,unsigned char id) {
    address&=0xf;
    if(id) {
        C=&CiaChipB;
    }
    else {
        C=&CiaChipA;
    }
    switch(address) {
        case 0x00: //pra
        if(C->id) {
            BusUpDateVicII();
        }
        C->CiaChip[address]=byte;
        break;
        case 0x01: //prb
        C->CiaChip[address]=byte;
        case 0x02: //ddra
        case 0x03: //ddrb
        C->CiaChip[address]=byte;
        break;
        case 0x04: //timerAlo
        C->CiaChip[CIATALO]=byte;
        break;
        case 0x05: //timerAhi
        C->CiaChip[CIATAHI]=byte;
        C->TA.latch.t.hi=byte;
        C->TA.latch.t.lo=C->CiaChip[CIATALO];
        C->TA.latch.value&=0xffff;
        if((C->CiaChip[CIACRA]&STARTT)==0) {
            C->TA.count.value=C->TA.latch.value;
        }
        break;
        case 0x06: //timerBlo
        C->CiaChip[CIATBLO]=byte;
        break;
        case 0x07: //timerBhi
        C->CiaChip[CIATBHI]=byte;
        C->TB.latch.t.hi=byte;
        C->TB.latch.t.lo=C->CiaChip[CIATBLO];
        C->TB.latch.value&=0xffff;
        if((C->CiaChip[CIACRB]&STARTT)==0) {
            C->TB.count.value=C->TB.latch.value;
        }
        break;
        case 0x08: //tens
        byte&=0xf;
        C->TOD.start=0xff;
        C->CiaChip[address]=byte;
        if(C->CiaChip[CIACRB]&TODALARM) {
            C->TOD.latch.t.tenths=byte;
        }
        else {
            C->TOD.count.t.tenths=byte;
        }
        break;
        case 0x09: //TODsecs
        byte&=0x3f;
        C->CiaChip[address]=byte;
        if(C->CiaChip[CIACRB]&TODALARM) {
            C->TOD.latch.t.seconds=byte;
        }
        else {
            C->TOD.count.t.seconds=byte;
        }
        break;
        case 0x0A: //TODmins
        byte&=0x3f;
        C->CiaChip[address]=byte;
        if(C->CiaChip[CIACRB]&TODALARM) {
            C->TOD.latch.t.minutes=byte;
        }
        else {
            C->TOD.count.t.minutes=byte;
        }
        break;
        case 0x0B: //TODhour
        byte&=0x1f;
        C->TOD.start=0x00;
        C->CiaChip[address]=byte;
        if(C->CiaChip[CIACRB]&TODALARM) {
            C->TOD.latch.t.hours=byte;
        }
        else {
            C->TOD.count.t.hours=byte;
        }
        break;
        case 0x0C: //ciasdr
        C->CiaChip[address]=byte;
        break;
        case 0x0D: //ciaicr
        aux1=C->CIAMASK&0x1f;

        C->CiaChip[CIAICR]=0;
        if(C->id==0) {
            BusClearIRQ(CIAAIRQ);
        }

        if(byte&0x80) {
            byte|=aux1;
        }
        else {
            byte=aux1&(~byte);
        }
        C->CIAMASK=byte&0x1f;
        break;
        case 0x0E: //cra
        if(byte&FORCE) {
            byte&=(0xff-FORCE);
            C->TA.latch.t.hi=C->CiaChip[CIATAHI];
            C->TA.latch.t.lo=C->CiaChip[CIATALO];
            C->TA.latch.value&=0xffff;
            C->TA.count.value=C->TA.latch.value;
        }
        C->CiaChip[CIACRA]=byte;
        break;
        case 0x0F: //crb
        if(byte&FORCE) {
            byte&=(0xff-FORCE);
            C->TB.latch.t.hi=C->CiaChip[CIATBHI];
            C->TB.latch.t.lo=C->CiaChip[CIATBLO];
            C->TB.latch.value&=0xffff;
            C->TB.count.value=C->TB.latch.value;
        }
        C->CiaChip[CIACRB]=byte;
        byte&=(32+64);
        if(byte) {
            if(!(byte&0x40)) {
                C->TB.CTA=0xff;
            }
        }
        else {
            C->TB.PHI2=0xff;
        }
        break;
    }
}

void TODexecute(void) {
    if(C->TOD.start) {
        if(C->TOD.count.time<(C->TOD.cycles)) {
            C->TOD.count.t.tenths++;
            if(C->TOD.count.t.tenths>9) {
                C->TOD.count.t.tenths=0;
                aux3=C->TOD.count.t.seconds&0x0f;
                aux4=C->TOD.count.t.seconds&0xf0;
                aux3++;
                if(aux3>9) {
                    aux4+=0x10;
                    aux3=0x00;
                }
                C->TOD.count.t.seconds=aux3|aux4;
                if(C->TOD.count.t.seconds>=0x60) {
                    C->TOD.count.t.seconds=0;
                    aux3=C->TOD.count.t.minutes&0x0f;
                    aux4=C->TOD.count.t.minutes&0xf0;
                    aux3++;
                    if(aux3>9) {
                        aux4+=0x10;
                        aux3=0x00;
                    }
                    C->TOD.count.t.minutes=aux3|aux4;
                    if(C->TOD.count.t.minutes>=0x60) {
                        C->TOD.count.t.minutes=0;
                        aux2=C->TOD.count.t.hours&0x80;
                        aux3=C->TOD.count.t.hours&0x0f;
                        aux4=C->TOD.count.t.hours&0x70;
                        aux3++;
                        if(aux3>9) {
                            aux4+=0x10;
                            aux3=0x00;
                        }
                        C->TOD.count.t.hours=aux3|aux4;
                        if(C->TOD.count.t.hours>0x12) {
                            aux2=(aux2^0x80)&0x80;
                            C->TOD.count.t.hours=0x01;
                        }
                        C->TOD.count.t.hours|=aux2;
                    }
                }
            }

            if(C->TOD.count.time>=C->TOD.latch.time) {
                C->CiaChip[CIAICR]|=TODINT;
                if(C->CIAMASK&TODINT) {
                    C->CiaChip[CIAICR]|=INTER; //flag an interupt
                    if(C->id) {
                        BusSetNMI();      //signal NMI
                    }
                    else {
                        BusSetIRQ(CIAAIRQ);      //signal IRQ
                    }
                }
            }
        }
        else {
            C->TOD.count.time-=C->TOD.cycles;
        }
    }
}

void CIASORTTB(void) {
    C->TB.count.value=C->TB.latch.value; //rest timer
    if(aux1&RUNMODE) {
        C->CiaChip[CIACRB]=aux1&(0xff-STARTT);    // stop timer B
        C->TB.PHI2=0;             // kill PHI2
        C->TB.CNT=0;              // kill CNT
        C->TB.CTA=0;              // kill CTA
    }
    C->CiaChip[CIAICR]|=TBINT;    //set interupt for timer B
    if(C->CIAMASK&TBINT) {
        C->CiaChip[CIAICR]|=INTER; //flag an interupt
        if(C->id) {
            BusSetNMI();      //signal NMI
        }
        else {
            BusSetIRQ(CIAAIRQ);      //signal IRQ
        }
    }

    // is timer chained with timer A
    if(aux1&OUTPRB) {
        C->CiaChip[CIADDRB]&=0x7f; // bit 7 to output mode
        if(aux1&TOGGLE) {
            C->CiaChip[CIAPRB]^=PRB7; // TOGGLE on
        }
        else {
            C->CiaChip[CIAPRB]|=PRB7;   //PULSE
            C->TB.Pulse=0xff;
        }
    }
}

void CIA(void) {
    aux1=C->CiaChip[CIACRA];
    //is timer A running
    if(aux1&STARTT) {
        //is timer A going to finish counting down
        C->TA.count.value-=C->cycles;
        if(C->TA.count.t.dm1) {
            // is timer chained with timer B
            if(aux1&OUTPRB) {
                C->CiaChip[CIADDRB]&=0xbf; // bit 6 to output mode
                if(aux1&TOGGLE) {
                    C->CiaChip[CIAPRB]^=PRB6; // TOGGLE on
                }
                else {
                    C->CiaChip[CIAPRB]|=PRB6;   //PULSE
                    C->TA.Pulse=0xff;
                }
            }
            C->TA.count.value=C->TA.latch.value; //rest timer
            if(aux1&RUNMODE) {
                // stop timer A
                C->CiaChip[CIACRA]=aux1&(0xff-STARTT);
                C->TA.PHI2=0;             // kill PHI2
                C->TA.CNT=0;              // kill CNT
            }

            C->CiaChip[CIAICR]|=TAINT;    //set interupt for timer A
            if(C->CIAMASK&TAINT) {
                C->CiaChip[CIAICR]|=INTER; //flag an interupt
                if(C->id) {
                    BusSetNMI();      //signal NMI
                }
                else {
                    BusSetIRQ(CIAAIRQ);      //signal IRQ
                }
            }

            if(C->TB.CTA)                //Timer B count Timer A
            {
                C->TB.count.value--;
                if(C->TB.count.t.dm1) {
                    aux1=C->CiaChip[CIACRB];
                    CIASORTTB();
                }
            }
        }
    }

    // Work Timer B now
    if(C->TB.PHI2) {
        C->TB.count.value-=C->cycles;
        if(C->TB.count.t.dm1) {
            aux1=C->CiaChip[CIACRB];
            CIASORTTB();
        }
    }
    TODexecute();
}

void M6526Execute(void) {
    C=&CiaChipA;
    CIA();
    C=&CiaChipB;
    CIA();
}




