#ifndef __RDP_H__
#define __RDP_H__

#include <vector>
#include <algorithm>
#include "token.hpp"
#include "keywordDict.hpp"
#include "panic.hpp"

static int header;
static std::vector<Token> code;


void forward() {
    header++;
}
bool chk(int kind) {
    if(header < code.size()) return code[header].kind == kind;
}
void must(int kind) {
    if(header >= code.size()) 
        panic("unexpected EOF");
    if(code[header].kind != kind)
        panicf("unexpected %s: %s needed", kDict.sprint(code[header].kind), kDict.sprint(kind));

    header++;
}

struct Factor {
    int kind;
    void *ptr;
};
struct Expr12 {
    Factor f;
    std::vector<std::pair<int, void*> > childs;
};
struct Expr11 {
    std::vector<std::pair<int, Expr12> > childs;
};
struct Expr10 {
    std::vector<std::pair<int, Expr11> > childs;
};
struct Expr9 {
    std::vector<std::pair<int, Expr10> > childs;
};
struct Expr8 {
    std::vector<std::pair<int, Expr9> > childs;
};
struct Expr7 {
    std::vector<std::pair<int, Expr8> > childs;
};
struct Expr6 {
    std::vector<std::pair<int, Expr7> > childs;
};
struct Expr5 {
    std::vector<std::pair<int, Expr6> > childs;
};
struct Expr4 {
    std::vector<Expr5> childs;
};
struct Expr3 {
    std::vector<Expr4> childs;
};
struct Expr2 {
    std::vector<Expr3> childs;
};
struct Expr1 {
    std::vector<Expr2> childs;
};
struct Expr {
    std::vector<Expr1> childs;
};
struct Number {
    long long value;
};
struct UNumber { 
    unsigned long long value;
};
struct ByteNumber {
    unsigned char value;
};
struct Word {
    char *word;
};
struct LiteralString {
    char *str;
};
struct True {

};
struct False {

};
struct Object {
    Type type;
    std::vector<std::pair<char*, Expr> > value;
};
struct Type {
    int kind;
    std::vector<Type> frame;
};
struct Stmt {
    int kind;
    void *stmt;
};
struct BlockStmt {
    std::vector<Stmt> childs;
};
struct IfStmt {
    std::vector<std::pair<Expr, BlockStmt> > _if;
    BlockStmt *_else;
};
struct ForStmt {
    Expr *init;
    Expr cond;
    Expr *act;

    BlockStmt body;
};
struct AssignStmt {
    Expr src;
    Expr dst;
};
struct DefVar {
    char *name;
    Type type;
    Expr *init;
};
struct Method {
    bool isPrivate;
    std::vector<std::pair<char *, Type> > frame;
    Type ret;
};
struct Attr {
    bool isPrivate;
    char *name;
    Type type;
};
struct DefInterface {
    char *name;
    std::vector<Method> method;
};
struct DefClass {
    char *name;
    std::vector<Attr> attr;
    std::vector<Method> method;
};
struct DefFunc {
    char *name;
    std::vector<std::pair<char *, Type> > frame;
    Type ret;
    BlockStmt body;
};
struct Program {
    std::vector<std::pair<int, void*> > childs;
};

Factor parseFactor() {
    if(chk(OBR)) {

    }
    else if(chk(LNUM)) {

    }
    else if(chk(LBYTE)) {
        
    }
    else if(chk(LSTR)) {

    }
    else if(chk(WORD)) {

    }
}
Expr12 parseExpr12() {

}
Expr11 parseExpr11() {

}
Expr10 parseExpr10() {

}
Expr9 parseExpr9() {

}
Expr8 parseExpr8() {

}
Expr7 parseExpr7() {

}
Expr6 parseExpr6() {

}
Expr5 parseExpr5() {

}
Expr4 parseExpr4() {

}
Expr3 parseExpr3() {

}
Expr2 parseExpr2() {

}
Expr1 parseExpr1() {

}
Expr parseExpr() {

}
AssignStmt parseAssignStmt() {

}
ForStmt parseForStmt() {

}
IfStmt parseIfStmt() {

}
BlockStmt parseBlockStmt() {

}
Stmt parseStmt() {

}
DefVar parseDefVar() {

}
DefClass parseDefClass() {

}
DefInterface parseDefInterface() {

}
DefFunc parseDefFunc() {

}

Program parseProgram() {
    Program ret;

    while(header < ::code.size()) {
        if(chk(FUNC)) {
            DefFunc *neo = new DefFunc();
            *neo = parseDefFunc();
            ret.childs.push_back(std::make_pair(FUNC, neo));
        }
        else if(chk(VAR)) {
            DefVar *neo = new DefVar();
            *neo = parseDefVar();
            ret.childs.push_back(std::make_pair(VAR, neo));
        }
        else if(chk(CLASS)) {
            DefClass *neo = new DefClass();
            *neo = parseDefClass();
            ret.childs.push_back(std::make_pair(CLASS, neo));
        }
        else if(chk(INTERFACE)) {
            DefInterface *neo = new DefInterface();
            *neo = parseDefInterface();
            ret.childs.push_back(std::make_pair(INTERFACE, neo));
        }
        else {
            panicf("unexpected %s", kDict.sprint(code[header].kind));
        }
    }
}

Program runRDP(std::vector<Token> code) {
    header = 0;
    ::code = code;

    return parseProgram();
}
#endif