#ifndef __RUN_HPP__
#define __RUN_HPP__

#include "RDP.hpp"
#include <setjmp.h>

struct JP {
    jmp_buf p;
};
int saveJP(JP jp) {
    jmp_buf p;
    int ret = setjmp(p);

    if(ret == 0) {
        for(int i = 0;i < 48;i++) {
            jp.p[i] = p[i];
        }
        return 0;
    }

    return ret;
}
void jmp(JP jp, int value) {
    longjmp(jp.p, value);
}

void f(JP p) {
    jmp(p, 1);
}
void run(Program prog) {
    JP jp;

    if(saveJP(jp) == 0) {
        puts("before jump");
        f(jp);
    }
    else {
        puts("after jump");
    }
}

#endif