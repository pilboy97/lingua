#pragma once
#ifndef __PROCSTRUCT_HPP__
#define __PROCSTRUCT_HPP__

#include <algorithm>
#include <vector>

#include "token.hpp"

struct Type {
    int kind;
    const char* name;
    std::vector<Type> add;
};

struct Factor {
    int kind;
    void* ptr;
};
struct Expr12 {
    Factor f;
    std::vector<std::pair<TokenCode, void*> > childs;
};
struct Expr11 {
    Expr12 child;
    std::vector<TokenCode> oper;
};
struct Expr10 {
    Expr11 child;
    std::vector<std::pair<TokenCode, Type> > oper;
};
struct Expr9 {
    std::vector<std::pair<TokenCode, Expr10> > childs;
};
struct Expr8 {
    std::vector<std::pair<TokenCode, Expr9> > childs;
};
struct Expr7 {
    std::vector<std::pair<TokenCode, Expr8> > childs;
};
struct Expr6 {
    std::vector<std::pair<TokenCode, Expr7> > childs;
};
struct Expr5 {
    std::vector<std::pair<TokenCode, Expr6> > childs;
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
struct Expr_1 { // src = dst
    Expr* src;
    Expr dst;
};
struct FCall {
    std::vector<Expr> list;
};
struct Idx {
    Expr id;
};
struct InitList {
    std::vector<std::pair<const char*, Expr> > list;
};
struct Number {
    long long value;
};
struct Real {
    double value;
};
struct ByteNumber {
    unsigned char value;
};
struct Word {
    const char* word;
};
struct LiteralString {
    const char* str;
};
struct LiteralArray {
    Type etype;
    std::vector<Expr> elem;
};
struct LiteralObject {
    Type type;
    InitList init;
};
struct Stmt {
    int kind;
    void* stmt;
};
struct BlockStmt {
    std::vector<Stmt> childs;
};
struct IfStmt {
    std::vector<std::pair<Expr, BlockStmt> > _if;
    BlockStmt* _else;
};
struct ForStmt {
    Expr_1* init;
    Expr_1* cond;
    Expr_1* act;

    BlockStmt body;
};
struct DeferStmt {
    BlockStmt block;
};
struct RetStmt {
    Expr* expr;
};
struct DefVar {
    const char* name;
    Type* type;
    Expr* init;
};
struct Method {
    bool isPrivate;
    const char* name;
    int idx;
};
struct Field {
    bool isPrivate;
    const char* name;
    Type type;
};
struct IMember {
    bool isPrivate;
    const char* name;
    std::vector<Type> frame;
    Type ret;
};
struct DefInterface {
    const char* name;
    std::vector<IMember> method;
};
struct DefClass {
    const char* name;
    const char* super;
    std::vector<Field> field;
    std::vector<Method> method;
};
struct DefFunc {
    const char* name;
    std::vector<std::pair<const char*, Type> > frame;
    std::vector<const char* > captured;
    std::vector<Type> cType;
    Type ret;
    BlockStmt body;
};
struct Program {
    std::vector<std::pair<int, void*> > childs;
    std::vector<DefFunc> fn;
};

bool isNum(Type x);
bool isByte(Type x);
bool isBool(Type x);
bool isReal(Type x);
bool isArray(Type x);
bool isFunc(Type x);
bool isObject(Type x);
bool isSameType(Type x, Type y);
bool isImplOf(DefInterface x, DefClass y);
bool isPriType(Type x);
Type fnToType(DefFunc x);

void printStmt(Stmt s);
void printForStmt(ForStmt s);
void printIfStmt(IfStmt s);
void printRetStmt(RetStmt s);
void printDefVar(DefVar s);

void printType(Type x);

void printExpr_1(Expr_1 e);
void printExpr(Expr e);
void printExpr1(Expr1 e);
void printExpr2(Expr2 e);
void printExpr3(Expr3 e);
void printExpr4(Expr4 e);
void printExpr5(Expr5 e);
void printExpr6(Expr6 e);
void printExpr7(Expr7 e);
void printExpr8(Expr8 e);
void printExpr9(Expr9 e);
void printExpr10(Expr10 e);
void printExpr11(Expr11 e);
void printExpr12(Expr12 e);
void printFactor(Factor f);

#endif