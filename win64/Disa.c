#include <SDL.h>
#include "AddressBus.h"
#include "CBM64.h"

enum {
    A_IMPL,
    A_ACCU, // A
    A_IMM, // #zz
    A_REL, // Branches
    A_ZERO, // zz
    A_ZEROX, // zz,x
    A_ZEROY, // zz,y
    A_ABS, // zzzz
    A_ABSX, // zzzz,x
    A_ABSY, // zzzz,y
    A_IND, // (zzzz)
    A_INDX, // (zz,x)
    A_INDY // (zz),y
};

enum {
    M_ADC=0, M_AND, M_ASL, M_BCC, M_BCS, M_BEQ, M_BIT, M_BMI, M_BNE, M_BPL,
    M_BRK, M_BVC, M_BVS, M_CLC, M_CLD, M_CLI, M_CLV, M_CMP, M_CPX, M_CPY,
    M_DEC, M_DEX, M_DEY, M_EOR, M_INC, M_INX, M_INY, M_JMP, M_JSR, M_LDA,
    M_LDX, M_LDY, M_LSR, M_NOP, M_ORA, M_PHA, M_PHP, M_PLA, M_PLP, M_ROL,
    M_ROR, M_RTI, M_RTS, M_SBC, M_SEC, M_SED, M_SEI, M_STA, M_STX, M_STY,
    M_TAX, M_TAY, M_TSX, M_TXA, M_TXS, M_TYA,

    //undocumented opcodes

    M_IANC, M_IANE, M_IARR, M_IASR, M_IDCP, M_IISB, M_IJAM, M_INOP, M_ILAS,
    M_ILAX, M_ILXA, M_IRLA, M_IRRA, M_ISAX, M_ISBC, M_ISBX, M_ISHA, M_ISHS,
    M_ISHX, M_ISHY, M_ISLO, M_ISRE
};

char *mn[]=
    {
        " adc "," and "," asl "," bcc "," bcs "," beq "," bit "," bmi "," bne "," bpl ",
        " brk "," bvc "," bvs "," clc "," cld "," cli "," clv "," cmp "," cpx "," cpy ",
        " dec "," dex "," dey "," eor "," inc "," inx "," iny "," jmp "," jsr "," lda ",
        " ldx "," ldy "," lsr "," nop "," ora "," pha "," php "," pla "," plp "," rol ",
        " ror "," rti "," rts "," sbc "," sec "," sed "," sei "," sta "," stx "," sty ",
        " tax "," tay "," tsx "," txa "," txs "," tya ",
        //undocumented mnemonics
        "*anc ","*ane ","*arr ","*asr ","*dcp ","*isb ","*jam ","*nop ","*las ","*lax ",
        "*lxa ","*rla ","*rra ","*sax ","*sbc ","*sbx ","*sha ","*shs ","*shx ","*shy ",
        "*slo ","*sre "
    };

