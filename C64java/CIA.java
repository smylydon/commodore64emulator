/*
 * CIA.java
 *
 * Created on February 2, 2006, 8:54 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

/**
 *
 * @author Owner
 */



public class CIA {
public static final int CIAPRA=0x0;
public static final int CIAPRB=0x1;
public static final int CIADDRA=0x2;
public static final int CIADDRB=0x3;
public static final int CIATALO=0x4;
public static final int CIATAHI=0x5;
public static final int CIATBLO=0x6;
public static final int CIATBHI=0x7;
public static final int CIATODTENS=0x8;
public static final int CIATODSECS=0x9;
public static final int CIATODMINS=0xa;
public static final int CIATODHOUR=0xb;
public static final int CIASDR=0xc;
public static final int CIAICR=0xd;
public static final int CIACRA=0xe;
public static final int CIACRB=0xf;

public static final int STARTT=0x01;
public static final int PBON=0x02;
public static final int OUTMODE=0x04;
public static final int RUNMODE=0x08;
public static final int FORCE=0x10;
public static final int INMODEA=0x20;
public static final int SPMODE=0x40;
public static final int TODALARM=0x80;
public static final int STOPTMR=0xfe;

public static final int OUTPRB=0x02;
public static final int TOGGLE=0x04;
public static final int PRB6=0x40;
public static final int PRB7=0x80;

public static final int TAINT=0x01;
public static final int TBINT=0x02;
public static final int TODINT=0x04;
public static final int INTER=0x80;
public static final int CIAAIRQ=0x0f;    
    
    
    private int cycles,aux1,aux2,aux3,aux4,i,CIAMASK;
    private CIATIMER TA,TB;
    private int todPeriod,todUpdate;
    private int[] ciaChip;
    private TODCLOCK TOD;
    private AddressBus Bus;
    private boolean id;
    /** Creates a new instance of CIA */
    public CIA(boolean AorB,AddressBus b) {
        id=AorB;
        ciaChip=new int[0x10];
        TA=new CIATIMER();
        TB=new CIATIMER();
        TOD=new TODCLOCK();
        Bus=b;
    };
    
    public void inialise(){
        
    };
    
     public void hardReset(){
        TA.reset();
        TB.reset();
        TOD.reset();
        CIAMASK=0;
        for(i=0;i<0x10;i++){
            ciaChip[i]=0;
        }        
    };  
    
    public void setCycles(int newCycles){
        cycles=newCycles;
    };
    
    public int GetKeyArr(int index){
        return 0xff;
    };
    
    public int GetInvKeyArr(int index){
        return 0xff;
    };
    
    public int GetJoy1(){
        return 0xff;
    };
    
    public int GetJoy2(){
        return 0xff;
    };
    
