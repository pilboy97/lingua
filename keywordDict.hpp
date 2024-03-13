#ifndef __KEYWORDDICT_HPP__
#define __KEYWORDDICT_HPP__

#include <algorithm>
#include <vector>
#include <set>

#include "code.hpp"

struct KeywordDict {
    std::vector<std::pair<const char *, int> > dict;

    int get(const char * keyword) {
        for(int i = 0;i < dict.size();i++) {
            if(strcmp(keyword, dict[i].first) == 0) {
                return dict[i].second;
            }
        }

        return -1;
    }
    void push(const char * keyword, int value) {
        if(get(keyword) >= 0) {
            return;
        }

        dict.push_back(std::make_pair(keyword, value));
    }
    const char* sprint(int value) {
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
                return "variable name";
            case EOL:
                return "end of line";
            default:
                break;
        }

        for(int i = 0;i < dict.size();i++) {
            if(dict[i].second == value) {
                return dict[i].first;
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
    kDict.push("unum", UNUM);
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
    kDict.push("func", FUNC);
    kDict.push("var", VAR);
    kDict.push("if", IF);
    kDict.push("else", ELSE);
    kDict.push("for", FOR);
    kDict.push("return", RETURN);
    kDict.push("break", BREAK);
    kDict.push("continue", CONTINUE);
    kDict.push("defer", DEFER);
    kDict.push(":", COLON);
    kDict.push(";", SCOLON);
}

#endif