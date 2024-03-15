#ifndef __EXEC_HPP__
#define __EXEC_HPP__

#include "RDP.hpp"


struct Value {
    Type type;
    int ptr;
};

Value newObj(Type type, InitList init);
Value Idx(Value list, Value idx);
Value fCall(Value fn, FCall arg);

Value runFactor(Factor node);
Value runExpr12(Expr12 node);
Value runExpr11(Expr11 node);
Value runExpr10(Expr10 node);
Value runExpr9(Expr9 node);
Value runExpr8(Expr8 node);
Value runExpr7(Expr7 node);
Value runExpr6(Expr6 node);
Value runExpr5(Expr5 node);
Value runExpr4(Expr4 node);
Value runExpr3(Expr3 node);
Value runExpr2(Expr2 node);
Value runExpr1(Expr1 node);
Value runExpr(Expr node);
void runBlockStmt(BlockStmt node);
void runIfStmt(IfStmt node);
void runForStmt(ForStmt node);
void runDefVar(DefVar node);
void runDefClass(DefClass node);
void runDefInterface(DefInterface node);
int runProgram(Program node);

int runProgram(Program node){
}

#endif