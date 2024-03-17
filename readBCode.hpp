#ifndef __READBCODE_HPP__
#define __READBCODE_HPP__

#include "bytecode.hpp"
#include <string.h>
#include <vector>
#include <stdio.h>

int strToB(const char * word) {
    if(strcmp(word, "add") == 0) {
        return _ADD;
    }
    if(strcmp(word, "sub") == 0) {
        return _SUB;
    }
    if(strcmp(word, "mul") == 0) {
        return _MUL;
    }
    if(strcmp(word, "div") == 0) {
        return _DIV;
    }
    if(strcmp(word, "mod") == 0) {
        return _MOD;
    }
    if(strcmp(word, "addf") == 0) {
        return _ADDR;
    }
    if(strcmp(word, "subf") == 0) {
        return _SUBF;
    }
    if(strcmp(word, "mulf") == 0) {
        return _MULF;
    }
    if(strcmp(word, "divf") == 0) {
        return _DIVF;
    }
    if(strcmp(word, "and") == 0) {
        return _AND;
    }
    if(strcmp(word, "or") == 0) {
        return _OR;
    }
    if(strcmp(word, "xor") == 0) {
        return _XOR;
    }
    if(strcmp(word, "lsh") == 0) {
        return _LSH;
    }
    if(strcmp(word, "rsh") == 0) {
        return _RSH;
    }
    if(strcmp(word, "not") == 0) {
        return _NOT;
    }
    if(strcmp(word, "land") == 0) {
        return _LAND;
    }
    if(strcmp(word, "lor") == 0) {
        return _LOR;
    }
    if(strcmp(word, "lnot") == 0) {
        return _LNOT;
    }
    if(strcmp(word, "push") == 0) {
        return _PUSH;
    }
    if(strcmp(word, "pop") == 0) {
        return _POP;
    }
    if(strcmp(word, "cp") == 0) {
        return _CP;
    }
    if(strcmp(word, "ass") == 0) {
        return _ASS;
    }
    if(strcmp(word, "acc") == 0) {
        return _ACC;
    }
    if(strcmp(word, "addr") == 0) {
        return _ADDR;
    }
    if(strcmp(word, "alloc") == 0) {
        return _ALLOC;
    }
    if(strcmp(word, "free") == 0) {
        return _FREE;
    }
    if(strcmp(word, "data") == 0) {
        return _DATA;
    }
    if(strcmp(word, "imd") == 0) {
        return _IMD;
    }
    if(strcmp(word, "enter") == 0) {
        return _ENTER;
    }
    if(strcmp(word, "leave") == 0) {
        return _LEAVE;
    }
    if(strcmp(word, "jmp") == 0) {
        return _JMP;
    }
    if(strcmp(word, "if") == 0) {
        return _IF;
    }
    if(strcmp(word, "call") == 0) {
        return _CALL;
    }
    if(strcmp(word, "return") == 0) {
        return _RET;
    }
    if(strcmp(word, "end") == 0) {
        return _END;
    }
    if(strcmp(word, "clr") == 0) {
        return _CLR;
    }
    return 0;
}
const char *bToStr(int x) {
    switch (x) {
    case _NULL:
        return "";
    case _ADD:
        return "add";
    case _SUB:
        return "sub";
    case _MUL:
        return "mul";
    case _DIV: 
        return "div";
    case _MOD:
        return "mod";
    case _ADDF:
        return "addf"; 
    case _SUBF: 
        return "subf";
    case _MULF: 
        return "mulf";
    case _DIVF: 
        return "divf";
    case _AND: 
        return "and";
    case _OR: 
        return "or";
    case _XOR: 
        return "xor";
    case _LSH: 
        return "lsh";
    case _RSH: 
        return "rsh";
    case _NOT:
        return "not";
    case _LAND: 
        return "land";
    case _LOR: 
        return "lor";
    case _LNOT:
        return "lnot";
    case _PUSH: 
        return "push";
    case _POP: 
        return "pop";
    case _CP: 
        return "cp";
    case _ASS: 
        return "ass";
    case _ACC: 
        return "acc";
    case _ADDR: 
        return "addr";
    case _ALLOC: 
        return "alloc";
    case _FREE:
        return "free";
    case _DATA: 
        return "data";
    case _IMD: 
        return "imd";
    case _ENTER: 
        return "enter";
    case _LEAVE:
        return "leave";
    case _JMP: 
        return "jmp";
    case _IF: 
        return "if";
    case _CALL: 
        return "call";
    case _RET: 
        return "return";
    case _END:
        return "end"; 
    case _CLR:
        return "clr";
    }
    return "";
}
std::vector<BCode> readBFile(const char *code) {
    std::vector<BCode> ret;

    int len = strlen(code);
    int begin = 0;
    bool done = false;
    int c[4]={0,};
    int cnt = 0;
    int i = 0;
    
    while(i < len) {
        if(code[i] == '#') {
            done = true;
            i++;
        }
        else if(code[i] == '\n') {
            ret.push_back(bc(c[0], c[1], c[2], c[3]));

            done = false;
            c[0] = 0;
            c[1] = 0;
            c[2] = 0;
            c[3] = 0;
            cnt = 0;

            i++;
        }

        if(done) {
            i++;
            continue;
        }

        if(code[i] >= '0' && code[i] <= '9') {
            int x = 0;
            while(i < len && code[i] >= '0' && code[i] <= '9') {
                x *= 10;
                x += code[i] - '0';
                i++;
            }
            c[cnt++] = x;
        }
        else if(code[i] >= 'a' && code[i] <= 'z') {
            int begin = i;
            int end;
            while(i < len && code[i] >= 'a' && code[i] <= 'z') {
                i++;
            }
            end = i;

            char *str = (char *) malloc(sizeof(char) * (end - begin));
            for(int j = 0;j < end-begin;j++) {
                str[j] = code[j + begin];
            }
            c[cnt++] = strToB(str);
            free((void*)str);
        }
        else i++;
    }

    ret.push_back(bc(c[0], c[1], c[2], c[3]));
    return ret;
}
void printBC(std::vector<BCode> bc) {
    for(int i = 0;i < bc.size();i++) {
        printf("%s %s %d %s\n", bToStr(bc[i].opcode),bToStr(bc[i].a),bc[i].b,bToStr(bc[i].c));
    }
}

#endif