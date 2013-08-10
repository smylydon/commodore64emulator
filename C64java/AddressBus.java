/*
 * AddressBus.java
 *
 * Created on February 2, 2006, 4:29 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

/**
 *
 * @author Owner
 */
public class AddressBus {
    private static final int LORAM=0x01;
    private static final int HIRAM=0x02;
    private static final int CHAREN=0x04;
    private static final int CPUCYCLES=65;
    private static final int CIACYCLES=65;
    private static final int VICLINES=263;
    private static final int PERIOD=1022727;
        
    private CIA M6526A;
    private CIA M6526B;
    private CPU M6510;
    private SID M6581;
    private VicII M6569;
    private Basic basic;
    private Charset charset;
    private Kernal kernal;
        
    private int[] memory,colorRam;
    private int reader,sp;
    private int busword,aux1,aux2,aux3,NMI,IRQ;
    private boolean ReadWriteModify;
    
    /** Creates a new instance of AddressBus */
    public AddressBus() {
        memory=new int[0x10000];
        colorRam=new int[0x400];
        basic = new Basic();
        charset= new Charset();
        kernal= new Kernal();
    };
    
    public void setupBus(CIA a, CIA b, CPU c, SID s, VicII v){
        M6526A=a;
        M6526B=b;
        M6581=s;
        M6569=v;
        M6510=c;
    };
    
    public void resetBus(){
        reader=0xef;
        sp=0x100;
        memory[0x00]=reader;
        memory[0x01]=reader;    

        NMI=IRQ=0; 
	M6510.hardReset();
	M6569.hardReset();
	M6526A.hardReset();
        M6526B.hardReset();
	M6581.hardReset();    
        M6526A.setCycles(CIACYCLES);
        M6526B.setCycles(CIACYCLES);        
        M6569.setCycles(VICLINES);
        M6510.setCycles(CPUCYCLES);      
    };
    
    void CPUnoBadline(){
        M6510.setCycles(CPUCYCLES); 
    };

    void CPUyesBadline(){
        M6510.setCycles(23); 
    };
    
    public boolean readIRQ(){
        if(IRQ!=0){
            return true;
        }
        return false;
    };

    public void clearIRQ(int MIRQ){
	IRQ&=(0xff-MIRQ);
    };

    public void setIRQ(int MIRQ){
        IRQ|=MIRQ;
    };

    public boolean readNMI(){
        if(NMI!=0){
            return true;
        }
        return false;
    };

    public void setNMI(){
	NMI=0xff;
    };

    public void clearNMI(){
	NMI=0;;
    };   

    public int readCiaB(){
        return (M6526B.readCIA(0)&0xff);
    };

    void upDateVicII(){
        M6569.upDateVicII();
    };

    public int readCPUCycles(){
        return (CPUCYCLES);
    };

    public void setCPUCycles(int cycles){
        M6510.setCycles(cycles);
    };

private int readCustomChips(int address) {
    aux1=address&0xff00;
    aux2=address&0x00ff;
    switch(aux1) {
        case 0xd000:
        case 0xd100:
        case 0xd200:
        case 0xd300:
        return M6569.readVIC(aux2);

        case 0xd400:
        case 0xd500:
        case 0xd600:
        case 0xd700:
        return M6581.readSID(aux2);

        case 0xd800:
        case 0xd900:
        case 0xda00:
        case 0xdb00:
        address&=0x3ff;
        if(address>0x3e7){
            return 0xff;
        }
        return  readColorRam(address);

        case 0xdc00:
        return M6526A.readCIA(aux2);

        case 0xdd00:
        return M6526B.readCIA(aux2);

        case 0xde00:
        case 0xdf00:
        default:
        return 0xff;

    }
};

public int readByte(int address) {
    address&=0xffff;
    reader=memory[1]&0x03;

    if (reader==0)
        return (memory[address]&0xff);
    if (address<0xa000)
        return (memory[address]&0xff);

    if(address>=0xe000) {
        if((reader&HIRAM)!=0) // Kernal turned on?
        {
            return(kernal.read(address));
        }
        return (memory[address]&0xff);
    }

    if (address>=0xc000) {
        if(address<0xd000) {
            return (memory[address]&0xff);
        }
        else {
            reader=memory[1]&7;
            if(reader<4) {

                return charset.read(address-0xd000);
            }
            else {
                return(readCustomChips(address));  //custom chips
            }
        }

    }

    if(reader==0x3)//Basic & Kernal bits must be on for Basic
    {
        return basic.read(address);
    }
    return (memory[address]&0xff);
};

int readWord(int address) {
    busword=0;
    address&=0xffff;
    busword=readByte(address);
    address++;
    busword|=(readByte(address)<<8);
    return (busword&0xffff);
};

int readRam(int address) {
    return(memory[address&0xffff]&0xff);
};

int readBounds(int address) {
    aux3=address&0xff00;
    aux2=address&0x00ff;
    busword=readByte(aux3|aux2);
    aux2=(aux2+1)&0xff;
    busword=readByte(aux3|aux2);
    return busword&0xffff;
};

private void writeCustomChip(int data,int address) {
    aux1=address&0xff00;
    aux2=address&0x00ff;
    data&=0xff;
    switch(aux1) {
        case 0xd000:
        case 0xd100:
        case 0xd200:
        case 0xd300:
        M6569.writeVIC(data,aux2);
        break;
        
        case 0xd400:
        case 0xd500:
        case 0xd600:
        case 0xd700:
        M6581.writeSID(data,aux2);
        break;
        
        case 0xd800:
        case 0xd900:
        case 0xda00:
        case 0xdb00:
        address&=0xfff;
        colorRam[address]=data|0xf0;
        break;
        
        case 0xdc00:
        M6526A.writeCIA(data,aux2);
        break;
        
        case 0xdd00:
        M6526B.writeCIA(data,aux2);
        break;
        
        case 0xde00:
        case 0xdf00:
        default:
        break;
    }
};

public void writeByte(int data,int address) {
    address&=0xffff;
    data&=0xff;
    reader=memory[0]&0x03;

    if(reader==0) {
        memory[address]=data;
        return;
    }

    if(address<0xd000) {
        memory[address]=data;
        return;
    }

    if(address<0xe000) {
        if((reader&CHAREN)!=0) {
            writeCustomChip(data,address);
            return;
        }
    }
    memory[address]=data;
};

public void writeRam(int data,int address) {
    memory[address&0xffff]=(data&0xff);
};

//Stack routines

public int popByte(int address) {
    sp=(address&0xff)|0x100;
    return (memory[sp]&0xff);
};

public int popWord(int address ) {
    busword=0;
    sp=(address&0xff)|0x100;
    busword=memory[sp];
    sp=((sp+1)&0xff)|0x100;
    busword|=((memory[sp]&0xff)<<8);
    return (busword&0xffff);
};

public void pushByte(int data,int address) {
    sp=(address&0xff)|0x100;
    memory[sp]=data;
};

public void pushWord(int data,int address) {
    sp=(address&0xff)|0x100;
    busword=data;
    memory[sp]=busword>>8;
    sp=((sp-1)&0xff)|0x100;
    memory[sp]=(busword&0xff);
};

public int readChar(int data) {
    return charset.read(data);
};

public int readColorRam(int address) {
    address&=0x3ff;
    return ((colorRam[address]|0xf0)&0xff);
};
}




