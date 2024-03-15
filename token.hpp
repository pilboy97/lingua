#ifndef __TOKEN_HPP__
#define __TOKEN_HPP__

#include "code.hpp"

struct Token {
    int kind;
    const char *str;
    unsigned long long value;
    int line, col;
};

#endif