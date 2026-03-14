#ifndef __KEYWORDDICT_HPP__
#define __KEYWORDDICT_HPP__

#include <algorithm>
#include <unordered_map>
#include <string>
#include <set>

#include "code.hpp"

struct KeywordDict {
    std::unordered_map<std::string, int> dict;

    int get(std::string keyword) {
        if (dict.find(keyword) != dict.end()) return dict[keyword];

        return -1;
    }
    void push(std::string keyword, int value) {
        if(get(keyword) >= 0) {
            return;
        }

        dict.insert(std::make_pair(keyword, value));
    }
    
    std::string sprint(int value) {
        switch (value) {
            case LNUM:
                return "literal number";
            case LSTR:
                return "literal string";
            case LUNUM:
                return "literal unsigned number";
            case LREAL:
                return "literal real number";
            case LBYTE:
                return "literal byte number";
            case LTRUE:
                return "true";
            case LFALSE:
                return "false";
            case WORD:
                return "word";
            case EOL:
                return "end of line";
            default:
                break;
        }

        for(auto it = dict.begin(); it != dict.end(); it++) {
            if(it->second == value) {
                return it->first;
            }
        }

        return "UNKNOWN";
    }
};

KeywordDict kDict;

void initDict() {
    kDict.push("true", LTRUE);
    kDict.push("false", LFALSE);
    kDict.push("num", NUM);
    kDict.push("real", REAL);
    kDict.push("byte", BYTE);
    kDict.push("bool", BOOL);
    kDict.push("+", ADD);
    kDict.push("-", SUB);
    kDict.push("*", MUL);
    kDict.push("/", DIV);
    kDict.push("%", MOD);
    kDict.push("=", ASS);
    kDict.push(">", GR);
    kDict.push("<", LE);
    kDict.push(">=", GEQ);
    kDict.push("<=", LEQ);
    kDict.push("==", EQ);
    kDict.push("!=", NEQ);
    kDict.push("~", NOT);
    kDict.push("&", AND);
    kDict.push("|", OR);
    kDict.push("^", XOR);
    kDict.push(">>", RSH);
    kDict.push("<<", LSH);
    kDict.push("not", LNOT);
    kDict.push("and", LAND);
    kDict.push("or", LOR);
    kDict.push("is", IS);
    kDict.push("as", AS);
    kDict.push(".", DOT);
    kDict.push(",", COMMA);
    kDict.push("[", OSB);
    kDict.push("(", OBR);
    kDict.push("{", OBL);
    kDict.push("->", ARROW);
    kDict.push("---", SEP);
    kDict.push("null", NIL);
    kDict.push(")", CBR);
    kDict.push("]", CSB);
    kDict.push("}", CBL);
    kDict.push("class", CLASS);
    kDict.push("interface", INTERFACE);
    kDict.push("this", THIS);
    kDict.push("func", FUNC);
    kDict.push("var", VAR);
    kDict.push("if", IF);
    kDict.push("else", ELSE);
    kDict.push("for", FOR);
    kDict.push("return", RETURN);
    kDict.push("break", BREAK);
    kDict.push("continue", CONTINUE);
    kDict.push("defer", DEFER);
    kDict.push("super", SUPER);
    kDict.push(":", COLON);
    kDict.push(";", SCOLON);
}

#endif