#ifndef __STRING_HPP__
#define __STRING_HPP__

#include <string.h>
#include <stdlib.h>

char* substr(const char* str, int begin, int end) {
    char* ret = (char*)malloc(sizeof(char) * (end - begin + 1));
    for (int i = begin; i < end; i++) {
        ret[i - begin] = str[i];
    }
    ret[end - begin] = '\0';
    return ret;
}
bool hasPrefix(const char* pre, const char* str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

#endif