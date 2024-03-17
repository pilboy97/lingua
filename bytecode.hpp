#ifndef __BYTE_CODE_HPP__
#define __BYTE_CODE_HPP__

enum ByteCode {
    _NULL, _ADD, _SUB, _MUL, _DIV, _MOD,
    _ADDF, _SUBF, _MULF, _DIVF,
    _AND, _OR, _XOR, _LSH, _RSH, _NOT,
    _LAND, _LOR, _LNOT,
    _PUSH, _POP, _CP, 
    _ASS, _ACC, _ADDR, _ALLOC, _FREE,
    _DATA, _IMD, _ENTER, _LEAVE,
    _JMP, _IF, _CALL, _RET, _END, _CLR
};

struct BCode {
    int opcode;
    int a;
    int b;
    int c;
};

BCode bc(int opcode, int a, int b, int c) {
    return (BCode){.opcode=opcode,.a=a,.b=b,.c=c};
}

#endif