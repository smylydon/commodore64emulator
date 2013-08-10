/*
 * DeBug.java
 *
 * Created on February 7, 2006, 11:11 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

/**
 *
 * @author Owner
 */
import java.awt.*;

public class DeBug {
    private static final int Nflag=0x80;
    private static final int Vflag=0x40;
    private static final int Uflag=0x20;
    private static final int Bflag=0x10;
    private static final int Dflag=0x08;
    private static final int Iflag=0x04;
    private static final int Zflag=0x02;
    private static final int Cflag=0x01;    
    private Graphics g;
    private AddressBus bus;
    private CPU cpu;
    private int cursx,cursy;
    /** Creates a new instance of DeBug */
    public DeBug(CPU aCpu,Graphics gfx,AddressBus aBus) {
        g=gfx;
        cpu=aCpu;
        bus=aBus;
        cursx=400;
        cursy=20;
    };
    
    private void doFlags(int flag,int x){
        if((cpu.flags()&flag)!=0){
            g.drawString("1",x,cursy);
        }
        else{
            g.drawString("0",x,cursy);
        }
    };
    
    public void dumpCpu(){
        cursy=20;
        g.setColor(Color.black);
        g.fillRect(cursx,cursy,100,200);
        g.setColor(Color.white);
        g.drawString("M6510 Status \n\n",cursx,cursy);
        cursy+=15;
        g.drawString("N V - B D I Z C\n",cursx,cursy);
        cursy+=15;
        doFlags(Nflag,cursx);
        doFlags(Vflag,cursx+10);
        doFlags(Uflag,cursx+20);
        doFlags(Bflag,cursx+30);
        doFlags(Dflag,cursx+40);
        doFlags(Iflag,cursx+50);
        doFlags(Zflag,cursx+60);
        doFlags(Cflag,cursx+70);
        cursy+=15;
        g.drawString("PC1   :",cursx,cursy);
        g.drawString(cpu.readPC1(),cursx+40,cursy);
        cursy+=15;
        g.drawString("PC2   :",cursx,cursy); 
        g.drawString(cpu.readPC2(),cursx+40,cursy);        
        cursy+=15;
        g.drawString("SP    :",cursx,cursy);  
        g.drawString(cpu.readSPreg(),cursx+40,cursy);       
        cursy+=15;
        g.drawString("AR    :",cursx,cursy);
        g.drawString(cpu.readAreg(),cursx+40,cursy);        
        cursy+=15;
        g.drawString("XR    :",cursx,cursy);
        g.drawString(cpu.readXreg(),cursx+40,cursy);       
        cursy+=15;
        g.drawString("YR    :",cursx,cursy);   
        g.drawString(cpu.readYreg(),cursx+40,cursy);        
        cursy+=15;
        g.drawString("0316  :",cursx,cursy);  
        g.drawString(Integer.toString(bus.readByte(0x316)|
                (bus.readByte(0x317)<<8),16),cursx+40,cursy);    
    };
}