//lookup
char decode[256] = {
                       M_BRK , M_ORA , M_IJAM, M_ISLO, M_INOP, M_ORA, M_ASL , M_ISLO, // 00
                       M_PHP , M_ORA , M_ASL , M_IANC, M_INOP, M_ORA, M_ASL , M_ISLO,
                       M_BPL , M_ORA , M_IJAM, M_ISLO, M_INOP, M_ORA, M_ASL , M_ISLO, // 10
                       M_CLC , M_ORA , M_INOP, M_ISLO, M_INOP, M_ORA, M_ASL , M_ISLO,
                       M_JSR , M_AND , M_IJAM, M_IRLA, M_BIT , M_AND, M_ROL , M_IRLA, // 20
                       M_PLP , M_AND , M_ROL , M_IANC, M_BIT , M_AND, M_ROL , M_IRLA,
                       M_BMI , M_AND , M_IJAM, M_IRLA, M_INOP, M_AND, M_ROL , M_IRLA, // 30
                       M_SEC , M_AND , M_INOP, M_IRLA, M_INOP, M_AND, M_ROL , M_IRLA,
                       M_RTI , M_EOR , M_IJAM, M_ISRE, M_INOP, M_EOR, M_LSR , M_ISRE, // 40
                       M_PHA , M_EOR , M_LSR , M_IASR, M_JMP , M_EOR, M_LSR , M_ISRE,
                       M_BVC , M_EOR , M_IJAM, M_ISRE, M_INOP, M_EOR, M_LSR , M_ISRE, // 50
                       M_CLI , M_EOR , M_INOP, M_ISRE, M_INOP, M_EOR, M_LSR , M_ISRE,
                       M_RTS , M_ADC , M_IJAM, M_IRRA, M_INOP, M_ADC, M_ROR , M_IRRA, // 60
                       M_PLA , M_ADC , M_ROR , M_IARR, M_JMP , M_ADC, M_ROR , M_IRRA,
                       M_BVS , M_ADC , M_IJAM, M_IRRA, M_INOP, M_ADC, M_ROR , M_IRRA, // 70
                       M_SEI , M_ADC , M_INOP, M_IRRA, M_INOP, M_ADC, M_ROR , M_IRRA,
                       M_INOP, M_STA , M_INOP, M_ISAX, M_STY , M_STA, M_STX , M_ISAX, // 80
                       M_DEY , M_INOP, M_TXA , M_IANE, M_STY , M_STA, M_STX , M_ISAX,
                       M_BCC , M_STA , M_IJAM, M_ISHA, M_STY , M_STA, M_STX , M_ISAX, // 90
                       M_TYA , M_STA , M_TXS , M_ISHS, M_ISHY, M_STA, M_ISHX, M_ISHA,
                       M_LDY , M_LDA , M_LDX , M_ILAX, M_LDY , M_LDA, M_LDX , M_ILAX, // a0
                       M_TAY , M_LDA , M_TAX , M_ILXA, M_LDY , M_LDA, M_LDX , M_ILAX,
                       M_BCS , M_LDA , M_IJAM, M_ILAX, M_LDY , M_LDA, M_LDX , M_ILAX, // b0
                       M_CLV , M_LDA , M_TSX , M_ILAS, M_LDY , M_LDA, M_LDX , M_ILAX,
                       M_CPY , M_CMP , M_INOP, M_IDCP, M_CPY , M_CMP, M_DEC , M_IDCP, // c0
                       M_INY , M_CMP , M_DEX , M_ISBX, M_CPY , M_CMP, M_DEC , M_IDCP,
                       M_BNE , M_CMP , M_IJAM, M_IDCP, M_INOP, M_CMP, M_DEC , M_IDCP, // d0
                       M_CLD , M_CMP , M_INOP, M_IDCP, M_INOP, M_CMP, M_DEC , M_IDCP,
                       M_CPX , M_SBC , M_INOP, M_IISB, M_CPX , M_SBC, M_INC , M_IISB, // e0
                       M_INX , M_SBC , M_NOP , M_ISBC, M_CPX , M_SBC, M_INC , M_IISB,
                       M_BEQ , M_SBC , M_IJAM, M_IISB, M_INOP, M_SBC, M_INC , M_IISB, // f0
                       M_SED , M_SBC , M_INOP, M_IISB, M_INOP, M_SBC, M_INC , M_IISB
                   };

const char adrmode[256] = {
                              A_IMPL, A_INDX, A_IMPL, A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO, // 00
                              A_IMPL, A_IMM , A_ACCU, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
                              A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX, // 10
                              A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
                              A_ABS , A_INDX, A_IMPL, A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO, // 20
                              A_IMPL, A_IMM , A_ACCU, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
                              A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX, // 30
                              A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
                              A_IMPL, A_INDX, A_IMPL, A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO, // 40
                              A_IMPL, A_IMM , A_ACCU, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
                              A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX, // 50
                              A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
                              A_IMPL, A_INDX, A_IMPL, A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO, // 60
                              A_IMPL, A_IMM , A_ACCU, A_IMM , A_IND  , A_ABS  , A_ABS  , A_ABS,
                              A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX, // 70
                              A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
                              A_IMM , A_INDX, A_IMM , A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO, // 80
                              A_IMPL, A_IMM , A_IMPL, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
                              A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROY, A_ZEROY, // 90
                              A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSY , A_ABSY,
                              A_IMM , A_INDX, A_IMM , A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO, // a0
                              A_IMPL, A_IMM , A_IMPL, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
                              A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROY, A_ZEROY, // b0
                              A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSY , A_ABSY,
                              A_IMM , A_INDX, A_IMM , A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO, // c0
                              A_IMPL, A_IMM , A_IMPL, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
                              A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX, // d0
                              A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX,
                              A_IMM , A_INDX, A_IMM , A_INDX, A_ZERO , A_ZERO , A_ZERO , A_ZERO, // e0
                              A_IMPL, A_IMM , A_IMPL, A_IMM , A_ABS  , A_ABS  , A_ABS  , A_ABS,
                              A_REL , A_INDY, A_IMPL, A_INDY, A_ZEROX, A_ZEROX, A_ZEROX, A_ZEROX, // f0
                              A_IMPL, A_ABSY, A_IMPL, A_ABSY, A_ABSX , A_ABSX , A_ABSX , A_ABSX
                          };
