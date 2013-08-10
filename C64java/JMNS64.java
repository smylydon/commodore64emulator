/*
 * JMNS64.java
 *
 * Created on February 2, 2006, 4:00 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

/**
 *
 * @author Owner
 */
import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import javax.swing.*;

public class JMNS64 extends Canvas implements KeyListener{
    public static final int WIDTH = 640;
    public static final int HEIGHT = 400;
    public Graphics Backgnd;
    public Image buffer;
    int myKeys;
      
    /** Creates a new instance of JMNS64 */
    public JMNS64() {
        JFrame myEmu= new JFrame("JMNS64 Emulator 1.0");
        JPanel myPanel = (JPanel)myEmu.getContentPane();
        setBounds(0,0,WIDTH,HEIGHT);
        myPanel.setPreferredSize(new Dimension(WIDTH,HEIGHT));
        myPanel.setLayout(null);
        myPanel.add(this);
        myEmu.setBounds(0,0,WIDTH,HEIGHT);
        myEmu.setVisible(true);
        myEmu.addWindowListener( new WindowAdapter() {
            public void windowClosing(WindowEvent e){
                System.exit(0);
            }        
        });
        
        myKeys=0;
        myEmu.addKeyListener(this);
    }
    
    public void keyReleased(KeyEvent e){
        myKeys=0;
    }

    public void keyPressed(KeyEvent e){
        switch (e.getKeyCode()){
            case KeyEvent.VK_DOWN :
                myKeys=2;
                break;
            case KeyEvent.VK_UP :
                myKeys=1;
                break;
        }        
    }
    
    public void keyTyped(KeyEvent e) {};
    
    public void DoubleBuffer(int width,int height) {
        buffer=createImage(width,height);
        Backgnd=buffer.getGraphics();
        Backgnd.setColor(Color.black);
        Backgnd.fillRect(0,0,width,height); 
    }
    
    public void myEmu(){
        DoubleBuffer(WIDTH,HEIGHT);
        AddressBus Bus=new AddressBus();
        CIA ciaChipA=new CIA(false,Bus);
        CIA ciaChipB=new CIA(true,Bus);
        CPU cpuChip=new CPU(Bus);
        SID sidChip=new SID();
        VicII vicChip=new VicII(Bus,Backgnd); 
        DeBug Disa=new DeBug(cpuChip,Backgnd,Bus);
        Bus.setupBus(ciaChipA,ciaChipB,cpuChip,sidChip,vicChip);
        Bus.resetBus();
        while (isVisible()){
            cpuChip.execute();
            ciaChipA.execute();
            ciaChipB.execute();
            vicChip.execute();
            Disa.dumpCpu();
            if(vicChip.isdone){
                paint(getGraphics());
                try{
                    Thread.sleep(2);
                }
                catch (InterruptedException e) {} 
           }
        }      
    };
    
    public void paint(Graphics g){
        Graphics p=Backgnd;
        p.setColor(Color.white);
        g.drawImage(buffer,0,0,this);
        p.setColor(Color.black);
    }
     
    public static void main(String[] args) {
        // TODO code application logic here
       JMNS64 EMU=new JMNS64();
       EMU.myEmu();    
    }
}
