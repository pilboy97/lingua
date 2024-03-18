#ifndef __RUN_HPP__
#define __RUN_HPP__

#include "RDP.hpp"

#include <vector>
#include <stack>
#include <algorithm>
#include <setjmp.h>
#include <string.h>

#define ll long long
#define MEM_SIZE 1024*1024

struct Pointer {
	Type type;
	bool lv;
	int ptr;
};
struct Frame {
	std::vector<std::vector<std::pair<const char *, Pointer>>> vars;
};
struct Object {
	int kind;
	void* ptr;
};
struct Str {
	const char* str;
};
struct Array {
	Type etype;
	std::vector<ll> array;
};
struct Instance {
	Type type;
	std::vector<ll> fields;
};
struct Closure {
	DefFunc fn;
	Pointer Self;
	std::vector<ll> cap;
};
struct Proc {
	int HP;
	int SP;
	int ESP;
	std::vector<Object> org;
	std::vector<ll> mem;
	std::stack<Frame> STACK;
	std::vector<DefClass> CLASS;
	std::vector<DefInterface> INTERFACE;
};

Proc proc;

enum jmpCode {
	_HALT = 1,
	_RET,
	_BRK,
	_CON
};

bool isNull(Pointer ptr);

void run(Program prog);

int hAlloc(int size);
int sAlloc(int size);

void runStmt(Stmt stmt, jmp_buf jmp);
void runIfStmt(IfStmt stmt, jmp_buf jmp);
void runForStmt(ForStmt stmt, jmp_buf jmp);

void runDefClass(DefClass df);
void runDefInterface(DefInterface df);
void runDefFunc(DefFunc df);
void runDefVar(DefVar df);

bool isExists(const char* name);
Pointer getVar(const char* name);
int getClass(const char* name);
int getInterface(const char* name);

Closure getFn(Pointer x);
Array getArray(Pointer x);
Str getStr(Pointer x);
Instance getInstance(Pointer x);

Closure makeClosure(DefFunc fn);

Pointer runExpr_1(Expr_1 e);
Pointer runExpr(Expr e);
Pointer runExpr1(Expr1 e);
Pointer runExpr2(Expr2 e);
Pointer runExpr3(Expr3 e);
Pointer runExpr4(Expr4 e);
Pointer runExpr5(Expr5 e);
Pointer runExpr6(Expr6 e);
Pointer runExpr7(Expr7 e);
Pointer runExpr8(Expr8 e);
Pointer runExpr9(Expr9 e);
Pointer runExpr10(Expr10 e);
Pointer runExpr11(Expr11 e);
Pointer runExpr12(Expr12 e);
Pointer runFactor(Factor e);


bool isNull(Pointer ptr) {
	return ptr.ptr == 0;
}
void initProc() {
	proc.mem = std::vector<ll>(MEM_SIZE);
	proc.HP = 1;
	proc.SP = MEM_SIZE - 1;
	proc.ESP = MEM_SIZE - 1;
	proc.STACK.push(Frame());
}
void run(Program prog) {
	initProc();

	for (int i = 0; i < prog.childs.size(); i++) {
		auto duo = prog.childs[i];
		auto kind = duo.first;
		auto df = duo.second;

		DefFunc* ptr1;
		DefClass* ptr2;
		DefInterface* ptr3;

		switch (kind) {
		case FUNC:
			ptr1 = (DefFunc*)df;
			runDefFunc(*ptr1);
			break;
		case CLASS:
			ptr2 = (DefClass*)df;
			runDefClass(*ptr2);
			break;
		case INTERFACE:
			ptr3 = (DefInterface*)df;
			runDefInterface(*ptr3);
		}
	}
}
void runDefFunc(DefFunc df) {

}
void runDefClass(DefClass df) {

}
void runDefInterface(DefInterface df) {

}
void runDefVar(DefVar df) {

}
bool isExists(const char* name) {
	auto& recent = proc.STACK.top().vars.back();
	for (int i = 0; i < recent.size(); i++) {
		auto duo = recent[i];
		auto vname = duo.first;
		if (strcmp(name, vname) == 0) {
			return true;
		}
	}

	return false;
}
Pointer getVar(const char* name) {
	for (int i = proc.STACK.top().vars.size(); i >= 0; i--) {
		auto& scope = proc.STACK.top().vars[i];
		for (int j = 0; j < scope.size(); j++) {
			auto duo = scope[j];
			auto vname = duo.first;
			auto vptr = duo.second;

			if (strcmp(name, vname) == 0) {
				return vptr;
			}
		}
	}

	return Pointer{ Type{}, false, 0 };
}
int getClass(const char* name) {
	for (int i = 0; i < proc.CLASS.size(); i++) {
		auto df = proc.CLASS[i];
		if (strcmp(df.name, name) == 0) {
			return i;
		}
	}

	return -1;
}
int getInterface(const char* name) {
	for (int i = 0; i < proc.INTERFACE.size(); i++) {
		auto df = proc.INTERFACE[i];
		if (strcmp(df.name, name) == 0) {
			return i;
		}
	}

	return -1;
}


#endif