void getCharacter(){
    SDL_Event event;

    int msg,GUIKey;
    int done=0xff;
    int cx=0,cy=0;
    while(done) {
        msg=GUIKey=0;
        if(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                switch(event.key.keysym.sym ) {
                    case SDLK_DOWN:
                    break;
                    case SDLK_UP:
                    break;
                    case SDLK_RETURN:
                    msg=0xf00f;
                    done=0;
                    break;
                }// end inner switch
                break;
                case SDL_KEYUP:
                // If escape is pressed, return (and thus, quit)
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    msg=0xdead;
                    done=0;
                }
                break;
                case SDL_QUIT:
                msg=0xdead;
                done=0;
                break;
            }// end outter switch
        }// end if
    }// end while
}

void Assem(){

}

void Disa(unsigned int count,int pc) {
    int temp,mem;
    int opnd1,opnd2,op;
    unsigned int c=0;
    while((c<count)&&(c<=15)) {
        temp=0;
        mem=pc&0xffff;
        op=BusReadByte(mem)&0xff;
        opnd1=BusReadByte(mem+1)&0xff;
        opnd2=BusReadByte(mem+2)&0xff;
        aStringToBuffer("\n");
        anIntToBuffer(mem,2,1);
        switch(adrmode[op]) {
            case A_ACCU:
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("          ");
            aStringToBuffer(mn[decode[op]]);
            pc++;
            break;
            case A_IMPL:
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("          ");
            aStringToBuffer(mn[decode[op]]);
            pc++;
            break;
            case A_REL:
            temp=opnd1&0xff;
            if(opnd1>0x80) {
                temp=opnd1|0xff00;
            }
            temp=(temp+mem+2)&0xffff;
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("      ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  $");
            anIntToBuffer(temp,2,1);
            pc+=2;
            break;
            case A_IMM:
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("      ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  #$");
            anIntToBuffer(opnd1,1,1);
            pc+=2;
            break;
            case A_ZERO:
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("      ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  $");
            anIntToBuffer(opnd1,1,1);
            pc+=2;
            break;
            case A_ZEROX:
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("      ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  $");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer(",x");
            pc+=2;
            break;
            case A_ZEROY:
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("      ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  $");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer(",y");
            pc+=2;
            break;
            case A_INDX:
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("      ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  ($");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer(",x)");
            pc+=2;
            break;
            case A_INDY:
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("      ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  ($");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("),y");
            pc+=2;
            break;
            case A_ABS:
            temp=(opnd1+(opnd2<<8))&0xffff;
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd2,1,1);
            aStringToBuffer("  ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  $");
            anIntToBuffer(temp,2,1);
            pc+=3;
            break;
            case A_ABSX:
            temp=(opnd1+(opnd2<<8))&0xffff;
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd2,1,1);
            aStringToBuffer("  ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  $");
            anIntToBuffer(temp,2,1);
            aStringToBuffer(",x  ");
            pc+=3;
            break;
            case A_ABSY:
            temp=(opnd1+(opnd2<<8))&0xffff;
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd2,1,1);
            aStringToBuffer("  ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  $");
            anIntToBuffer(temp,2,1);
            aStringToBuffer(",y");
            pc+=3;
            break;
            case A_IND:
            temp=(opnd1+(opnd2<<8))&0xffff;
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd2,1,1);
            aStringToBuffer("  ");
            aStringToBuffer(mn[decode[op]]);
            aStringToBuffer("  ($");
            anIntToBuffer(temp,2,1);
            aStringToBuffer(")");
            pc+=3;
            break;
            default:
            aStringToBuffer("  ");
            anIntToBuffer(op,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            aStringToBuffer("  ");
            anIntToBuffer(opnd1,1,1);
            pc++;
            break;
        }
        c++;
    }
    aStringToBuffer("\n");
}
