#ifndef __PANIC_HPP__
#define __PANIC_HPP__

#include <stdlib.h>
#include <cstdint>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define ERR_MSG_LEN 1024

void panic(std::string msg) {
    printf("%s\n", msg.c_str());
    exit(1);
}
void panicf(std::string msg, ...) {
    va_list argptr;
    va_start(argptr, msg);

    int len = msg.length();
    int i;

    for(i = 0;i < len;i++) {
        if(msg[i] == '%') {
            i++;
            if(i >= len) {
                panic("panicf: wrong formatting string");
            }

            if(msg[i] == 'd') {
                int num = va_arg(argptr, int);
                printf("%d", num);
            }
            else if(msg[i] == 's') {
                const char * str = va_arg(argptr, const char *);
                printf("%s", str);
            }
            else if(msg[i] == 'f') {
                double fp = va_arg(argptr, double);
                printf("%f", fp);
            }
            else if(msg[i] == 'b') {
                bool b = va_arg(argptr, bool);
                if(b) {
                    printf("true");
                }
                else {
                    printf("false");
                }
            }
        } else {
            putchar(msg[i]);
        }
    }
    puts("");
    exit(1);
}

#endif