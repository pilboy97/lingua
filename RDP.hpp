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
struct Expression {
    std::vector<Expr1> childs;
};
struct Number {
    unsigned long long value;
};
struct VarName {
    char *name;
};
struct Type {
    int kind;
    std::vector<Type> frame;
};
struct LiteralString {
    char *str;
};
struct True {

};
struct False {

};
struct Statement {
    int kind;
    void *stmt;
};
struct BlockStatment {
    std::vector<Statement> childs;
};
struct IfStatement {
    std::vector<std::pair<Expression, BlockStatment> > _if;
    BlockStatment *_else;
};
struct ForStatement {
    Expression *init;
    Expression cond;
    Expression *act;

    BlockStatment body;
};
struct AssignStatement {
    Expression src;
    Expression dst;
};
struct DefVar {
    char *name;
    Type type;
    Expression *init;
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
    BlockStatment body;
};
struct Program {
    std::vector<std::pair<int, void*> > childs;
};

Program parseProgram(std::vector<Token> code) {
    header = 0;
    ::code = code;
}
#endif