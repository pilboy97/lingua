#ifndef __TOKEN_HPP__
#define __TOKEN_HPP__

#include "code.hpp"
#include <string>

struct Token {
    int kind;
    std::string str;
    double real;
    unsigned long long value;
    int line, col;
};

#endif