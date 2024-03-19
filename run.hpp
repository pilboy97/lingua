#ifndef __RUN_HPP__
#define __RUN_HPP__

#include "RDP.hpp"

#include <vector>
#include <stack>
#include <algorithm>
#include <setjmp.h>
#include <string.h>

#define ll long long
#define ull unsigned long long

struct Pointer {
	Type type;
	bool lv;
	ll ptr;
};
struct Var {
	const char *name;
	Type type;
	ll ptr;
};
struct Frame {
	std::vector<std::vector<Var>> vars;
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
	DefClass *df;
	std::vector<std::pair<const char *, ll>> fields;
};
struct Closure {
	DefFunc fn;
	std::vector<Pointer> cap;
};
struct Proc {
	ll SP;
	ll ESP;
	Pointer RET;
	Type Rtype;
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

Pointer nil = {Type{0}, false, 0};
Var vnil = {"",Type{0}, 0};

Type tBool = Type{BOOL};
Type tNum = Type{NUM};
Type tUNum = Type{UNUM};
Type tByte = Type{BYTE};
Type tReal = Type{REAL};

bool isNull(Pointer ptr);

void run(Program prog);

ll hAlloc(Object o);
ll sAlloc(int size);
Object& hAccess(ll ptr);
ll& sAccess(ll ptr);

bool isSameType(Type x, Type y);
bool isAssignable(Type dst, Type src);
void assign(Pointer dst, Pointer src);
Type fnToType(DefFunc df);

void runStmt(Stmt stmt, jmp_buf jmp);
void runBlockStmt(BlockStmt stmt, jmp_buf jmp);
void runIfStmt(IfStmt stmt, jmp_buf jmp);
void runForStmt(ForStmt stmt, jmp_buf jmp);
void runRetStmt(RetStmt stmt, jmp_buf jmp);

void runDefClass(DefClass df);
void runDefInterface(DefInterface df);
void runDefFunc(DefFunc df);
void runDefVar(DefVar df);

Pointer runFCall(Pointer p, FCall arg);

void destroy(Var v);
void destroyArray(Array arr);
void destroyClosure(Closure cl);
void destroyInstance(Instance ins);
void clearFrame();

bool isExists(const char* name);
Pointer getVar(const char* name);
void newVar(Var dptr);
Var makeVar(const char* name, Pointer x);
Var makeDefault(const char *name, Type type);

Array makeArray(LiteralArray arr);

Pointer converse(Pointer x, Type type);

int getClass(const char* name);
int getInterface(const char* name);

void sStoreReal(ll ptr,double x);
double sLoadReal(ll ptr);

bool toBool(Pointer ptr);
char toByte(Pointer ptr);
ll toNum(Pointer ptr);
ull toUNum(Pointer ptr);
double toReal(Pointer ptr);
Array toArray(Pointer ptr);
Closure toClosure(Pointer ptr);
Instance toInstance(Pointer ptr);

Pointer runIdx(Pointer ptr, Idx idx);
Pointer runMember(Pointer ptr, Word word);
Pointer run(Pointer ptr, Idx idx);

ll cpObj(Object obj);

bool isPriType(Type x);

Array strToArray(LiteralString str);

void chkMem();
bool isValidPtr(int ptr);

Closure getFn(Pointer x);
Array getArray(Pointer x);
Str getStr(Pointer x);
Instance getInstance(Pointer x);

Closure makeClosure(DefFunc fn, Pointer self);

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

void chkMem() {
	if(!(0 < proc.SP && proc.SP < proc.mem.size())) {
		panic("SP error");
	}
	if(!(0 < (proc.ESP - 1) && (proc.ESP - 1) < proc.mem.size())) {
		panic("ESP error");
	}
	if(proc.SP >= proc.ESP) {
		panic("SP must be less than ESP");
	}
}
bool isValidPtr(ll ptr) {
	chkMem();
	if(ptr > 0) {
		return proc.SP < ptr && ptr < proc.ESP;
	}
	else if(ptr < 0) {
		ptr = -ptr;

		return 0 < ptr && ptr < proc.org.size();
	}

	return false;
}
bool isNull(Pointer ptr) {
	return ptr.ptr == 0;
}
void initProc() {
	proc.SP = 1;
	proc.ESP = 2;
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
	Closure *c = new Closure;
	*c = makeClosure(df, nil);

	Object o = Object{FUNC, (void*)c};
	int idx = proc.org.size();
	proc.org.push_back(o);

	int ptr = sAlloc(1);
	sAccess(ptr) = idx;

	newVar(Var{c->fn.name,fnToType(c->fn), ptr});
}
void runDefClass(DefClass df) {
	proc.CLASS.push_back(df);
	newVar(Var{"",nil.type, 0});
}
void runDefInterface(DefInterface df) {
	proc.INTERFACE.push_back(df);
	newVar(Var{"",nil.type, 0});
}
void runDefVar(DefVar df) {
	auto name = df.name;

	if(df.type == NULL && df.init == NULL) {
		panic("cannot decide type of variable");
	}

	if(df.type == NULL) {
		auto value = runExpr(*df.init);

		newVar(makeVar(name, value));
		return;
	}
	if(df.init == NULL) {
		auto type = *df.type;

		newVar(makeDefault(name, type));
	}

	auto type = *df.type;
	auto init = runExpr(*df.init);

	if(!isAssignable(type, init.type)) {
		panic("cannot assign");
	}

	Var v;
	v = makeDefault(name, type);

	Pointer src, dst;
	dst = Pointer {v.type, true, v.ptr};
	src = init;

	assign(dst, src);

	newVar(v);
}
bool isExists(const char* name) {
	auto& recent = proc.STACK.top().vars.back();
	for (int i = 0; i < recent.size(); i++) {
		auto v = recent[i];
		auto vname = v.name;
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
			auto v = scope[j];
			auto vname = v.name;

			if (strcmp(name, vname) == 0) {
				return Pointer {v.type, true, sAccess(v.ptr)};
			}
		}
	}

