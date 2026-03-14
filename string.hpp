#ifndef __STRING_HPP__
#define __STRING_HPP__

#include <string.h>
#include <stdlib.h>

char* substr(std::string str, int begin, int end) {
    char* ret = (char*)malloc(sizeof(char) * (end - begin + 1));
    for (int i = begin; i < end; i++) {
        ret[i - begin] = str[i];
    }
    ret[end - begin] = '\0';
    return ret;
}
bool hasPrefix(std::string pre, std::string str) {
    return strncmp(pre.c_str(), str.c_str(), pre.length()) == 0;
}

#endif