    public int readCIA(int address){
        address&=0xf;
        switch(address){
            case 0x00: //pra
                if(id){
                    aux1=ciaChip[CIAPRA];
                    aux1=(aux1|(~ciaChip[CIADDRA]))&0x3f;
                }
                else{
                    aux1=ciaChip[CIAPRA]|(~ciaChip[CIADDRA]);
                    aux2=ciaChip[CIAPRB]|(~ciaChip[CIADDRB]);
                    aux2&=GetJoy1();
                    aux3=0x01;
                    aux4=0;
                    while(aux3!=0){
                        if((aux2&aux3)!=0){
                          aux1&=GetInvKeyArr(aux4);
                        }
                        aux3<<=1;
                        aux4++;
                    }
                    aux1&=GetJoy2();
                };
                return aux1;
            case 0x01: //prb
                if(id){
                    aux1=ciaChip[CIAPRB];
                    aux1=(aux1|(~ciaChip[CIADDRB]));
                    return aux1;
                }
                else{
                    aux1=~ciaChip[CIADDRB];
                    aux2=ciaChip[CIAPRA]|(~ciaChip[CIADDRA]);
                    aux2&=GetJoy2();
                    aux3=0x01;
                    aux4=0;
                    while(aux3!=0){
                        if((aux2&aux3)==0){
                            aux1&=GetKeyArr(aux4);
                        }
                        aux3<<=1;
                        aux4++;
                    }
                    aux1|=(ciaChip[CIAPRB]&ciaChip[CIADDRB]);
                    return (aux1&GetJoy1());
                }
            case 0x02: //ddra
            case 0x03: //ddrb
                return ciaChip[address];
            case 0x04: //timerAlo
                return (TA.count&0xff);
            case 0x05: //timerAhi
                return ((TA.count>>8)&0xff);
            case 0x06: //timerBlo
                return TB.count&0xff;
            case 0x07: //timerBhi
                return ((TB.count>>8)&0xff);
            case 0x08: //tens
                aux1=ciaChip[address];
                TOD.update=false;
                return aux1;
            case 0x09: //TODsecs
            case 0x0A: //TODmins
                return ciaChip[address];
            case 0x0B: //TODhour
                aux1=ciaChip[address];
                TOD.update=true;
                return aux1;
            case 0x0C: //ciasdr
                return ciaChip[address];
            case 0x0D: //ciaicr
                aux1=ciaChip[CIAICR];
                ciaChip[CIAICR]=0;
                if(id){
                    Bus.clearNMI();
                }
                else{
                    Bus.clearIRQ(CIAAIRQ);
                }
                return aux1;
            case 0x0E: //cra
            case 0x0F: //crb
                return ciaChip[address];
        }
        return aux1;
    };
    
    public void writeCIA(int data,int address){
        address&=0xf;
        data&=0xff;
        switch(address){
            case 0x00: //pra
                if(id){
                    Bus.upDateVicII();
                }
                ciaChip[address]=data;
                break;
            case 0x01: //prb
                ciaChip[address]=data;
            case 0x02: //ddra
            case 0x03: //ddrb
                ciaChip[address]=data;
                break;
            case 0x04: //timerAlo
                ciaChip[CIATALO]=data;
                break;
            case 0x05: //timerAhi
                ciaChip[CIATAHI]=data;
                TA.latch=(data<<0x8)&0xff00;
                TA.latch|=(ciaChip[CIATALO]&0xff);
                TA.latch&=0xffff;
                if((ciaChip[CIACRA]&STARTT)!=0){
                    TA.count=TA.latch;
                }
                break;
            case 0x06: //timerBlo
                ciaChip[CIATBLO]=data;
                break;
            case 0x07: //timerBhi
                ciaChip[CIATBHI]=data;
                TB.latch=(data<<8)&0xff00;
                TB.latch|=(ciaChip[CIATBLO]&0xff);
                TB.latch&=0xffff;
                if((ciaChip[CIACRB]&STARTT)!=0){
                    TB.count=TB.latch;
                }
                break;
            case 0x08: //tens
                data&=0xf;
                TOD.start=true;
                ciaChip[address]=data;
                if((ciaChip[CIACRB]&TODALARM)!=0){
                    TOD.latch[0]=data;
                }
                else{
                    TOD.count[0]=data;
                }
                break;
            case 0x09: //TODsecs
                data&=0xf;
                ciaChip[address]=data;
                if((ciaChip[CIACRB]&TODALARM)!=0){
                    TOD.latch[1]=data;
                }
                else{
                    TOD.count[1]=data;
                }
                break;
            case 0x0A: //TODmins
                data&=0xf;
                ciaChip[address]=data;
                if((ciaChip[CIACRB]&TODALARM)!=0){
                    TOD.latch[2]=data;
                }
                else{
                    TOD.count[2]=data;
                }
                break;
            case 0x0B: //TODhour
                data&=0xf;
                ciaChip[address]=data;
                if((ciaChip[CIACRB]&TODALARM)!=0){
                    TOD.latch[3]=data;
                }
                else{
                    TOD.count[3]=data;
                }
                break;
            case 0x0C: //ciasdr
                ciaChip[address]=data;
                break;
            case 0x0D: //ciaicr
                aux1=CIAMASK&0x1f;
             
                ciaChip[CIAICR]=0;
                if(id==false){
                    Bus.clearIRQ(CIAAIRQ);
                };

                if((data&0x80)!=0){
                    data|=aux1;
                }
                else{
                    data=aux1&(~data);
                }
                CIAMASK=data&0x1f;
                break;
            case 0x0E: //cra
                if((data&FORCE)!=0){
                    TA.latch=(ciaChip[CIATAHI]<<8)&0xff00;
                    TA.latch|=(ciaChip[CIATALO]&0xff);
                    TA.latch&=0xffff;
                    TA.count=TA.latch;
                }
                data&=(0xff-FORCE);        
                ciaChip[CIACRA]=data;
                break;
            case 0x0F: //crb
                if((data&FORCE)!=0){
                    TB.latch=(ciaChip[CIATBHI]<<8)&0xff00;
                    TB.latch|=(ciaChip[CIATBLO]&0xff);
                    TB.latch&=0xffff;
                    TB.count=TB.latch;
                }
                data&=(0xff-FORCE);        
                ciaChip[CIACRB]=data;
                data&=(32+64);
                if(data!=0){
                    if((data&0x40)==0){
                        TB.CTA=true;
                    }
                }
                else{
                    TB.PHI2=true;
                }
                break;
        };        
    }; 