	return Pointer { Type{0}, false, 0 };
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
Closure makeClosure(DefFunc fn, Pointer self) {
	Closure ret;
	ret.fn = fn;
	
	ret.cap.push_back(self);
	for(int i = 0;i < fn.captured.size();i++) {
		Pointer ptr = getVar(fn.captured[i]);
		ret.cap.push_back(ptr);
	}

	return ret;
}
Var makeDefault(const char *name, Type type) {
	ll ptr = sAlloc(1);
	sAccess(ptr) = 0;
	return Var{name, type, ptr};
}

ll& sAccess(ll ptr) {
	if(!(proc.SP + 1 < ptr && ptr < proc.ESP)) panic("invalid pointer");

	return proc.mem[ptr];
}
Object& hAccess(ll ptr) {
	if(!(0 < ptr && ptr < proc.org.size())) panic("invalid pointer");

	return proc.org[ptr];
}
ll sAlloc(int size) {
	ll ret = proc.ESP;
	
	while(proc.ESP + size < proc.mem.size()) {
		proc.mem.push_back(0);
	}

	proc.ESP += size;

	return ret;
}
ll hAlloc(Object obj) {
	int ret = proc.org.size();
	proc.org.push_back(obj);

	return ret;
}
ll cpObj(Object obj) {
	Object neo;
	Array *ptr1;
	Instance *ptr2;
	Closure *ptr3;

	switch(obj.kind) {
	case OSB:
		ptr1 = new Array();
		*ptr1 = *(Array*)obj.ptr;
		neo = Object{OSB, (void*)ptr1};
		break;
	case WORD:
		ptr2 = new Instance();
		*ptr2 = *(Instance*)obj.ptr;
		neo = Object{OSB, (void*)ptr2};
		break;
	case FUNC:
		ptr3 = new Closure();
		*ptr3 = *(Closure*)obj.ptr;
		neo = Object{OSB, (void*)ptr3};
	}

	proc.org.push_back(obj);
	return proc.org.size() - 1;
}
bool isSameType(Type x, Type y) {
	if(x.kind != y.kind) return false;

	if(x.kind == OSB) {
		return isSameType(x.add[0], y.add[0]);
	}
	if(x.kind == FUNC) {
		if(x.add.size() != y.add.size()) return false;
		for(int i = 0;i < x.add.size();i++) {
			if(!isSameType(x.add[i], y.add[i]))
				return false;
		}
	}
	if(x.kind == WORD) { 
		return strcmp(x.name, y.name) == 0;
	}

	return true;
}
void assign(Pointer dst, Pointer src) {
	if(!dst.lv) panic("cannot assign: dst is not lvalue");
	if(!isAssignable(dst.type, src.type)) panic("cannot assign: type mismatch");

	sAccess(dst.ptr) = sAccess(src.ptr);
}
void newVar(Var var) {
	if(isExists(var.name)) panicf("cannot create var: %s is already exists", var.name);

	proc.STACK.top().vars.back().push_back(var);
}
Var makeVar(const char *name, Pointer ptr) {
	ll p = sAlloc(1);
	sAccess(p) = ptr.ptr;

	return Var{name, ptr.type, p};
}
Type fnToType(DefFunc fn) {
	Type ret;
	ret.kind = FUNC;
	ret.add.push_back(fn.ret);
	for(int i = 0;i < fn.frame.size();i++) {
		ret.add.push_back(fn.frame[i].second);
	}

	return ret;
}
bool isAssignable(Type dst, Type src) {
	return isSameType(dst, src);
}
bool isPriType(Type x) {
	return (x.kind == NUM || x.kind == UNUM || x.kind == BYTE || x.kind == BOOL);
}
void runStmt(Stmt stmt, jmp_buf jmp) {
	switch(stmt.kind) {
	case SCOLON:
		runExpr_1(*(Expr_1*)stmt.stmt);
		break;
	case IF:
		runIfStmt(*(IfStmt*)stmt.stmt, jmp);
		break;
	case FOR:
		runForStmt(*(ForStmt*)stmt.stmt, jmp);
		break;
	case VAR:
		runDefVar(*(DefVar*)stmt.stmt);
		break;
	case OBL:
		runBlockStmt(*(BlockStmt*)stmt.stmt, jmp);
		break;
	case RETURN:
		runRetStmt(*(RetStmt*)stmt.stmt, jmp);
		break;
	case CONTINUE:
		longjmp(jmp, 2);
		break;
	case BREAK:
		longjmp(jmp, 3);
		break;
	}
}
void runIfStmt(IfStmt stmt, jmp_buf jmp) {
	for(int i = 0;i < stmt._if.size();i++) {
		auto cond = runExpr(stmt._if[i].first);
		auto body = stmt._if[i].second;

		if(toBool(cond)) {
			runBlockStmt(body, jmp);
			return;
		}
	}
	if(stmt._else != NULL) {
		runBlockStmt(*stmt._else, jmp);
	}
}
void runBlockStmt(BlockStmt stmt, jmp_buf jmp) {
	for(int i = 0;i < stmt.childs.size();i++) {
		auto s = stmt.childs[i];

		runStmt(s, jmp);
	}
}
void runForStmt(ForStmt stmt, jmp_buf jmp) {
	if(stmt.init != NULL) {
		runExpr_1(*stmt.init);
	}

	while(true) {
		auto p = runExpr_1(stmt.cond);
		if(!toBool(p)) break;

		jmp_buf jp;
		int jv = setjmp(jp);

		if(jv == 0) {
			runBlockStmt(stmt.body, jp);
		}
		else if(jv == 1) {
			// return
			longjmp(jmp, 1);
		}
		else if(jv == 2) {
			// break
			break;
		}
		else {
			// continue
			continue;
		}
	}
}
void runRetStmt(RetStmt stmt, jmp_buf jmp) {
	proc.RET = runExpr(*stmt.expr);
	
	if(!isAssignable(proc.Rtype, proc.RET.type))
		panic("cannot return: type mismatch");

	longjmp(jmp, 1);
}
Pointer runFCall(Pointer fn, FCall args) {
	Closure c = toClosure(fn);
	Pointer ret;

	jmp_buf jp;
	int jv = setjmp(jp);

	if(jv == 0) {
		std::vector<Pointer> pargs;
		for(int i = 0;i < args.list.size();i++) {
			pargs.push_back(runExpr(args.list[i]));
		}

		proc.STACK.push(Frame());

		newVar(makeVar("", c.cap[0]));
		for(int i = 1;i < c.cap.size();i++) {
			auto ptr = c.cap[i];
			auto name = c.fn.captured[i];

			newVar(makeVar(name, ptr));
		}

		for(int i = 0;i < pargs.size();i++) {
			auto name = c.fn.frame[i].first;
			auto ptr = pargs[i];

			newVar(makeVar(name, ptr));
		}
	}
	else if(jv == 1) {
	}
	else if(jv == 2) {
		panic("unexpected break");
	}
	else {
		panic("unexpected continue");
	}

	return ret;
}
bool toBool(Pointer p) {
	if(p.type.kind != BOOL) {
		panic("toBool: it is not bool");
	}

	return (bool)sAccess(p.ptr);
}
char toByte(Pointer p) {
	if(p.type.kind != BYTE) {
		panic("toByte: it is not byte");
	}

	return (char)sAccess(p.ptr);
}
ll toNum(Pointer p) {
	if(p.type.kind != BOOL) {
		panic("toNum: it is not num");
	}

	return (ll)sAccess(p.ptr);
}
ull toUNum(Pointer p) {
	if(p.type.kind != BOOL) {
		panic("toUNum: it is not unum");
	}

	return (ull)sAccess(p.ptr);
}
double toReal(Pointer p) {
	if(p.type.kind != REAL) {
		panic("toUNum: it is not unum");
	}

	return (ull)sAccess(p.ptr);
}
Closure toClosure(Pointer p) {
	if(p.type.kind != FUNC) {
		panic("toClosure: it is not function");
	}

	int ptr = sAccess(p.ptr);

	return *(Closure*)hAccess(ptr).ptr;
}
Array toArray(Pointer p) {
	if(p.type.kind != FUNC) {
		panic("toArray: it is not array");
	}

	int ptr = sAccess(p.ptr);

	return *(Array*)hAccess(ptr).ptr;
}
Instance toInstance(Pointer p) {
	if(p.type.kind != WORD) {
		panic("toInstance: it is not object");
	}

	int ptr = sAccess(p.ptr);

	return *(Instance*)hAccess(ptr).ptr;
}
Pointer runExpr_1(Expr_1 e) {
	if(e.src == NULL) {
		return runExpr(e.dst);
	}

	auto src = runExpr(*e.src);
	auto dst = runExpr(e.dst);

	assign(dst, src);

	return nil;
}
Pointer runExpr(Expr e) {
	Pointer ret = runExpr1(e.childs[0]);
	if(e.childs.size() == 1) return ret;

	if(toBool(ret)) {
		sAccess(ret.ptr) = true;
		return ret;
	}

	for(int i = 1;i < e.childs.size();i++) {
		Pointer p = runExpr1(e.childs[i]);

		if(toBool(p)) {
			sAccess(ret.ptr) = true;
			return ret;
		}
	}

	sAccess(ret.ptr) = false;
	return ret;
}
Pointer runExpr1(Expr1 e) {
	Pointer ret = runExpr2(e.childs[0]);
	if(e.childs.size() == 1) return ret;

	if(!toBool(ret)) {
		return ret;
	}

	for(int i = 0;i < e.childs.size();i++) {
		auto p = runExpr2(e.childs[i]);
		if(!toBool(p)) {
			sAccess(ret.ptr) = false;
			return ret;
		}
	}

	sAccess(ret.ptr) = false;
	return ret;
}
Pointer runExpr2(Expr2 e) {
	Pointer ret = runExpr3(e.childs[0]);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto p = runExpr3(e.childs[i]);
		if(ret.type.kind == NUM && p.type.kind == NUM) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			c = a | b;

			sAccess(ret.ptr) = c;
		}
		else if(ret.type.kind == UNUM && p.type.kind == UNUM) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			c = a | b;

