#ifndef __RUN_HPP__
#define __RUN_HPP__

#include "bytecode.hpp"
#include "panic.hpp"
#include <vector>
#include <stack>
#include <algorithm>

static int pc = 0;
static bool alive = false;
static int exitcode = 0;
static std::vector<std::pair<int, int> > HEAP;
static std::vector<int> STACK;
static int SP = 1;
static int ESP = 1;
static std::stack<int> SAVE;
static std::stack<std::stack<int> > CSTACK;

void enter() {
    SAVE.push(ESP);
}
void leave() {
    ESP = SAVE.top();
    SAVE.pop();
}
int store(int x) {
    int ret = ESP;
    if(ESP + 1 >= STACK.size()) {
        STACK.push_back(0);
    }

    STACK[ESP] = x;
    ESP++;

    return -ret;
}
int &access(int ptr) {
    if(ptr > 0) {
        if(HEAP[ptr].first <= 0) panicf("invalid memory: %d", ptr);
        if(ptr >= HEAP.size()) panicf("invalid memory: %d", ptr);
        return HEAP[ptr].second;
    }
    if(ptr == 0) {
        panic("null pointer access");
    }

    ptr = -ptr;
    if(ptr >= STACK.size()) panicf("invalid memory: %d", -ptr);
    return STACK[SP + ptr];
}
int get() {
    int ret = access(CSTACK.top().top());
    CSTACK.top().pop();
    return ret;
}
double getDouble() {
    int ptr = CSTACK.top().top();
    CSTACK.pop();

    long long bytes = access(ptr);
    long long rest;
    if(ptr > 0) {
        rest = access(ptr + 1);
    }
    else {
        rest = access(ptr - 1);
    }

    long long tmp = bytes << 32 | rest;
    void *vptr = &tmp;
    double ret = *(double*) vptr;

    return ret;
}
long long getLL() {
    int ret = CSTACK.top().top();
    CSTACK.pop();

    long long bytes = access(ret);
    long long rest;
    if(ret > 0) {
        rest = access(ret + 1);
    }
    else {
        rest = access(ret - 1);
    }

    ret = bytes << 32 | rest;

    return ret;
}
void cpush(int x) {
    CSTACK.top().push(store(x));
}
void add(BCode code) {
    // add <s1> <s2> : s1 + s2
    // add imd <c1> <s1> : s1 + c1

    if(code.opcode != _ADD) panic("wrong add inst");

    int s1 = get();
    if(code.a == _IMD) {
        int c1 = code.b;
        
        cpush(access(s1) + c1);
    }
    else {
        int s2 = get();
        printf("%d %d\n", s2, s1);
        cpush(access(s2) + access(s1));
    }
}
void sub(BCode code) {
    // sub <s1> <s2> : s1 - s2
    // sub imd <c1> <s1> : s1 - c1

    if(code.opcode != _SUB) panic("wrong add inst");

    int s1 = get();
    if(code.a == _IMD) {
        int c1 = code.b;
        
        cpush(access(s1) - c1);
    }
    else {
        int s2 = get();
        cpush(access(s2) - access(s1));
    }
}
void mul(BCode code) {
    // mul <s1> <s2> : s1 * s2
    // mul imd <c1> <s1> : s1 * c1

    if(code.opcode != _MUL) panic("wrong add inst");

    int s1 = get();
    if(code.a == _IMD) {
        int c1 = code.b;
        
        cpush(access(s1) * c1);
    }
    else {
        int s2 = get();
        cpush(access(s2) * access(s1));
    }
}
void div(BCode code) {
    // div <s1> <s2> : s1 / s2
    // div imd <c1> <s1> : s1 / c1

    if(code.opcode != _DIV) panic("wrong add inst");

    int s1 = get();
    if(code.a == _IMD) {
        int c1 = code.b;
        
        cpush(access(s1) / c1);
    }
    else {
        int s2 = get();
        cpush(access(s2) / access(s1));
    }
}
void mod(BCode code) {
    // mod <s1> <s2> : s1 % s2
    // mod imd <c1> <s1> : s1 % c1

    if(code.opcode != _MOD) panic("wrong add inst");

    int s1 = get();
    if(code.a == _IMD) {
        int c1 = code.b;
        
        cpush(access(s1) % c1);
    }
    else {
        int s2 = get();
        cpush(access(s2) % access(s1));
    }
}
void addf(BCode code) {
    // addf <s1> <s2> : s1 + s2

    double y = getDouble();
    double x = getDouble();

    cpush(x+y);
}
void subf(BCode code) {
    // subf <s1> <s2> : s1 - s2

    double y = getDouble();
    double x = getDouble();

    cpush(x-y);
}
void mulf(BCode code) {
    // mulf <s1> <s2> : s1 * s2

    double y = getDouble();
    double x = getDouble();

    cpush(x*y);
}
void divf(BCode code) {
    // divf <s1> <s2> : s1 / s2

    double y = getDouble();
    double x = getDouble();

    cpush(x/y);
}
void _and(BCode code) {
    // and <s1> <s2> : s1 & s2
    // and imd <c1> <s1> : s1 & c1
}
void _or(BCode code) {
    // or <s1> <s2> : s1 | s2
    // or imd <c1> <s2> : s1 | c1
}
void _xor(BCode code) {
    // xor <s1> <s2> : s1 ^ s2
    // xor imd <c1> <s1> : s1 ^ c1
}
void lsh(BCode code) {
    // lsh <s1> <s2> : s1 << s2
    // lsh imd <c1> <s1> : s1 << c1
}
void rsh(BCode code) {
    // rsh <s1> <s2> : s1 >> s2
    // rsh imd <c1> <s1> : s1 >> c1
}
void _not(BCode code) {
    // not <s1> : ~s1
    // not imd <c1> : ~c1
}
void land(BCode code) {
    // land <s1> <s2> : s1 && s2
}
void lor(BCode code) {
    // lor <s1> <s2> : s1 || s2
}
void lnot(BCode code) {
    // lnot <s1> : ! s1
}
void push(BCode code) {
    // push 0 <c1>
    // push imd <c1>

    if(code.opcode != _PUSH) {
        panic("wrong push inst");
    }

    if(code.a == _IMD) {
        int ptr = store(code.b);
        cpush(ptr);
    }
    else {
        cpush(code.b);
    }
}
void pop(BCode code) {
    // pop
}
void cp(BCode code) {
    // cp
}
void ass(BCode code) {
    // ass <s1> <s2> : s1 = s2
    // ass imd <c1> <s1> : s1 = c1
}
void acc(BCode code) {
    // acc
}
void addr(BCode code) {
    // addr

}
void alloc(BCode code) {
    // alloc <s1>
    // alloc imd <c1>

    int size;

    if(code.a == _IMD) {
        size = code.b;
    }
    else {
        size = get();
    }

    int i = 1;
    int ret;
    int cnt;
    while(i < HEAP.size()){
        if(HEAP[i].first != 0) {
            for(int j = 0;j < HEAP[i].first;j++) {
                i++;
            }
        }
        else {
            ret = i;
            cnt = 1;
            while(i < HEAP.size() && HEAP[i].first == 0 && cnt < size) {
                cnt++;
                i++;
            }

            if(cnt >= size) {
                HEAP[ret].first = size;
                break;
            }
        }
    }

    if(cnt < size) {
        for(int i = 0;i < size - cnt;i++) {
            HEAP.push_back(std::make_pair(0,0));
        }
        HEAP[ret].first = size;
    }

    cpush(ret);
}
void _free(BCode code) {
    // free

    int ptr = get();

    if(ptr <= 0) return;

    HEAP[ptr].first = 0;
}
void data(BCode code) {
    // data <c1>

    alloc(bc(_ALLOC,_IMD,1,0));
    push(bc(_PUSH,_IMD,code.a,0));
    ass(bc(_ASS,0,0,0));
}
void jmp(BCode code) {
    // jmp <s1>
    // jmp add <s1>
    // jmp <c1> imd 
    // jmp add <c1> imd 

    int jp;
    if(code.a == _ADD) {
        if(code.c == _IMD) {
            jp = code.b;
        }
        else {
            jp = pc + get();
        }
    }
    else if(code.a == _IMD) {
        jp = code.b;
    }
    else {
        jp = get();
    }

    pc = jp;
}
void _if(BCode code) {
    // if <s1> <s2>
    // if add <s1> <s2>
    // if <c1> imd <s1>
    // if add <c1> imd <s1>

    int cond;
    int jp;
    if(code.a == _ADD) {
        if(code.c == _IMD) {
            jp = pc + code.b;
            cond = get();
        }
        else {
            jp = pc + get();
            cond = get();
        }
    }
    else if(code.a == _IMD) {
        jp = code.b;
        cond = get();
    }
    else {
        jp = get();
        cond = get();
    }

    if(cond != 0) {
        pc = jp;
    }
}
void call(BCode code) {
    // call <s1> <c1> <arg>
    // call imd <c1> <c1> <arg>
}
void _return(BCode codd) {
    // return <s1>
    // return imd <c1>
}
void end(BCode code) {
    // end <s1>
    // end imd <c1>
    if(code.opcode != _END) panic("wrong end inst");
    if(code.a == _IMD) {
        exitcode = code.b;
    }
    else {
        exitcode = get();
    }

    alive = false;
}
void clr(BCode code) {
    // clr

    if(code.opcode != _CLR) panic("wrong clr inst");
    while(!CSTACK.empty()) {
        CSTACK.pop();
    }
}
void initRun() {
    STACK.push_back(0);
    HEAP.push_back(std::make_pair(1,0));
    CSTACK.push(std::stack<int>());
}
int run(std::vector<BCode> code) {
    alive = true;
    while(pc < code.size() && alive) {
        auto c = code[pc];

        switch (c.opcode) {
        case _ADD:
            add(c);
            break;
        case _SUB:
            sub(c);
            break;
        case _MUL:
            mul(c);
            break;
        case _DIV:
            div(c);
            break;
        case _MOD:
            mod(c);
            break;
        case _ADDF:
            addf(c);
            break;
        case _SUBF:
            subf(c);
            break;
        case _MULF:
            mulf(c);
            break;
        case _DIVF:
            divf(c);
            break;
        case _AND:
            _and(c);
            break;
        case _OR:
            _or(c);
            break;
        case _XOR:
            _xor(c);
            break;
        case _LSH:
            lsh(c);
            break;
        case _RSH:
            rsh(c);
            break;
        case _NOT:
            _not(c);
            break;
        case _LAND:
            land(c);
            break;
        case _LOR:
            lor(c);
            break;
        case _LNOT:
            lnot(c);
            break;
        case _PUSH:
            push(c);
            break;
        case _POP:
            pop(c);
            break;
        case _CP: 
            cp(c);
            break;
        case _ASS:
            ass(c);
            break;
        case _ACC:
            acc(c);
            break;
        case _ADDR:
            addr(c);
            break;
        case _ALLOC:
            alloc(c);
            break; 
        case _FREE:
            _free(c);
            break;
        case _JMP:
            jmp(c);
            break;
        case _IF:
            _if(c);
            break;
        case _CALL:
            call(c);
            break;
        case _RET:
            _return(c);
            break;
        case _END:
            end(c);
            break;
        case _CLR:
            clr(c);
            break;
        case _ENTER:
            enter();
            break;
        case _LEAVE:
            leave();
            break;
        default :
            panicf("unknown opcode %d", c.opcode);
        }

        pc++;
    }
    return exitcode;
}

#endif