    private void sortTB(){
        TB.count=TB.latch; //rest timer
        if((aux1&RUNMODE)!=0){
            ciaChip[CIACRB]=aux1&(0xff-STARTT);    // stop timer B
            TB.PHI2=false;             // kill PHI2
            TB.CNT=false;              // kill CNT
            TB.CTA=false;              // kill CTA
        }
        
        ciaChip[CIAICR]|=TBINT;    //set interupt for timer B
        if((CIAMASK&TBINT)!=0){
            ciaChip[CIAICR]|=INTER; //flag an interupt
            if(id){
                Bus.setNMI();      //signal NMI
            }
            else{
                Bus.setIRQ(CIAAIRQ);      //signal IRQ
            }        
        }
    
        // is timer chained with timer A
        if((aux1&OUTPRB)!=0){
            ciaChip[CIADDRB]&=0x7f; // bit 7 to output mode
            if((aux1&TOGGLE)!=0){
                ciaChip[CIAPRB]^=PRB7; // TOGGLE on
            }
            else{
                ciaChip[CIAPRB]|=PRB7;   //PULSE
                TB.Pulse=true;
            }
        }
    };

    public void timerAB(){
        aux1=ciaChip[CIACRA];
        //is timer A running
        if((aux1&STARTT)!=0){
            //is timer A going to finish counting down
            TA.count-=cycles;
            if((TA.count&0xffff0000)!=0){
                // is timer chained with timer B
                if((aux1&OUTPRB)!=0){
                    ciaChip[CIADDRB]&=0xbf; // bit 6 to output mode
                    if((aux1&TOGGLE)!=0){
                        ciaChip[CIAPRB]^=PRB6; // TOGGLE on
                    }
                    else{
                        ciaChip[CIAPRB]|=PRB6;   //PULSE
                        TA.Pulse=true;
                    }
                }
                
                TA.count=TA.latch; //rest timer
                if((aux1&RUNMODE)!=0){
                    // stop timer A
                    ciaChip[CIACRA]=aux1&(0xff-STARTT);
                    TA.PHI2=false;             // kill PHI2
                    TA.CNT=false;              // kill CNT
                }
            
                ciaChip[CIAICR]|=TAINT;    //set interupt for timer A
                if((CIAMASK&TAINT)!=0){
                    ciaChip[CIAICR]|=INTER; //flag an interupt
                    if(id){
                        Bus.setNMI();      //signal NMI
                    }
                    else{
                        Bus.setIRQ(CIAAIRQ);      //signal IRQ
                    }
                }
            
                if(TB.CTA){                //Timer B count Timer A
                   TB.count--;
                   if((TB.count&0xffff0000)!=0){
                        aux1=ciaChip[CIACRB];
                        sortTB();
                   }
                }
            }
        }
            
        // Work Timer B now
        if(TB.PHI2){
            aux1=ciaChip[CIACRB];
            if((aux1&STARTT)!=0){
                TB.count-=cycles;
                if((TB.count&0xffff0000)!=0){
                    sortTB();
                }
            }
        }            
    };
    