			sAccess(ret.ptr) = c;
		}
		else {
			panic("runExpr3: cannot calc: type mismatch");
		}
	}

	return ret;
}
Pointer runExpr3(Expr3 e) {
	Pointer ret = runExpr4(e.childs[0]);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto p = runExpr4(e.childs[i]);
		if(ret.type.kind == NUM && p.type.kind == NUM) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			c = a ^ b;

			sAccess(ret.ptr) = c;
		}
		else if(ret.type.kind == UNUM && p.type.kind == UNUM) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			c = a ^ b;

			sAccess(ret.ptr) = c;
		}
		else {
			panic("runExpr3: cannot calc: type mismatch");
		}
	}

	return ret;
}
Pointer runExpr4(Expr4 e) {
	Pointer ret = runExpr5(e.childs[0]);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto p = runExpr5(e.childs[i]);
		if(ret.type.kind == NUM && p.type.kind == NUM) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			c = a & b;

			sAccess(ret.ptr) = c;
		}
		else if(ret.type.kind == UNUM && p.type.kind == UNUM) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			c = a & b;

			sAccess(ret.ptr) = c;
		}
		else {
			panic("runExpr3: cannot calc: type mismatch");
		}
	}

	return ret;
}
Pointer runExpr5(Expr5 e) {
	Pointer ret = runExpr6(e.childs[0].second);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto duo = e.childs[i];
		auto kind = duo.first;
		auto exp = duo.second;

		auto p = runExpr6(exp);
		if(ret.type.kind == NUM && p.type.kind == NUM) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			
			if(kind == EQ)
				c = (a == b);
			else
				c = (a != b);

			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else if(ret.type.kind == UNUM && p.type.kind == UNUM) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			
			if(kind == EQ)
				c = (a == b);
			else
				c = (a != b);

			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else if(ret.type.kind == BYTE && ret.type.kind == BYTE) {
			char a,b,c;
			a = toByte(ret);
			b = toByte(p);

			if(kind == EQ) {
				c = (a == b);
			}
			else {
				c = (a != b);
			}

			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else if(ret.type.kind == REAL && ret.type.kind == REAL) {
			bool a,b,c;
			a = toReal(ret);
			b = toReal(p);

			if(kind == EQ) {
				c = (a == b);
			}
			else {
				c = (a != b);
			}
			
			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else if(ret.type.kind == BOOL && ret.type.kind == BOOL) {
			bool a,b,c;
			a = toBool(ret);
			b = toBool(p);

			if(kind == EQ) {
				c = (a == b);
			}
			else {
				c = (a != b);
			}
			
			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else {
			panic("runExpr3: cannot calc: type mismatch");
		}
	}

	return ret;
}
Pointer runExpr6(Expr6 e) {
	Pointer ret = runExpr7(e.childs[0].second);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto duo = e.childs[i];
		auto kind = duo.first;
		auto exp = duo.second;

		auto p = runExpr7(exp);
		if(ret.type.kind == NUM && p.type.kind == NUM) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			
			switch(kind) {
			case GR:
				c = (a > b);
				break;
			case LE:
				c = (a < b);
				break;
			case GEQ:
				c = (a >= b);
				break;
			case LEQ:
				c = (a <= b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else if(ret.type.kind == UNUM && p.type.kind == UNUM) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			
			switch(kind) {
			case GR:
				c = (a > b);
				break;
			case LE:
				c = (a < b);
				break;
			case GEQ:
				c = (a >= b);
				break;
			case LEQ:
				c = (a <= b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else if(ret.type.kind == BYTE && ret.type.kind == BYTE) {
			char a,b,c;
			a = toByte(ret);
			b = toByte(p);

			switch(kind) {
			case GR:
				c = (a > b);
				break;
			case LE:
				c = (a < b);
				break;
			case GEQ:
				c = (a >= b);
				break;
			case LEQ:
				c = (a <= b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else {
			panic("runExpr3: cannot calc: type mismatch");
		}
	}

	return ret;
}
Pointer runExpr7(Expr7 e) {
	Pointer ret = runExpr8(e.childs[0].second);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto duo = e.childs[i];
		auto kind = duo.first;
		auto exp = duo.second;

		auto p = runExpr8(exp);
		if(ret.type.kind == NUM && p.type.kind == NUM) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			
			switch(kind) {
			case LSH:
				c = (a << b);
				break;
			case RSH:
				c = (a >> b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else if(ret.type.kind == UNUM && p.type.kind == UNUM) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			
			switch(kind) {
			case LSH:
				c = (a << b);
				break;
			case RSH:
				c = (a >> b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else if(ret.type.kind == BYTE && ret.type.kind == BYTE) {
			char a,b,c;
			a = toByte(ret);
			b = toByte(p);
			
			switch(kind) {
			case LSH:
				c = (a << b);
				break;
			case RSH:
				c = (a >> b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else {
			panic("runExpr3: cannot calc: type mismatch");
		}
	}

	return ret;
}
Pointer runExpr8(Expr8 e) {
	Pointer ret = runExpr9(e.childs[0].second);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto duo = e.childs[i];
		auto kind = duo.first;
		auto exp = duo.second;

		auto p = runExpr9(exp);
		if(ret.type.kind == NUM && p.type.kind == NUM) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			
			switch(kind) {
			case ADD:
				c = (a + b);
				break;
			case SUB:
				c = (a - b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else if(ret.type.kind == UNUM && p.type.kind == UNUM) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			
			switch(kind) {
			case ADD:
				c = (a + b);
				break;
			case SUB:
				c = (a - b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else if(ret.type.kind == BYTE && ret.type.kind == BYTE) {
			char a,b,c;
			a = toByte(ret);
			b = toByte(p);
			
			switch(kind) {
			case ADD:
				c = (a + b);
				break;
			case SUB:
				c = (a - b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else {
			panic("runExpr3: cannot calc: type mismatch");
		}
	}

	return ret;
}
Pointer runExpr9(Expr9 e) {
	Pointer ret = runExpr10(e.childs[0].second);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto duo = e.childs[i];
		auto kind = duo.first;
		auto exp = duo.second;

		auto p = runExpr10(exp);
		if(ret.type.kind == NUM && p.type.kind == NUM) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			
			switch(kind) {
			case MUL:
				c = (a * b);
				break;
			case DIV:
				c = (a / b);
				break;
			case MOD:
				c = (a % b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else if(ret.type.kind == UNUM && p.type.kind == UNUM) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			
			switch(kind) {
			case MUL:
				c = (a * b);
				break;
			case DIV:
				c = (a / b);
				break;
			case MOD:
				c = (a % b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else if(ret.type.kind == BYTE && ret.type.kind == BYTE) {
			char a,b,c;
			a = toByte(ret);
			b = toByte(p);
			
			switch(kind) {
			case MUL:
				c = (a * b);
				break;
			case DIV:
				c = (a / b);
				break;
			case MOD:
				c = (a % b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else {
			panic("runExpr3: cannot calc: type mismatch");
		}
	}

	return ret;
}
Pointer converse(Pointer x, Type type) {
	Pointer ret;
	if(isPriType(x.type) && isPriType(type)) {
		long long value = sAccess(x.ptr);
		ret.ptr = sAlloc(1);
		ret.type = type;
		switch (type.kind)
		{
		case BOOL:
			sAccess(ret.ptr) = value & 1;
			break;
		case BYTE:
			sAccess(ret.ptr) = value & 0xFF;
			break;
		case NUM:
			sAccess(ret.ptr) = value;
			break;
		case UNUM:
			sAccess(ret.ptr) = value;
			break;
		default:
			break;
		}

		return ret;
	}

	if(isSameType(ret.type, type)) {
		return ret;
	}

	return nil;
}
Pointer runExpr10(Expr10 e) {
	Pointer ret = runExpr11(e.child);
	if(e.oper.size() == 0) return ret;

	for(int i = 1;i < e.oper.size();i++) {
		auto duo = e.oper[i];
		auto kind = duo.first;
		auto type = duo.second;

		switch (kind) {
		case NIL:
			if(ret.ptr == 0) {
				ret.ptr = sAlloc(1);
				ret.type = tBool;
				sAccess(ret.ptr) = true;
			}
			else {
				ret.type = tBool;
				sAccess(ret.ptr) = false;
			}
			return ret;
		case LNOT:
			if(ret.ptr == 0) {
				ret.ptr = sAlloc(1);
				ret.type = tBool;
				sAccess(ret.ptr) = false;
			}
			else {
				ret.type = tBool;
				sAccess(ret.ptr) = true;
			}
			return ret;
		case AS:
			ret = converse(ret, type);
			break;
		case IS:
			if(ret.ptr == 0) {
				ret.ptr = sAlloc(1);
				sAccess(ret.ptr) = false;
			}
			else if(isAssignable(type, ret.type)) {
				ret.type = tBool;
				sAccess(ret.ptr) = true;
			}
			else {
				ret.type = tBool;
				sAccess(ret.ptr) = false;
			}
			return ret;
		default:
			panic("wrong operator");
		}
	}

	return ret;
}
Pointer runExpr11(Expr11 e) {
	Pointer ret = runExpr12(e.child);
	if(e.oper.size() == 0) return ret;

	for(int i = 1;i < e.oper.size();i++) {
		auto kind = e.oper[i];

		switch (kind) {
			case NOT:
				if(ret.type.kind == NUM  || ret.type.kind == UNUM || ret.type.kind == BYTE) {
					sAccess(ret.ptr) = ~sAccess(ret.ptr);
				}
				else panic("cannot execute ~ operator");
			case LNOT:
				if(ret.type.kind != BOOL) {
					panic("cannot execute not operator");
				}
				sAccess(ret.ptr) = !sAccess(ret.ptr);
			case SUB:
				if(ret.type.kind == NUM || ret.type.kind == BYTE) {
					sAccess(ret.ptr) = -sAccess(ret.ptr);
				}
				else {
					panic("cannot execute negative operator");
				}
				break;
			default:
				panic("wrong operator");
		}
	}

	return ret;
}
Pointer runExpr12(Expr12 e) {
	Pointer ret = runFactor(e.f);
	if(e.childs.size() == 0) return ret;

	for(int i = 0;i < e.childs.size();i++) {
		auto duo = e.childs[i];
		auto kind = duo.first;
		auto ptr = duo.second;

		Closure c;
		Array arr;
		Instance in;

		std::vector<Expr> e;
		FCall *ptr1;
		Idx *ptr2;
		Word *ptr3;

		int j;

		switch(kind) {
		case OBR:
			if(ret.type.kind != FUNC) 
				panic("cannot call function: it is not function");
			ptr1 = (FCall*)ptr;
			runFCall(ret, *ptr1);
			break;
		case OSB:
			if(ret.type.kind != OSB) 
				panic("it is not array");
			ptr2 = (Idx*)ptr;
			runIdx(ret, *ptr2);
			break;
		case DOT:
			if(ret.type.kind != WORD) {
				panic("it is not instance");
			}
			ptr3 = (Word*)ptr;
		default:
			panic("wrong operator");
		}
	}
	return ret;
}
Pointer runFactor(Factor e) {
	Pointer ret;
	Array *arr;
	Var v;
	switch(e.kind) {
	case OBR:
		ret = runExpr(*(Expr*)e.ptr);
		break;
	case LNUM:
		ret.type = tNum;
		ret.ptr = sAlloc(1);
		sAccess(ret.ptr) = ((Number*)e.ptr)->value;
		break;
	case LUNUM:
		ret.type = tUNum;
		ret.ptr = sAlloc(1);
		sAccess(ret.ptr) = ((Number*)e.ptr)->value;
		break;
	case LBYTE:
		ret.type = tByte;
		ret.ptr = sAlloc(1);
		sAccess(ret.ptr) = ((Number*)e.ptr)->value;
		break;
	case LSTR:
		arr = new Array();
		*arr = strToArray(*(LiteralString*)e.ptr);
		proc.org.push_back(Object{OSB, (void*)arr});
		
		ret.type.kind = OSB;
		ret.type.add.push_back(tByte);
		break;
	case LREAL:
		ret.type = tReal;
		ret.ptr = sAlloc(1);
		sStoreReal(ret.ptr, (*(Real*)e.ptr).value);
		break;
	case WORD:
		ret = getVar((*(Word*)e.ptr).word);
	case OSB:
		
	case LTRUE:

	case LFALSE:
	
	case FUNC:
	default:
		break;
	}
	return nil;
}
Pointer runMember(Pointer ptr, Word word) {
	Instance inst = toInstance(ptr);
	const char *member = word.word;

	for(int i = 0;i < inst.df->field.size();i++) {
		if(strcmp(inst.df->field[i].name, member) == 0) {
			Pointer ret;
			ret.ptr = sAlloc(1);
			sAccess(ret.ptr) = inst.fields[i].second;

			return ret;
		}
	}

	panic("cannot find member");
}
Pointer runIdx(Pointer ptr, Idx idx) {
	if(ptr.type.kind != OSB)
		panic("runIdx: it is not array");

	Array arr = toArray(ptr);
	Type type = arr.etype;
	Pointer id = runExpr(idx.id);
	if(isAssignable(tNum, ptr.type)) {
		panic("index of array must be able to converse int");
	}

	id = converse(id, tNum);

	int i = sAccess(id.ptr);
	return Pointer{type,true,arr.array[i]};
}
#endif