#ifndef __PARSE_HPP__
#define __PARSE_HPP__

#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "string.hpp"
#include "addScolon.hpp"
#include "panic.hpp"
#include "keywordDict.hpp"
#include "token.hpp"

char errMsg[ERR_MSG_LEN];
extern KeywordDict kDict;

void parsePanic(int line, int character, const char *msg) {
    panicf("at line %d, %d: %s", line, character, msg);
}
double makeReal(unsigned long long x, unsigned long long y) {
    double a,b;
    a = x;
    b = y;

    while(1.0 < b) {
        b /= 10;
    }

    return a + b;
}
unsigned long long parseNumber(const char* str, int &header, int length) {
    //parse number that has less than 'length' digits

    int len = strlen(str);

    unsigned long long ret = 0;

    if(hasPrefix("0x", str)) {
        // hexadecimal number

        header += 2;
        while(header < len && ((str[header] >= '0' && str[header] <= '9') || (str[header] >= 'a' && str[header] <= 'f' || (str[header] >= 'A' && str[header] <= 'F')))) {
            ret *= 16;
            if(str[header] >= '0' && str[header] <= '9') {
                ret += str[header] - '0';
            }
            else if(str[header] >= 'a' && str[header] <= 'f') {
                ret += str[header] - 'a' + 10;
            }
            else {
                ret += str[header] - 'A' + 10;
            }

            header++;
        }
    }
    else if(hasPrefix("0", str)) {
        // octal number
        header += 1;
        while(header < len && (str[header] >= '0' && str[header] <= '7')) {
            ret *= 8;
            if(str[header] >= '0' && str[header] <= '7') {
                ret += str[header] - '0';
            }
            header++;
        }
    }
    else {
        // decimal number
        while(header < len && (str[header] >= '0' && str[header] <= '9')) {
            ret *= 10;
            if(str[header] >= '0' && str[header] <= '9') {
                ret += str[header] - '0';
            }
            header++;
        }
    }

    return ret;
}
std::vector<Token> parse(const char* str) {
    std::vector<Token> ret;
    int len = strlen(str);

    int line = 1;
    int sp = -1;

    for(int i = 0;i < len;) {
        if(str[i] == ' ' || str[i] == '\t' || str[i] == '\r') {
            // white space
            i++;
        }
        else if(str[i] == '\n') {
            // end of line
            while(i < len && str[i] == '\n') {
                if(ret.size() != 0) {
                    auto last = ret.back();
                    if(!isTaboo(last.kind)) {
                        ret.push_back(Token{SCOLON, ";", 0, 0, line, i-sp});
                    }
                }
                i++;
                line++;
                sp = i - 1;
            }
        }
        else if(i + 1 < len && str[i] == '/' && str[i+1] == '/') {
            i += 2;
            while(i < len && str[i] != '\n') i++;
        }
        else if(str[i] >= '0' && str[i] <= '9') {
            // literal number
            unsigned long long x = parseNumber(str, i, 32);
            if(i < len && str[i] == '.'){
                i++;
                unsigned long long y = parseNumber(str, i, 32);
                ret.push_back(Token{LREAL, NULL, makeReal(x, y), 0, line, i - sp});
            }
            else
                ret.push_back(Token{LNUM, NULL, 0, x, line, i-sp});
        }
        else if(str[i] == '\\') {
            // maybe unsigned number literal or byte literal
            i++;
            if(i >= len) {
                parsePanic(line, i - sp, "unexpected \\");
            }
            else if(str[i] == 'b' || str[i] == 'B') {
                //literal byte number

                i++;
                Token t = {LBYTE, NULL, 0, parseNumber(str, i, 2), line, i-sp-1};

                ret.push_back(t);
            }
            else {
                parsePanic(line, i - sp, "unknown literal format");
            }
        }
        else if(str[i] == '"') {
            // maybe literal string

            int begin = i+1;
            int end;

            i++;
            while(i < len && str[i] != '"') {
                if(str[i] == '\\') {
                    i++;
                }
                else if(str[i] == '\n') {
                    parsePanic(line, i - sp, "literal string cannot include end of line");
                }
                
                i++;
            }
            end = i;
            i++;

            ret.push_back(Token{LSTR, substr(str, begin, end), 0, 0, line, begin-sp});
        }
        else if(str[i] == '_' || (str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z')) {
            // maybe word or keyword

            int begin = i;
            int end;

            while(i < len && (str[i] == '_' || (str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= '0' && str[i] <= '9'))) {
                i++;
            }
            end = i;

            char *word = substr(str, begin, end);

            int v = kDict.get(word);
            if(v != -1) {
                ret.push_back(Token{v, word, 0, 0, line, begin-sp});
            }
            else {
                ret.push_back(Token{WORD, word, 0, 0, line, begin-sp});
            }
        }
        else {
            // maybe operator

            bool found = false;
            for(int length = 3; length >= 1 && !found; length--) {
                if(i + length <= len) {
                    char *s = substr(str, i, i+length);
                    int v = kDict.get(s);

                    if(v != -1) {
                        found = true;
                        i += length;
                        ret.push_back(Token{ v, s, 0, 0, line, i - length - sp });
                    }
                }
            }
            if(!found) {
                parsePanic(line, i - sp, "unknown operator");
            }
        }
    }

    ret.push_back(Token{SCOLON});

    return ret;
}

#endif