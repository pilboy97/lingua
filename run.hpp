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
	Type rtype;
	Pointer RET;
	std::vector<std::vector<Var>> vars;
};
struct Object {
	int cnt;
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
struct NFunc {
	const char* name;
	Type rtype;
	std::vector<Type> frame;
};
struct Closure {
	NFunc* nf;
	DefFunc fn;
	std::vector<Pointer> cap;
};
struct Proc {
	ll SP;
	ll ESP;
	ll GP;
	std::vector<Var> Global;
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
Type tStr = Type{ OSB,"",{tByte} };

bool isNull(Pointer ptr);

void run(Program prog);

ll hAlloc(Object o);
ll sAlloc(int size);
Object& hAccess(ll ptr);
ll& sAccess(ll ptr);
ll& gAccess(ll ptr);

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

void point(Pointer ptr);
void destroy(Pointer v);
void destroyArray(Array arr);
void destroyClosure(Closure cl);
void destroyInstance(Instance ins);
void clearFrame();

bool isExists(const char* name);
bool isGExists(const char* name);
Pointer getVar(const char* name);
void newVar(Var dptr);
void newGVar(Var dptr);
Var makeVar(const char* name, Pointer x);
Var makeDefault(const char *name, Type type);

Array makeArray(LiteralArray arr);

Pointer converse(Pointer x, Type type);

int getClass(const char* name);
int getInterface(const char* name);

void sStoreReal(ll ptr,double x);

bool toBool(Pointer ptr);
char toByte(Pointer ptr);
ll toNum(Pointer ptr);
ull toUNum(Pointer ptr);
double toReal(Pointer ptr);
Array toArray(Pointer ptr);
Closure toClosure(Pointer ptr);
Instance toInstance(Pointer ptr);

bool isNum(Type x);
bool isUNum(Type x);
bool isByte(Type x);
bool isBool(Type x);
bool isReal(Type x);
bool isStr(Type x);

Pointer runIdx(Pointer ptr, Idx idx);
Pointer runMember(Pointer ptr, Word word);

Pointer runNFunc(NFunc fn, FCall args);

ll cpObj(Object obj);

bool isPriType(Type x);

Array strToArray(LiteralString str);
const char* arrayToStr(Array arr);

const char* strAdd(const char* a, const char* b);
const char* strAdd(const char* a, ll b);
const char* strAdd(const char* a, ull b);
const char* strAdd(const char* a, char b);
const char* strAdd(const char* a, double b);
const char* strAdd(const char* a, bool b);

void chkMem();

Closure makeClosure(DefFunc fn, Pointer self);

void enterScope();
void exitScope();

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

Pointer runNFunc(NFunc fn, FCall args) {
	if (strcmp(fn.name, "println") == 0) {
		if (args.list.size() != 1) {
			panic("cannot call println: argument mismatch");
		}

		Array arr = toArray(runExpr(args.list[0]));
		if (arr.etype.kind != BYTE) {
			panic("cannot call println: argument is not []byte");
		}

		for (int i = 0; i < arr.array.size(); i++) {
			putchar(arr.array[i]);
		}
		puts("");

		return nil;
	}
	if (strcmp(fn.name, "print") == 0) {
		if (args.list.size() != 1) {
			panic("cannot call print: argument mismatch");
		}

		Array arr = toArray(runExpr(args.list[0]));
		if (arr.etype.kind != BYTE) {
			panic("cannot call print: argument is not []byte");
		}

		for (int i = 0; i < arr.array.size(); i++) {
			putchar(arr.array[i]);
		}

		return nil;
	}

	panic("unknown nested function");
}
void point(Pointer ptr) {
	if (isPriType(ptr.type)) return;
	Object &obj = hAccess(sAccess(sAccess(ptr.ptr)));
	obj.cnt++;
}
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
	proc.STACK.top().vars.emplace_back();

	proc.org.push_back(Object{ 0, NULL });

	proc.mem.push_back(0);
	proc.mem.push_back(0);