    public void execute(){
        timerAB();
        TOD.execute();
        if(TOD.readAlarm()){
            ciaChip[CIAICR]|=TODINT;
            if((CIAMASK&TODINT)!=0){
                ciaChip[CIAICR]|=INTER; //flag an interupt
                if(id){
                    Bus.setNMI();      //signal NMI
                }
                else{
                    Bus.setIRQ(CIAAIRQ);      //signal IRQ
                }
            }           
        }
    };
};

class CIATIMER{
    int count;
    int latch;
    boolean CNT,PHI2,CTA,Pulse;
    CIATIMER()
    {
        count=0;
        latch=0;
        reset();
    };
    
    public void reset(){
        Pulse=false;
        CNT=CTA=false;
        PHI2=true;
        count=latch=0xffff;
    };
};

class TODCLOCK{
    int[] count;                //actual time
    int[] latch;                //latch
    int   period;               //period for cycles
    int   cycles;               //number of cycles executed
    boolean  start,update,show; // flags
    boolean alarm;
    int aux1,aux2,aux3,aux4;
    
    public TODCLOCK(){
        count=new int[4];
        latch=new int[4];
        period=0;
        cycles=0;        
        reset();
    };
    
    public void reset(){
        int i;
        start=update=show=false;
        alarm=false;
        for(i=0;i<4;i++){
            count[i]=latch[i]=0;
        }        
    };
    
    public boolean readAlarm(){
        if(alarm){
            alarm=false;
            return true;
        }
        return false;
    }
    
    void execute(){
        if(start){
            if(period<cycles){
                count[0]++;
                if(count[0]>9){
                    count[0]=0;
                    aux3=count[1]&0x0f;
                    aux4=count[1]&0xf0;
                    aux3++;
                    if(aux3>9){
                        aux4+=0x10;
                        aux3=0x00;
                    }
                    count[1]=aux3|aux4;
                    if(count[1]>=0x60){
                        count[1]=0;
                        aux3=count[2]&0x0f;
                        aux4=count[2]&0xf0;
                        aux3++;
                        if(aux3>9){
                            aux4+=0x10;
                            aux3=0x00;
                        }
                        count[2]=aux3|aux4;
                        if(count[2]>=0x60){
                            count[2]=0;
                            aux2=count[3]&0x80;
                            aux3=count[3]&0x0f;
                            aux4=count[3]&0x70;
                            aux3++;
                            if(aux3>9){
                                aux4+=0x10;
                                aux3=0x00;
                            }
                            count[3]=aux3|aux4;
                            if(count[3]>0x12){
                                aux2=(aux2^0x80)&0x80;
                                count[3]=0x01;
                            }
                            count[3]|=aux2;
                        }
                    }
                }
                aux1=count[3]<<24;
                aux1|=count[2]<<16;
                aux1|=count[1]<<8;
                aux1|=count[0];
                
                aux2=count[3]<<24;
                aux2|=count[2]<<16;
                aux2|=count[1]<<8;
                aux2|=count[0];                
                if(aux1==aux2){
                    alarm=true;
                }
                else{
                    alarm=false;
                }
            }
            else{
                cycles+=63;
            };
        };
    };    
};