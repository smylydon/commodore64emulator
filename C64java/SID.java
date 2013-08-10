/*
 * SID.java
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
public class SID {
    int cycles;
    /** Creates a new instance of SID */
    public SID() {
    };
    
     public void inialise(){
        
    };
    
    public void setCycles(int newCycles){
        cycles=newCycles;
    };
    
    public int readSID(int address){
        address&=0xf;
        return 0;
    };
    public void writeSID(int data,int address){
        
    };    
    public void hardReset(){
        
    };    
}