	NFunc* println = new NFunc{ "println", Type{0}, {Type{OSB,"",{tByte}}} };
	newGVar(makeVar("println", 
		Pointer{
			Type{
				FUNC,
				"",
				{
				Type{0},
				Type{OSB,"",{tByte}}
				}
			}, 
			false,
			hAlloc(Object{1, 0, println})
		}
	));
	NFunc* print = new NFunc{ "print", Type{0}, {Type{OSB,"",{tByte}}} };
	newGVar(makeVar("print",
		Pointer{
			Type{
				FUNC,
				"",
				{
				Type{0},
				Type{OSB,"",{tByte}}
				}
			},
			false,
			hAlloc(Object{1, 0, print})
		}
	));

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

	proc.GP = proc.ESP;
	runFCall(getVar("main"), FCall{});
}
void runDefFunc(DefFunc df) {
	Closure *c = new Closure;
	*c = makeClosure(df, nil);

	Object o = Object{1, FUNC, (void*)c};

	int ptr = hAlloc(o);

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
		return;
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
bool isGExists(const char* name) {
	auto& global = proc.Global;
	for (int i = 0; i < global.size(); i++) {
		auto v = global[i];
		auto vname = v.name;
		if (strcmp(name, vname) == 0) {
			return true;
		}
	}

	return false;
}
Pointer getVar(const char* name) {
	for (int i = proc.STACK.top().vars.size() - 1; i >= 0; i--) {
		auto& scope = proc.STACK.top().vars[i];
		for (int j = 0; j < scope.size(); j++) {
			auto v = scope[j];
			auto vname = v.name;

			if (strcmp(name, vname) == 0) {
				return Pointer {v.type, true, v.ptr};
			}
		}
	}
	
	for (int i = 0; i < proc.Global.size(); i++) {
		auto v = proc.Global[i];
		if (strcmp(v.name, name) == 0) {
			return Pointer{ v.type, true, gAccess(v.ptr) };
		}
	}

	panicf("cannot find %s", name);
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
	ret.nf = NULL;
	
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
	if(!
		(
			(proc.SP < ptr && ptr < proc.ESP)||
			(0<ptr&&ptr<proc.GP)
		)
	) panic("invalid pointer");

	return proc.mem[ptr];
}
ll& gAccess(ll ptr) {
	if (!(0 < ptr && ptr < proc.GP)) panic("invalid pointer");

	return proc.mem[ptr];
}
Object& hAccess(ll ptr) {
	ptr = -ptr;
	if(!(0 < ptr && ptr < proc.org.size())) panic("invalid pointer");

	return proc.org[ptr];
}
ll sAlloc(int size) {
	ll ret = proc.ESP;
	
	while(proc.ESP + size > proc.mem.size()) {
		proc.mem.push_back(0);
	}

	proc.ESP += size;

	return ret;
}
ll hAlloc(Object obj) {
	int ret = proc.org.size();
	proc.org.push_back(obj);

	int p = sAlloc(1);
	sAccess(p) = -ret;
	int p2 = sAlloc(1);
	sAccess(p2) = p;
	return p2;
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
		neo = Object{1, OSB, (void*)ptr1};
		break;
	case WORD:
		ptr2 = new Instance();
		*ptr2 = *(Instance*)obj.ptr;
		neo = Object{1, OSB, (void*)ptr2};
		break;
	case FUNC:
		ptr3 = new Closure();
		*ptr3 = *(Closure*)obj.ptr;
		neo = Object{1, OSB, (void*)ptr3};
	}

	return hAlloc(neo);
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
	if(strlen(var.name) > 0 && isExists(var.name)) panicf("cannot create var: %s is already exists", var.name);

	proc.STACK.top().vars.back().push_back(var);
}
void newGVar(Var var) {
	if (strlen(var.name) > 0 && isGExists(var.name)) panicf("cannot create global var: %s is already exists", var.name);

	proc.Global.push_back(var);
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
bool isStr(Type x) {
	if (x.kind != OSB) return false;
	if (x.add.size() != 1) return false;
	return x.add[0].kind == BYTE;
}
bool isNum(Type x) {
	return x.kind == NUM;
}
bool isUNum(Type x) {
	return x.kind == UNUM;
}
bool isReal(Type x) {
	return x.kind == REAL;
}
bool isByte(Type x) {
	return x.kind == BYTE;
}
bool isBool(Type x) {
	return x.kind == BOOL;
}
bool isPriType(Type x) {
	return (x.kind == 0 || x.kind == NUM || x.kind == UNUM || x.kind == BYTE || x.kind == BOOL);
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
	enterScope();
	for(int i = 0;i < stmt.childs.size();i++) {
		auto s = stmt.childs[i];

		jmp_buf aj;
		int av = setjmp(aj);
		if(av == 0)
			runStmt(s, jmp);
		else {
			exitScope();
			longjmp(jmp, av);
		}
	}
	exitScope();
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
	proc.STACK.top().RET = runExpr(*stmt.expr);
	
	if(!isAssignable(proc.STACK.top().rtype, proc.STACK.top().RET.type))
		panic("cannot return: type mismatch");

	longjmp(jmp, 1);
}
Pointer runFCall(Pointer fn, FCall args) {
	Closure c = toClosure(fn);

	if (c.nf != NULL) {
		return runNFunc(*c.nf, args);
	}

	Pointer ret;

	jmp_buf jp;
	int jv = setjmp(jp);

	if(jv == 0) {
		std::vector<Pointer> pargs;
		for(int i = 0;i < args.list.size();i++) {
			pargs.push_back(runExpr(args.list[i]));
		}

		proc.STACK.push(Frame());
		proc.STACK.top().vars.emplace_back();

		for (int i = 1; i < c.cap.size(); i++) {
			auto ptr = c.cap[i];
			point(ptr);
		}

		for (int i = 0; i < pargs.size(); i++) {
			auto ptr = pargs[i];
			point(ptr);
		}

		int ptr = sAlloc(1);
		sAccess(ptr) = proc.SP;
		proc.SP = proc.ESP - 1;

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


		runBlockStmt(c.fn.body, jp);
		return nil;
	}
	else if(jv == 1) {
		ret = proc.STACK.top().RET;
		clearFrame();
		proc.STACK.pop();

		if (proc.mem[proc.SP] != 0) {
			proc.SP = proc.mem[proc.SP];
		}
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
	if(p.type.kind != NUM) {
		panic("toNum: it is not num");
	}

	return (ll)sAccess(p.ptr);
}
ull toUNum(Pointer p) {
	if(p.type.kind != UNUM) {
		panic("toUNum: it is not unum");
	}

	return (ull)sAccess(p.ptr);
}
double toReal(Pointer p) {
	if(p.type.kind != REAL) {
		panic("toUNum: it is not unum");
	}

	return *(double*)((void*) & sAccess(p.ptr));
}
Closure toClosure(Pointer p) {
	if(p.type.kind != FUNC) {
		panic("toClosure: it is not function");
	}

	int ptr = sAccess(sAccess(p.ptr));
	Object obj = hAccess(ptr);
	if (obj.kind == 0) {
		Closure ret;
		NFunc *nf = (NFunc*)obj.ptr;

		std::vector<std::pair<const char*, Type>> args;
		for (int i = 0; i < nf->frame.size(); i++)
			args.emplace_back("", nf->frame[i]);

		ret.nf = nf;
		ret.fn.ret = nf->rtype;
		ret.fn.frame = args;
		ret.cap.push_back(nil);
		return ret;
	}

	Closure *cl = (Closure*)(obj.ptr);
	return *cl;
}
Array toArray(Pointer p) {
	if(p.type.kind != OSB) {
		panic("toArray: it is not array");
	}

	int ptr = sAccess(sAccess(p.ptr));

	return *(Array*)hAccess(ptr).ptr;
}
Instance toInstance(Pointer p) {
	if(p.type.kind != WORD) {
		panic("toInstance: it is not object");
	}

	int ptr = sAccess(sAccess(p.ptr));

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

	ret.type = tBool;
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

	ret.type = tBool;
	sAccess(ret.ptr) = false;
	return ret;
}
Pointer runExpr2(Expr2 e) {
	Pointer ret = runExpr3(e.childs[0]);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto p = runExpr3(e.childs[i]);
		if(isNum(ret.type) && isNum(p.type)) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			c = a | b;

			ret.type = tNum;
			sAccess(ret.ptr) = c;
		}
		else if(isUNum(ret.type) && isUNum(p.type)) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			c = a | b;

			ret.type = tUNum;
			sAccess(ret.ptr) = c;
		}
		else if (isByte(ret.type) && isByte(p.type)) {
			char a, b, c;
			a = toByte(ret);
			b = toByte(p);
			c = a | b;

			ret.type = tByte;
			sAccess(ret.ptr) = c;
		}
		else {
			panic("runExpr2: cannot calc: type mismatch");
		}
	}

