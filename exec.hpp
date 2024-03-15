#ifndef __EXEC_HPP__
#define __EXEC_HPP__

#include "RDP.hpp"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <stack>
#include <algorithm>

struct Value {
    Type type;
    int value;
};
struct Scope {
    std::vector<std::pair<const char*, Value> > local; 
};

const char *printType(Type type);
const char *join(std::vector<const char*> strs);

static std::stack<std::vector<Scope> > STACK;
static std::vector<int> HEAP;
static std::vector<DefClass> _CLASS;
static std::vector<DefFunc> _FUNC;
static std::vector<DefInterface> _INTERFACE;

static int exitCode = 0;

void enterScope();
void exitScope();

Scope &now() {
    return STACK.top().back();
}

int alloc(Type x);
void free(Type x);

bool assignAble(Type x, Type y);
void assign(Value &x, Value &y);

bool isExists(const char *name);
int findVar(const char *name);
int findClass(const char *name);
int findInterface(const char *name);
int findFunc(const char *name);

void initRun(Program prog);

Value defaultValue(Type type);

Value newObj(Type type, InitList init);
Value _Idx(Value list, Value idx);
Value fCall(int idx, FCall arg);

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
int runProgram(Program node);

const char *join(std::vector<const char *> strs) {
    int len = 0;
    for(int i = 0;i < strs.size();i++) {
        len += strlen(strs[i]);
    }

    char *str = (char *) malloc(sizeof(char) * len);
    int cnt = 0;
    for(int i = 0;i < strs.size();i++) {
        int len = strlen(strs[i]);
        for(int j = 0;j < len;j++) {
            str[cnt++] = strs[i][j];
        }
    }

    return str;
}
const char *printType(Type type) {
    if(type.kind == NUM) {
        return "num";
    }
    else if(type.kind == UNUM) {
        return "unum";
    }
    else if(type.kind == BYTE) {
        return "byte";
    }
    else if(type.kind == BOOL) {
        return "bool";
    }
    else if(type.kind == OSB) {
        char* str = (char *)malloc(sizeof(char) * 1024);
        sprintf(str, "[]%s", printType(type.frame[0]));
        return str;
    }
    else if(type.kind == FUNC) {
        std::vector<const char *> strs;
        strs.push_back("func (");
        strs.push_back(")");
        if(type.frame[0].kind != 0) {
            strs.push_back(" -> ");
            strs.push_back(printType(type.frame[0]));
        }
        return join(strs);
    }
    else if(type.kind == CLASS) {
        return type.name;
    }
    else if(type.kind == 0) {
        return "void";
    }
    else {
        panicf("wrong type");
    }

    return "";
}
void runDefVar(DefVar node) {
    DefVar defvar = node;
    Value init;

    Value var;

    if(defvar.init != NULL) {
        init = runExpr(*defvar.init);
        if(defvar.type != NULL) {
            if(!assignAble(*defvar.type, init.type)) {
                panicf("cannot assign %s to %s", printType(init.type), printType(var.type));
            }

            var.type = *defvar.type;
            assign(var, init);
        }
        else {
            var.type = init.type;
            assign(var, init);
        }
    }
    else {
        if(defvar.type == NULL) {
            panicf("cannot decide type of var:%s", defvar.name);
        }

        var.type = *defvar.type;
        assign(var, init);
    }

    now().local.push_back(std::make_pair(defvar.name, var));
}

void initRun(Program prog) {
    STACK.push(std::vector<Scope>());
    STACK.top().push_back((Scope){});

    for(int i = 0;i < prog.childs.size();i++) {
        auto p = prog.childs[i];
        auto kind = p.first;
        auto ptr = p.second;

        if(kind == CLASS) {
            _CLASS.push_back(*(DefClass*) ptr);
        }
        else if(kind == INTERFACE) {
            _INTERFACE.push_back(*(DefInterface*) ptr);
        }
        else if(kind == FUNC) {
            _FUNC.push_back(*(DefFunc*) ptr);
        }
        else if(kind == VAR) {
            runDefVar(*(DefVar*) ptr);
        }
        else panic("wrong PROGRAM");
    }
}
int runProgram(Program node){
    initRun(node);

    int _main = findFunc("main");
    if(_main == -1) panic("main function does not exist");

    return fCall(_main, (FCall){}).value;
}

#endif