	return ret;
}
Pointer runExpr3(Expr3 e) {
	Pointer ret = runExpr4(e.childs[0]);
	if(e.childs.size() == 1) return ret;

	for(int i = 1;i < e.childs.size();i++) {
		auto p = runExpr4(e.childs[i]);
		if(isNum(ret.type) && isNum(p.type)) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			c = a ^ b;

			ret.type = tNum;
			sAccess(ret.ptr) = c;
		}
		else if(isUNum(ret.type) && isUNum(p.type)) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			c = a ^ b;

			ret.type = tUNum;
			sAccess(ret.ptr) = c;
		}
		else if (isByte(ret.type) && isByte(p.type)) {
			char a, b, c;
			a = toByte(ret);
			b = toByte(p);
			c = a ^ b;

			ret.type = tByte;
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
		if(isNum(ret.type) && isNum(p.type)) {
			ll a,b,c;
			a = toNum(ret);
			b = toNum(p);
			c = a & b;

			ret.type = tNum;
			sAccess(ret.ptr) = c;
		}
		else if(isUNum(ret.type) && isUNum(p.type)) {
			ull a,b,c;
			a = toUNum(ret);
			b = toUNum(p);
			c = a & b;

			ret.type = tUNum;
			sAccess(ret.ptr) = c;
		}
		else if (isByte(ret.type) && isByte(p.type)) {
			char a, b, c;
			a = toByte(ret);
			b = toByte(p);
			c = a & b;

			ret.type = tByte;
			sAccess(ret.ptr) = c;
		}
		else {
			panic("runExpr4: cannot calc: type mismatch");
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
		if(isNum(ret.type) && isNum(p.type)) {
			ll a, b;
			bool c;
			a = toNum(ret);
			b = toNum(p);
			
			if(kind == EQ)
				c = (a == b);
			else
				c = (a != b);

			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else if(isUNum(ret.type) && isUNum(p.type)) {
			ull a, b;
			bool c;
			a = toUNum(ret);
			b = toUNum(p);
			
			if(kind == EQ)
				c = (a == b);
			else
				c = (a != b);

			sAccess(ret.ptr) = c;
			ret.type = tBool;
		}
		else if(isByte(ret.type) && isByte(p.type)) {
			char a, b;
			bool c;
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
		else if(isReal(ret.type) && isReal(p.type)) {
			double a, b;
			bool c;

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
		else if(isBool(ret.type) && isBool(p.type)) {
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
			panic("runExpr5: cannot calc: type mismatch");
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
		if(isNum(ret.type) && isNum(p.type)) {
			ll a, b;
			bool c;
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
		else if(isUNum(ret.type) && isUNum(p.type)) {
			ull a, b;
			bool c;
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
		else if(isByte(ret.type) && isByte(p.type)) {
			char a, b;
			bool c;
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
		else if (isReal(ret.type) && isReal(p.type)) {
			double a, b;
			bool c;
			a = toReal(ret);
			b = toReal(p);

			switch (kind) {
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
			panic("runExpr6: cannot calc: type mismatch");
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
		if(isNum(ret.type) && isNum(p.type)) {
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
			ret.type = tNum;
		}
		else if(isUNum(ret.type) && isUNum(p.type)) {
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
			ret.type = tUNum;
		}
		else if(isByte(ret.type) && isByte(p.type)) {
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
			ret.type = tByte;
		}
		else {
			panic("runExpr7: cannot calc: type mismatch");
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
		if(isNum(ret.type) && isNum(p.type)) {
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

			ret.type = tNum;
			sAccess(ret.ptr) = c;
		}
		else if(isUNum(ret.type) && isUNum(p.type)) {
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

			ret.type = tUNum;
			sAccess(ret.ptr) = c;
		}
		else if(isByte(ret.type) && isByte(p.type)) {
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

			ret.type = tByte;
			sAccess(ret.ptr) = c;
		}
		else if (isReal(ret.type) && isReal(p.type)) {
			double a, b, c;
			a = toReal(ret);
			b = toReal(p);

			switch (kind) {
			case ADD:
				c = (a + b);
				break;
			case SUB:
				c = (a - b);
				break;
			default:
				panic("wrong operator");
			}

			ret.type = tReal;
			sStoreReal(ret.ptr, c);
		}
		else if (isStr(ret.type) && isStr(p.type)) {
			const char* a, * b, * c;
			a = arrayToStr(toArray(ret));
			b = arrayToStr(toArray(p));
			c = NULL;

			switch (kind) {
			case ADD:
				c = strAdd(a, b);
				break;
			default:
				panic("wrong operator");
			}

			Array* arr = new Array();
			*arr = strToArray(LiteralString{ c });
			ret.type = tStr;
			ret.ptr = hAlloc(Object{ 1, OSB, (void*)arr });
		}
		else if (isStr(ret.type) && isNum(p.type)) {
			const char* a, * c;
			ll b;

			a = arrayToStr(toArray(ret));
			b = toNum(p);
			c = NULL;

			switch (kind) {
			case ADD:
				c = strAdd(a, b);
				break;
			default:
				panic("wrong operator");
			}

			Array* arr = new Array();
			*arr = strToArray(LiteralString{ c });
			ret.type = tStr;
			ret.ptr = hAlloc(Object{ 1, OSB, (void*)arr });
		}
		else if (isStr(ret.type) && isStr(p.type)) {
			const char* a, * b, * c;
			a = arrayToStr(toArray(ret));
			b = arrayToStr(toArray(p));
			c = NULL;

			switch (kind) {
			case ADD:
				c = strAdd(a, b);
				break;
			default:
				panic("wrong operator");
			}

			Array* arr = new Array();
			*arr = strToArray(LiteralString{ c });
			ret.type = tStr;
			ret.ptr = hAlloc(Object{ 1, OSB, (void*)arr });
			}
		else if (isStr(ret.type) && isUNum(p.type)) {
			const char* a, * c;
			ull b;
			a = arrayToStr(toArray(ret));
			b = toUNum(p);
			c = NULL;

			switch (kind) {
			case ADD:
				c = strAdd(a, b);
				break;
			default:
				panic("wrong operator");
			}

			Array* arr = new Array();
			*arr = strToArray(LiteralString{ c });
			ret.type = tStr;
			ret.ptr = hAlloc(Object{ 1, OSB, (void*)arr });
			}
		else if (isStr(ret.type) && isByte(p.type)) {
			const char* a, * c;
			char b;
			a = arrayToStr(toArray(ret));
			b = toByte(p);
			c = NULL;

			switch (kind) {
			case ADD:
				c = strAdd(a, b);
				break;
			default:
				panic("wrong operator");
			}

			Array* arr = new Array();
			*arr = strToArray(LiteralString{ c });
			ret.type = tStr;
			ret.ptr = hAlloc(Object{ 1, OSB, (void*)arr });
		}
		else if (isStr(ret.type) && isBool(p.type)) {
			const char* a, * c;
			bool b;
			a = arrayToStr(toArray(ret));
			b = toBool(p);
			c = NULL;

			switch (kind) {
			case ADD:
				c = strAdd(a, b);
				break;
			default:
				panic("wrong operator");
			}

			Array* arr = new Array();
			*arr = strToArray(LiteralString{ c });
			ret.type = tStr;
			ret.ptr = hAlloc(Object{ 1, OSB, (void*)arr });
		}
		else if (isStr(ret.type) && isReal(p.type)) {
			const char* a, * c;
			double b;
			a = arrayToStr(toArray(ret));
			b = toReal(p);
			c = NULL;

			switch (kind) {
			case ADD:
				c = strAdd(a, b);
				break;
			default:
				panic("wrong operator");
			}

			Array* arr = new Array();
			*arr = strToArray(LiteralString{ c });
			ret.type = tStr;
			ret.ptr = hAlloc(Object{ 1, OSB, (void*)arr });
		}
		else {
			panic("runExpr8: cannot calc: type mismatch");
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
		if(isNum(ret.type) && isNum(p.type)) {
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
		else if(isUNum(ret.type) && isUNum(p.type)) {
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
		else if(isByte(ret.type) && isByte(p.type)) {
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
		else if (isReal(ret.type) && isReal(p.type)) {
			double a, b, c;
			a = toReal(ret);
			b = toReal(p);

			switch (kind) {
			case MUL:
				c = (a * b);
				break;
			case DIV:
				c = (a / b);
				break;
			default:
				panic("wrong operator");
			}

			sAccess(ret.ptr) = c;
		}
		else {
			panic("runExpr9: cannot calc: type mismatch");
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
				if(isNum(ret.type)  || isUNum(ret.type) || isByte(ret.type)) {
					sAccess(ret.ptr) = ~sAccess(ret.ptr);
				}
				else panic("cannot execute ~ operator");
			case LNOT:
				if(!isBool(ret.type)) {
					panic("cannot execute not operator");
				}
				sAccess(ret.ptr) = (sAccess(ret.ptr) == 0) ? 1 : 0;
			case SUB:
				if(isNum(ret.type) || isByte(ret.type)) {
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
	Closure *cl;

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
		
		ret.type.kind = OSB;
		ret.type.add.push_back(tByte);
		ret.ptr = hAlloc(Object{1, OSB, (void*)arr});
		break;
	case LREAL:
		ret.type = tReal;
		ret.ptr = sAlloc(1);
		sStoreReal(ret.ptr, (*(Real*)e.ptr).value);
		break;
	case WORD:
		ret = getVar((*(Word*)e.ptr).word);
		ret.lv = true;
		break;
	case OSB:
		arr = new Array();
		*arr = makeArray(*(LiteralArray*)e.ptr);
		ret.ptr = hAlloc(Object{ 1, OSB, arr });
		ret.type = Type{ OSB, "", {arr->etype} };
		sAccess(ret.ptr) = proc.org.size() - 1;
		break;
	case LTRUE:
		ret.type = tBool;
		ret.ptr = sAlloc(1);
		sAccess(ret.ptr) = true;
		break;
	case LFALSE:
		ret.type = tBool;
		ret.ptr = sAlloc(1);
		sAccess(ret.ptr) = false;
		break;
	case FUNC:
		cl = new Closure();
		*cl = makeClosure(*(DefFunc*)e.ptr, nil);
		ret.type = fnToType(*(DefFunc*)e.ptr);
		ret.ptr = hAlloc(Object{ 1, FUNC, cl });
		break;
	default:
		panic("unknwon factor");
		break;
	}
	return ret;
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
void clearFrame() {
	auto& f = proc.STACK.top();
	for (int i = f.vars.size() - 1; i >= 0; i--) {
		auto& list = f.vars[i];

		for (int j = list.size() - 1; j >= 0; j--) {
			auto& var = list[j];
		
			destroy(Pointer{var.type, false, sAccess(var.ptr)});
		}
	}
}
void enterScope() {
	proc.STACK.push(Frame());
	proc.STACK.top().vars.emplace_back();
}
void exitScope() {
	auto list = proc.STACK.top().vars.back();
	for (int i = list.size() - 1; i >= 0; i--) {
		auto var = list[i];

		destroy(Pointer{ var.type, false, var.ptr });
	}
	proc.STACK.top().vars.pop_back();
}
Array strToArray(LiteralString lstr) {
	int len = strlen(lstr.str);
	Array ret;

	ret.etype = tByte;

	for (int i = 0; i < len; i++) {
		ret.array.push_back(lstr.str[i]);
	}

	return ret;
}
const char* strAdd(const char* a, const char* b) {
	int alen, blen;
	alen = strlen(a);
	blen = strlen(b);

	int cnt = 0;
	char* ret = (char*)malloc(sizeof(char) * (alen + blen + 1));
	for (int i = 0; i < alen; i++) ret[cnt++] = a[i];
	for (int i = 0; i < blen; i++) ret[cnt++] = b[i];
	ret[cnt++] = '\0';

	return ret;
}
const char* strAdd(const char* a, ll b) {
	char buf[32];
	sprintf(buf, "%lld", b);

	return strAdd(a, buf);
}
const char* strAdd(const char* a, ull b) {
	char buf[32];
	sprintf(buf, "%llu", b);

	return strAdd(a, buf);
}
const char* strAdd(const char* a, char b) {
	char buf[32];
	sprintf(buf, "%d", b);

	return strAdd(a, buf);
}
const char* strAdd(const char* a, bool b) {
	char buf[32];
	sprintf(buf, "%s", (b==0)?"false":"true");

	return strAdd(a, buf);
}
const char* strAdd(const char* a, double b) {
	char buf[32];
	sprintf(buf, "%f", b);

	return strAdd(a, buf);
}
void sStoreReal(ll ptr, double val) {
	ll v = *(ll*)((void*)&val);
	sAccess(ptr) = v;
}
Array makeArray(LiteralArray arr) {
	Array ret;
	ret.etype = arr.type;
	for (int i = 0; i < arr.elem.size(); i++) {
		ret.array.push_back(sAccess(runExpr(arr.elem[i]).ptr));
	}
	return ret;
}
void destroy(Pointer ptr) {
	if (isPriType(ptr.type)) return;

	Object& obj = hAccess(sAccess(sAccess(ptr.ptr)));
	obj.cnt--;

	switch (ptr.type.kind) {
	case OSB:
		destroyArray(toArray(ptr));
		break;
	case FUNC:
		destroyClosure(toClosure(ptr));
		break;
	case WORD:
		destroyInstance(toInstance(ptr));
	}
}
void destroyArray(Array arr) {
	if (isPriType(arr.etype)) return;

	for (int i = 0; i < arr.array.size(); i++) {
		destroy(Pointer{ arr.etype, false,arr.array[i] });
	}
}
void destroyClosure(Closure cl) {
	for (int i = 0; i < cl.cap.size(); i++) {
		destroy(cl.cap[i]);
	}
}
void destroyInstance(Instance in) {
	for (int i = 0; i < in.fields.size(); i++) {
		auto type = in.df->field[i].type;
		auto ptr = in.fields[i].second;

		destroy(Pointer{ type, false, ptr });
	}
}
const char* arrayToStr(Array arr) {
	if (!isByte(arr.etype)) panic("arrayToStr: it is not string");
	char* buf = (char*)malloc(arr.array.size() + 1);
	for (int i = 0; i < arr.array.size(); i++) {
		buf[i] = arr.array[i];
	}
	buf[arr.array.size()] = '\0';

	return buf;
}
#endif