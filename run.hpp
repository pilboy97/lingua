#ifndef __RUN_HPP__
#define __RUN_HPP__

#include "procStruct.hpp"
#include "panic.hpp"

#include <vector>
#include <algorithm>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>

#define ll long long

bool DEBUG = false;

struct Array {
    Type eType;
    int len;
    int cap;
    std::vector<ll> data;
};
struct Closure {
    int fid;

    std::vector<ll> cap;
};
struct Instance {
    DefClass cl;

    std::vector<ll> data;
};
struct IInstance {
    DefInterface in;
    DefClass cl;

    std::vector<ll> data;
};
struct Pointer {
    Type type;
    bool lv;
    ll ptr;
};
struct Frame {
    ll rValue;
    Type rType;
    Pointer _this;

    std::vector<Pointer> tmp;
    std::vector<std::vector<std::pair<const char*, Pointer>>> vars;
};

typedef Pointer(*func)(FCall args);
typedef void(*act)(Pointer ptr);

struct NFunc {
    Type type;
    const char* name;
    func fn;
    ll ptr;
};
struct Proc {
    ll SP;
    ll ESP;

    std::vector<Frame> STACK;
};

ll GP;
std::vector<ll> mem;
std::vector<ll> hmem;

std::vector<DefClass> _class;
std::vector<DefInterface> _interface;
std::vector<DefFunc> _fn;
std::vector<NFunc> _nf;

Type tVoid = Type{ 0 };
Type tAny = Type{ -1 };
Type tNum = Type{ NUM };
Type tByte = Type{ BYTE };
Type tBool = Type{ BOOL };
Type tReal = Type{ REAL };
Type tStr = Type{ OSB, "", {tByte} };

Pointer nil = { tVoid, false, 0 };

ll& access(Pointer p);
ll& hAccess(ll ptr);
ll& gAccess(ll ptr);
ll& sAccess(ll ptr);
std::vector<ll> oAccess(ll ptr);
Pointer pAccess(Pointer p);
Pointer pAlloc(Pointer ptr);
ll oAlloc(std::vector<ll> data);
ll hAlloc(int size);
ll sAlloc(ll value);

ll pAdd(ll ptr, ll x) {
    return (ptr < 0) ? -(-ptr + x) : ptr + x;
}

bool toBool(Pointer ptr);
char toByte(Pointer ptr);
ll toNum(Pointer ptr);
double toReal(Pointer ptr);
Array toArray(Pointer ptr);
Closure toClosure(Pointer ptr);
Instance toInstance(Pointer ptr);
IInstance toIInstance(Pointer ptr);
const char* toStr(Pointer ptr);
bool isStr(Type type);

Pointer makeClosure(int fid, Pointer _this);
Pointer makeArray(LiteralArray arr);
Pointer makeInstance(LiteralObject obj);
Pointer makeString(LiteralString str);

Pointer converse(Pointer x, Type type);

void memStat();
void objStat();
void varStat();

void initProc();

void run(Program prog);

void runStmt(Stmt stmt, jmp_buf jmp);
void runBlockStmt(BlockStmt stmt, jmp_buf jmp);
void runIfStmt(IfStmt stmt, jmp_buf jmp);
void runForStmt(ForStmt stmt, jmp_buf jmp);
void runRetStmt(RetStmt stmt, jmp_buf jmp);

Pointer runFCall(Pointer ptr, FCall args);
Pointer runIdx(Pointer ptr, Idx idx);
Pointer runMember(Pointer ptr, Word word);

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
Pointer runFactor(Factor f);

void defVar(DefVar var);
void defFunc(int fid);
void defClass(DefClass cl);
void defInterface(DefInterface in);

bool isAssAble(Type dst, Type src);
void assign(Pointer dst, Pointer src);

void nullPointerException();

void newGVar(const char* name, Pointer ptr);
void newLVar(const char* name, Pointer ptr);

Pointer findLVar(const char* name);
Pointer findGVar(const char* name);
int findNF(const char* name);
int findClass(const char* name);
int findInterface(const char* name);

const char* strAdd(const char* str, const char* str2);
const char* strAdd(const char* str, ll x);
const char* strAdd(const char* str, char x);
const char* strAdd(const char* str, bool x);
const char* strAdd(const char* str, double x);

void clear();
void cleanScope();
void cleanFrame();

void enterScope();
void exitScope();


void ref(Pointer ptr) {
    if (ptr.ptr == 0) return;
    if (isPriType(ptr.type)) return;

    hAccess(pAdd(ptr.ptr, -2))++;

    if (isArray(ptr.type)) {
        ll p = hAccess(ptr.ptr);
        ll len = hAccess(pAdd(ptr.ptr, 1));
        ll cap = hAccess(pAdd(ptr.ptr, 2));

        if (p == 0) return;

        for (int i = 0; i < len; i++) {
            ref(Pointer{ ptr.type.add[0],false,hAccess(pAdd(p, i)) });
        }
    }
    else if (isFunc(ptr.type)) {
        auto cl = toClosure(pAlloc(ptr));
        auto fn = _fn[cl.fid];

        ll p = hAccess(ptr.ptr);
        if (p == 0) return;
        for (int i = 0; i < cl.cap.size(); i++) {
            ref(Pointer{ fn.cType[i], false, hAccess(pAdd(p, i)) });
        }
    }
    else if (isInstance(ptr.type)) {
        auto in = toInstance(pAlloc(ptr));
        auto cl = in.cl;

        ll p = hAccess(ptr.ptr);
        if (p == 0) return;
        for (int i = 0; i < cl.field.size(); i++) {
            ref(Pointer{ cl.field[i].type, false, hAccess(pAdd(p, i)) });
        }
    }
    else if (isIInstance(ptr.type)) {
        auto iin = toIInstance(pAlloc(ptr));
        auto in = iin.in;
        auto cl = iin.cl;

        ll p = hAccess(ptr.ptr);
        if (p == 0) return;
        for (int i = 0; i < in.method.size(); i++) {
            ref(Pointer{ Type{FUNC}, false, hAccess(pAdd(p, i + 2)) });
        }

        hAccess(pAdd(p, -2)) = 0;
    }
    else panic("ref: wrong type");
}
void dref(Pointer ptr) {
    if (ptr.ptr == 0) return;
    if (isPriType(ptr.type)) return;

    if (access(ptr) == 0) return;

    if(hAccess(pAdd(ptr.ptr, -2)) > 0)
        hAccess(pAdd(ptr.ptr, -2))--;

    if (hAccess(pAdd(ptr.ptr, -2)) > 0) return;
    hAccess(pAdd(ptr.ptr, -2)) = 0;

    if (isArray(ptr.type)) {
        ll p = hAccess(ptr.ptr);
        ll len = hAccess(pAdd(ptr.ptr, 1));
        ll cap = hAccess(pAdd(ptr.ptr, 2));

        if (p == 0) return;

        for (int i = 0; i < len; i++) {
            dref(Pointer{ ptr.type.add[0],false,hAccess(pAdd(p, i)) });
            hAccess(pAdd(p, i)) = 0;
        }
        
        hAccess(pAdd(p, -2)) = 0;
    }
    else if (isFunc(ptr.type)) {
        auto cl = toClosure(pAlloc(ptr));
        auto fn = _fn[cl.fid];

        ll p = hAccess(ptr.ptr);
        if (p == 0) return;
        for (int i = 1; i < cl.cap.size(); i++) {
            dref(Pointer{ fn.cType[i], false, hAccess(pAdd(p, i)) });
        }
        for (int i = 0; i < cl.cap.size(); i++) {
            hAccess(pAdd(p, i)) = 0;
        }

        hAccess(pAdd(p, -2)) = 0;
    }
    else if (isInstance(ptr.type)) {
        auto in = toInstance(pAlloc(ptr));
        auto cl = in.cl;

        ll p = hAccess(ptr.ptr);
        if (p == 0) return;
        for (int i = 0; i < cl.field.size(); i++) {
            dref(Pointer{ cl.field[i].type, false, hAccess(pAdd(p, i)) });
        }
        for (int i = 0; i < cl.field.size(); i++) {
            hAccess(pAdd(p, i)) = 0;
        }

        hAccess(pAdd(p, -2)) = 0;
    }
    else if (isIInstance(ptr.type)) {
        auto iin = toIInstance(pAlloc(ptr));
        auto in = iin.in;
        auto cl = iin.cl;

        ll p = hAccess(ptr.ptr);
        if (p == 0) return;
        for (int i = 0; i < in.method.size(); i++) {
            dref(Pointer{ Type{FUNC}, false, hAccess(pAdd(p, i + 2)) });
        }
        for (int i = 0; i < in.method.size(); i++) {
            hAccess(pAdd(p, i)) = 0;
        }

        hAccess(pAdd(p, -2)) = 0;
    }
    else panic("dref: wrong type");
}

bool isImplOf(DefInterface x, DefClass y) {
    for (int j = 0; j < x.method.size(); j++) {
        auto m2 = x.method[j];
        auto f2 = x.method[j];
        bool found = false;
        for (int i = 0; i < y.method.size(); i++) {
            auto m = y.method[i];
            auto f = _fn[m.idx];

            if (strcmp(m.name, m2.name) == 0) {
                found = true;
                if (!isSameType(f.ret, f2.ret)) return false;

                for (int k = 0; k < f.frame.size(); k++) {
                    if (!isSameType(f.frame[k].second, f2.frame[k])) return false;
                }

                break;
            }
        }
        if (!found) return false;
    }
    return true;
}
ll realBits(double x) {
    ll ret;
    memcpy(&ret, &x, sizeof(ret));
    return ret;
}
double restore(ll x) {
    double ret;
    memcpy(&ret, &x, sizeof(ret));
    return ret;
}


Proc proc;

const char* toStr(Pointer x) {
    if (!isStr(x.type)) panic("toStr: it is not string");
    Array arr = toArray(x);

    char* buf = (char*)malloc(sizeof(char) * (arr.len + 1));
    for (int i = 0; i < arr.len; i++) {
        buf[i] = arr.data[i];
    }
    buf[arr.len] = '\0';
    return buf;
}

bool isStr(Type type) {
    return isArray(type) && isByte(type.add[0]);
}

Pointer nfPrint(FCall args) {
    if (args.list.size() != 1) panic("nfPrint: argument mismatch");

    Pointer ptr = runExpr(args.list[0]);
 
    if (!isStr(ptr.type)) panic("nfPrint: argument mismatch");

    Array arr = toArray(ptr);
    int len = arr.len;

    for (int i = 0; i < len; i++) {
        if(arr.data[i] != '\\')
            putchar(arr.data[i]);
        else {
            if (arr.data[i + 1] == 'n') {
                putchar('\n');
            }
            else if (arr.data[i + 1] == 'r') {
                putchar('\r');
            }
            else if (arr.data[i + 1] == 't') {
                putchar('\t');
            }
            i++;
        }
    }

    return nil;
}
Pointer nflen(FCall args) {
    if (args.list.size() != 1) panic("len: argument mismatch");

    Pointer ptr = runExpr(args.list[0]);

    if (!isArray(ptr.type)) panic("len: argument mismatch: not Array");

    Array arr = toArray(ptr);
    int len = arr.len;

    return Pointer{tNum, false, sAlloc(len)};
}
Pointer nfappend(FCall args) {
    if (args.list.size() != 2) panic("append: argument mismatch");

    Pointer ptr = runExpr(args.list[0]);
    Pointer elem = runExpr(args.list[1]);

    ref(pAccess(elem));

    if (!isArray(ptr.type)) panic("append: argument mismatch: not Array");
    if (!isAssAble(ptr.type.add[0], elem.type)) panic("append: argument mismatch: type mismatch");

    Array arr = toArray(ptr);
    int len = arr.len;
    int cap = arr.cap;

    ptr = pAccess(ptr);
    if (len + 1 >= cap) {
        hAccess(pAdd(ptr.ptr, 1)) = len + 1;
        hAccess(pAdd(ptr.ptr, 2)) = cap * 2;

        ll data = hAlloc(cap * 2);
        for (int i = 0; i < arr.data.size(); i++) {
            hAccess(pAdd(data, i)) = arr.data[i];
        }
        hAccess(pAdd(data, len)) = access(elem);

        ll old = hAccess(ptr.ptr);
        hAccess(ptr.ptr) = data;

        ll& cnt = hAccess(pAdd(old, -2));
        cnt--;
        if (cnt <= 0) cnt = 0;
    }
    else {
        hAccess(pAdd(ptr.ptr, 1)) = len + 1;

        ll data = hAccess(ptr.ptr);
        hAccess(pAdd(data, len)) = access(elem);
    }

    return nil;
}
ll& access(Pointer p) {
    if (p.ptr < 0) return hAccess(p.ptr);
    if (p.ptr == 0) nullPointerException();
    if (0 < p.ptr && p.ptr < GP) return gAccess(p.ptr);
    
    return sAccess(p.ptr);
}
ll& hAccess(ll ptr) {
    ptr = -ptr;
    if (!(0 < ptr && ptr < hmem.size()))
        panic("hAccess: invalid pointer");

    return hmem[ptr];
}
ll& gAccess(ll ptr) {
    if (!(0 < ptr && ptr < GP))
        panic("gAccess: invalid pointer");

    return mem[ptr];
}
ll& sAccess(ll ptr) {
    if (!(proc.SP < ptr && ptr < proc.ESP))
        panic("sAccess: invalid pointer");

    return mem[ptr];
}
std::vector<ll> oAccess(ll ptr) {
    int size = hAccess(ptr);
    int p = hAccess(pAdd(ptr, 1));

    std::vector<ll> data;
    for (int i = 0; i < size; i++) {
        data.push_back(hAccess(pAdd(p, i)));
    }

    return data;
}
Pointer pAccess(Pointer ptr) {
    return Pointer{ ptr.type, ptr.lv, access(ptr) };
}
ll oAlloc(std::vector<ll> data) {
    ll ptr = hAlloc(2);
    ll ptr2 = hAlloc(data.size());

    hAccess(ptr) = data.size();
    hAccess(ptr) = ptr2;
    
    for (int i = 0; i < data.size(); i++) {
        hAccess(ptr2 + i) = data[i];
    }

    return ptr;
}
ll hAlloc(int size) {
    if (size == 0) return 0;

    int nbsize = size + 2;
    int begin = -1;
    int end;

    int i = 1;

    while (i < hmem.size()) {
        int cnt = hmem[i];
        int bsize = hmem[i + 1];

        if (bsize == 0) {
            panic("mem is corrupted");
        }

        if (cnt == 0) {
            if (begin == -1) {
                begin = i;
            }
        }
        else {
            if (begin != -1) {
                end = i;

                int nowbsize = end - begin;

                if (nowbsize >= nbsize) {
                    if (nowbsize - nbsize <= 2) {
                        hmem[begin] = 1;
                        hmem[begin + 1] = nowbsize;
                    }
                    else {
                        hmem[begin] = 1;
                        hmem[begin + 1] = nbsize;
                        hmem[begin + nbsize] = 0;
                        hmem[begin + nbsize + 1] = nowbsize - nbsize;
                    }
                    return -(begin + 2);
                }
                else {
                    begin = -1;
                }
            }
        }
        i += bsize;
    }
    if (begin == -1) {
        begin = hmem.size();
    }

    while (begin + nbsize > hmem.size()) {
        hmem.push_back(0);
    }

    hmem[begin] = 1;
    hmem[begin + 1] = nbsize;
    while (begin + nbsize < hmem.size()) hmem.pop_back();

    return -(begin + 2);
}
ll sAlloc(ll value) {
    ll ret = proc.ESP;
    while (proc.ESP + 1 > mem.size()) {
        mem.push_back(0);
    }
    proc.ESP++;

    mem[ret] = value;

    return ret;
}
Pointer pAlloc(Pointer ptr) {
    return Pointer{ ptr.type, ptr.lv, sAlloc(ptr.ptr) };
}
void nullPointerException() {
    panic("try to access null pointer");
}

void run(Program prog) {
    initProc();

    mem.push_back(0);
    hmem.push_back(0);

    ll pPrint = makeClosure(-1, nil).ptr;
    ll pLen = makeClosure(-2, nil).ptr;
    ll pAppend = makeClosure(-3, nil).ptr;

    _fn = prog.fn;
    _nf.push_back(NFunc{});
    _nf.push_back(NFunc{Type{FUNC, "", {tVoid,tStr}}, "print", nfPrint, pPrint});
    _nf.push_back(NFunc{ Type{FUNC, "", {tVoid,tStr}}, "len", nflen, pLen });
    _nf.push_back(NFunc{ Type{FUNC, "", {tVoid,tStr}}, "append", nfappend, pAppend });


    for (int i = 0; i < prog.childs.size(); i++) {
        auto duo = prog.childs[i];
        auto kind = duo.first;
        auto def = duo.second;

        int* ptr1;
        DefClass* ptr2;
        DefInterface* ptr3;

        switch (kind) {
        case FUNC:
            ptr1 = (int*)def;
            defFunc(*ptr1);

            break;
        case CLASS:
            ptr2 = (DefClass*)def;
            defClass(*ptr2);

            break;
        case INTERFACE:
            ptr3 = (DefInterface*)def;
            defInterface(*ptr3);
        }
    }

    GP = proc.ESP;

    jmp_buf jmp;
    if (setjmp(jmp) == 0) {
        runFCall(findGVar("main"), FCall{});
    }
}

void initProc() {
    proc.SP = 1;
    proc.ESP = 2;
    
    proc.STACK.push_back(Frame{});
    proc.STACK.back().vars.push_back(std::vector<std::pair<const char*, Pointer>>());
}
void debug() {
    objStat();
    varStat();
}
void defVar(DefVar var) {
    auto name = var.name;
    if (findLVar(name).ptr != 0) panicf("defVar: %s is already exists");

    auto& vars = proc.STACK.back().vars;

    if (var.init == NULL && var.type == NULL) {
        panic("defVar: cannot decide variable type");
    }

    if (var.init == NULL) {
        Pointer ptr;
        if(var.type->kind != OSB)
            ptr = { *var.type, true, sAlloc(0) };
        else {
            ll p = hAlloc(3);
            ll data = hAlloc(1);

            hAccess(p) = data;
            hAccess(pAdd(p, 1)) = 0;
            hAccess(pAdd(p, 2)) = 1;

            ptr = { *var.type, true, sAlloc(p) };
        }
        newLVar(name, ptr);
        return;
    }

    if (var.type == NULL) {
        Pointer ptr = runExpr(*var.init);
        Pointer ret = { ptr.type, ptr.lv, sAlloc(1) };
        assign(ret, ptr);
        newLVar(name, ret);
        return;
    }

    Pointer ptr = runExpr(*var.init);
    if (!isAssAble(*var.type, ptr.type)) {
        panic("defVar: cannot initialize variable: type mismatch");
    }

    Pointer ret = Pointer{ *var.type, true, sAlloc(1) };
    assign(ret, ptr);
    newLVar(name, ret);
}
bool isAssAble(Type dst, Type src) {
    if (isPriType(dst) && isPriType(src)) return true;
    if (isSameType(dst, src)) return true;
    if (src.kind == 0) return true;
    
    int cid = findClass(src.name);
    int iid = findInterface(dst.name);

    if (iid != -1 && cid != -1) {
        return isImplOf(_interface[iid], _class[cid]);
    }

    return false;
}
void assign(Pointer dst, Pointer src) {
    if (!isAssAble(dst.type, src.type)) panic("assign: cannot assign: type mismatch");

    dref(pAccess(dst));
    ref(pAccess(src));

    if (isSameType(dst.type, src.type)) {
        access(dst) = access(src);
    }
    else {
        ll iid = findInterface(dst.type.name);
        ll cid = findClass(src.type.name);

        DefInterface in = _interface[iid];
        DefClass cl = _class[cid];

        std::vector<ll> data;

        data.push_back(findClass(src.type.name));
        data.push_back(access(src));

        for (int i = 0; i < in.method.size(); i++) {
            auto m = in.method[i];
            for (int j = 0; j < cl.method.size(); j++) {
                auto m2 = cl.method[j];

                if (strcmp(m.name, m2.name) == 0) {
                    data.push_back(access(makeClosure(m2.idx, src)));
                }
            }
        }

        ll ptr = hAlloc(2);
        ll ptr2 = hAlloc(data.size());

        hAccess(ptr) = ptr2;
        hAccess(pAdd(ptr, 1)) = iid;

        for (int i = 0; i < data.size(); i++) {
            hAccess(pAdd(ptr2, i)) = data[i];
        }

        access(dst) = ptr;
    }
}
Pointer findLVar(const char* name) {
    auto frame = proc.STACK.back().vars;
    for (int i = frame.size() - 1; i >= 0; i--) {
        for (int j = 0; j < frame[i].size(); j++) {
            auto duo = frame[i][j];
            auto vname = duo.first;
            auto vptr = duo.second;

            if (strcmp(name, vname) == 0) {
                return vptr;
            }
        }
    }
    return Pointer{ tVoid, false, 0 };
}
Pointer findGVar(const char* name) {
    auto frame = proc.STACK.front().vars;
    for (int i = frame.size() - 1; i >= 0; i--) {
        for (int j = 0; j < frame[i].size(); j++) {
            auto duo = frame[i][j];
            auto vname = duo.first;
            auto vptr = duo.second;

            if (strcmp(name, vname) == 0) {
                return vptr;
            }
        }
    }

    int fid = findNF(name);
    if (fid > 0)
        return Pointer{ _nf[fid].type, false, _nf[fid].ptr};
    else return nil;
}
int findClass(const char* name) {
    for (int i = 0; i < _class.size(); i++) {
        if (strcmp(_class[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}
int findInterface(const char* name) {
    for (int i = 0; i < _interface.size(); i++) {
        if (strcmp(_interface[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void newGVar(const char* name, Pointer ptr) {
    if (findGVar(name).ptr != 0) panicf("newGVar: %s is already exists", name);
    if(findClass(name) != -1)  panicf("newGVar: %s is already exists", name);
    if (findInterface(name) != -1)  panicf("newGVar: %s is already exists", name);

    auto& frame = proc.STACK.front();
    frame.vars.back().push_back(std::make_pair(name, Pointer{ ptr.type, true, ptr.ptr }));
    ref(pAccess(ptr));
}
void newLVar(const char* name, Pointer ptr) {
    if (findLVar(name).ptr != 0) panicf("newLVar: %s is already exists", name);

    auto& frame = proc.STACK.back();
    frame.vars.back().push_back(std::make_pair(name, Pointer{ ptr.type, true, ptr.ptr }));
    ref(pAccess(ptr));
}
void defClass(DefClass def) {
    if (findGVar(def.name).ptr != 0) panicf("defClass: %s is already exists", def.name);
    if (findClass(def.name) != -1)  panicf("defClass: %s is already exists", def.name);
    if (findInterface(def.name) != -1)  panicf("defClass: %s is already exists", def.name);

    _class.push_back(def);
}
void defInterface(DefInterface def) {
    if (findGVar(def.name).ptr != 0) panicf("defInterface: %s is already exists", def.name);
    if (findClass(def.name) != -1)  panicf("defInterface: %s is already exists", def.name);
    if (findInterface(def.name) != -1)  panicf("defInterface: %s is already exists", def.name);

    _interface.push_back(def);
}
void defFunc(int fid) {
    DefFunc& def = _fn[fid];

    if (findGVar(def.name).ptr != 0) panicf("defFunc: %s is already exists", def.name);
    if (findClass(def.name) != -1)  panicf("defFunc: %s is already exists", def.name);
    if (findInterface(def.name) != -1)  panicf("defFunc: %s is already exists", def.name);

    Pointer cl = makeClosure(fid, nil);

    newGVar(def.name, cl);
}

void runStmt(Stmt stmt, jmp_buf jmp) {
    int ESP = proc.ESP;

    if (DEBUG) {
        puts("");
        puts("---   ---   ---");
        printStmt(stmt);
    }

    switch (stmt.kind)
    {
    case OBL:
        runBlockStmt(*(BlockStmt*)stmt.stmt, jmp);
        break;
    case IF:
        runIfStmt(*(IfStmt*)stmt.stmt, jmp);
        break;
    case FOR:
        runForStmt(*(ForStmt*)stmt.stmt, jmp);
        break;
    case RETURN:
        runRetStmt(*(RetStmt*)stmt.stmt, jmp);
        break;
    case BREAK:
        longjmp(jmp, 2);
        break;
    case CONTINUE:
        longjmp(jmp, 3);
        break;
    case VAR:
        defVar(*(DefVar*)stmt.stmt);
        ESP = proc.ESP;
        break;
    case SCOLON:
        runExpr_1(*(Expr_1*)stmt.stmt);
        break;
    }
    clear();
    proc.ESP = ESP;
    if (DEBUG) {
        debug();
        printf("END STATEMENT\n");
        puts("---   ---   ---");
        puts("");
    }
}
void runBlockStmt(BlockStmt stmt, jmp_buf jmp) {
    enterScope();
    for (int i = 0; i < stmt.childs.size(); i++) {
        auto s = stmt.childs[i];
        
        runStmt(s, jmp);
    }
    exitScope();
}
void runIfStmt(IfStmt stmt, jmp_buf jmp) {
    for (int i = 0; i < stmt._if.size(); i++) {
        auto cond = stmt._if[i].first;
        auto body = stmt._if[i].second;

        if (toBool(runExpr(cond))) {
            runBlockStmt(body, jmp);
            return;
        }
    }
    if (stmt._else != NULL) {
        runBlockStmt(*stmt._else, jmp);
    }
}
void runForStmt(ForStmt stmt, jmp_buf jmp) {
    if (stmt.init != NULL) {
        runExpr_1(*stmt.init);
    }

    while (true) {
        if (stmt.cond != NULL && !toBool(runExpr_1(*stmt.cond))) {
            break;
        }

        jmp_buf jp;
        int jv;

        if ((jv = setjmp(jp)) == 0) {
            runBlockStmt(stmt.body, jp);
        }
        else if (jv == 1) {
            longjmp(jmp, 1);
        }
        else if (jv == 2) {
            break;
        }
        else {
            continue;
        }

        if (stmt.act != NULL) {
            runExpr_1(*stmt.act);
        }
    }
}
Pointer runFCall(Pointer ptr, FCall args) {
    auto fn = toClosure(ptr);
    ptr = pAccess(ptr);

    if (fn.fid >= 0) {
        auto def = _fn[fn.fid];

        std::vector<ll> alist;
        std::vector<ll> clist;

        for (int i = 0; i < args.list.size(); i++) {
            alist.push_back(access(runExpr(args.list[i])));
        }

        proc.STACK.push_back(Frame());
        proc.STACK.back().vars.push_back(std::vector<std::pair<const char*, Pointer>>());

        sAlloc(proc.SP);
        proc.SP = proc.ESP - 1;

        proc.STACK.back()._this = Pointer{ def.cType[0], true, sAlloc(fn.cap[0]) };

        if (fn.cap.size() != def.captured.size() + 1) panic("runFCall: cannot call function: captured mismatch");

        for (int i = 1; i < fn.cap.size(); i++) {
            auto c = fn.cap[i];
            auto f = def;

            newLVar(f.captured[i - 1], Pointer{ def.cType[i], true, sAlloc(c) });
        }

        if (alist.size() != def.frame.size()) panic("runFCall: cannot call function: argument mismatch");

        for (int i = 0; i < alist.size(); i++) {
            newLVar(def.frame[i].first, Pointer{ def.frame[i].second, true, sAlloc(alist[i]) });
        }

        jmp_buf jp;
        int jv;

        if ((jv = setjmp(jp)) == 0) {
            runBlockStmt(def.body, jp);

            proc.STACK.back().rValue = 0;
            proc.STACK.back().rType = tVoid;

            longjmp(jp, 1);
        }
        else if (jv == 1) {
            Pointer ret;

            if (proc.STACK.back().rType.kind != 0) {
                auto type = proc.STACK.back().rType;
                auto value = proc.STACK.back().rValue;

                ret = Pointer{ type, false, value };
            }
            else {
                ret = nil;
            }

            cleanFrame();

            proc.ESP = proc.SP;
            proc.SP = mem[proc.SP];
            proc.STACK.pop_back();

            if (ret.ptr != 0)
                ret = pAlloc(ret);
            
            return ret;
        }
        else if (jv == 2) {
            panic("runFCall: unexpected break");
        }
        else {
            panic("runFCall: unexpected continue");
        }

        return nil;
    }
    else {
        NFunc nf = _nf[-fn.fid];
        return (*nf.fn)(args);
    }
}
void runRetStmt(RetStmt stmt, jmp_buf jmp) {
    Pointer ret;
    if (stmt.expr == NULL) {
        ret = nil;
    }
    else {
        ret = pAccess(runExpr(*stmt.expr));
    }

    proc.STACK.back().rType = ret.type;
    proc.STACK.back().rValue = ret.ptr;

    longjmp(jmp, 1);
}

bool toBool(Pointer ptr) {
    if (!isBool(ptr.type)) panic("toBool: it is not bool");

    ll p = access(ptr);
    return (bool)p;
}
char toByte(Pointer ptr) {
    if (!isByte(ptr.type)) panic("toByte: it is not byte");

    ll p = access(ptr);
    return (char)p;
}
ll toNum(Pointer ptr) {
    if (!isNum(ptr.type)) panic("toNum: it is not num");

    ll p = access(ptr);
    return (ll)p;
}
const char* strAdd(const char* str, const char* str2) {
    int len1 = strlen(str);
    int len2 = strlen(str2);

    char* buf = (char*)malloc(sizeof(char) * (len1 + len2 + 1));
    sprintf(buf, "%s%s", str, str2);
    buf[len1 + len2] = '\0';
    return buf;
}
const char* strAdd(const char* str, ll x) {
    char buf[64];
    sprintf(buf, "%d", x);

    return strAdd(str, buf);
}
const char* strAdd(const char* str, char x) {
    char buf[64];
    sprintf(buf, "%d", x);

    return strAdd(str, buf);
}
const char* strAdd(const char* str, bool x) {
    if (x)
        return strAdd(str, "true");
    else
        return strAdd(str, "false");
}
const char* strAdd(const char* str, double x) {
    char buf[64];
    sprintf(buf, "%f", x);

    return strAdd(str, buf);
}
double toReal(Pointer ptr) {
    if (!isReal(ptr.type)) panic("toReal: it is not real");

    ll p = access(ptr);
    return (double)restore(p);
}
Array toArray(Pointer ptr) {
    if (!isArray(ptr.type)) panic("toArray: it is not array");

    Array ret;
    ret.eType = ptr.type.add[0];

    ptr = pAccess(ptr);
    ret.len = hAccess(pAdd(ptr.ptr, 1));
    ret.cap = hAccess(pAdd(ptr.ptr, 2));


    int p = access(ptr);
    if (p == 0) {
        if (ret.len != 0)
            panic("toArray: not empty array");
        return ret;
    }
    for (int i = 0; i < ret.len; i++) {
        ret.data.push_back(hAccess(pAdd(p, i)));
    }

    return ret;
}
Closure toClosure(Pointer ptr) {
    Closure ret;

    ptr = pAccess(ptr);
    ll fid = hAccess(pAdd(ptr.ptr, 1));
    
    ret.fid = fid;

    if (fid >= 0) {
        if (!(0 <= fid && fid < _fn.size())) panic("toClosure: wrong fid");
        ptr = pAccess(ptr);
        for (int i = 0; i < _fn[fid].cType.size(); i++) {
            ret.cap.push_back(hAccess(pAdd(ptr.ptr, i)));
        }
    }

    return ret;
}
Instance toInstance(Pointer ptr) {
    Instance ret;

    ptr = pAccess(ptr);
    int cid = hAccess(pAdd(ptr.ptr, 1));
    if (!(0 <= cid && cid < _class.size())) panic("toInstance: wrong cid");
    ret.cl = _class[cid];
    
    int p = hAccess(ptr.ptr);
    for (int i = 0; i < ret.cl.field.size(); i++) {
        ret.data.push_back(hAccess(pAdd(p, i)));
    }

    return ret;
}
IInstance toIInstance(Pointer ptr) {
    IInstance ret;

    ptr = pAccess(ptr);
    int iid = hAccess(pAdd(ptr.ptr, 1));
    if (!(0 <= iid && iid < _interface.size())) panic("toClosure: wrong fid");
    ret.in = _interface[iid];

    int p = hAccess(ptr.ptr);
    for (int i = 0; i < ret.in.method.size(); i++) {
        ret.data.push_back(hAccess(pAdd(p, i)));
    }

    ret.cl = _class[ret.data[0]];

    return ret;
}
Pointer makeClosure(int fid, Pointer _this) {
    Pointer ret;

    if (fid >= 0) {
        if (!(0 <= fid && fid < _fn.size())) panic("makeClosure: wrong fid");
        DefFunc& fn = _fn[fid];
        std::vector<ll> cap;
        std::vector<Type> ctype;

        fn.cType = std::vector<Type>(fn.captured.size() + 1);
        fn.cType[0] = _this.type;
        if (_this.ptr != 0) {
            cap.push_back(sAccess(_this.ptr));
        }
        else
            cap.push_back(0);
        for (int i = 0; i < fn.captured.size(); i++) {
            auto vptr = findLVar(fn.captured[i]);
            if (vptr.ptr == 0) {
                panicf("makeClosure: cannot capture variable: cannot find %s", fn.captured[i]);
            }

            fn.cType[i + 1] = vptr.type;
            cap.push_back(access(vptr));
        }

        ll ptr = hAlloc(2);
        ll ptr2 = hAlloc(cap.size());

        hAccess(ptr) = ptr2;
        hAccess(pAdd(ptr, 1)) = fid;
        for (int i = 0; i < cap.size(); i++) {
            hAccess(pAdd(ptr2, i)) = cap[i];
        }

        ret.type = fnToType(fn);
        ret.ptr = ptr;

        ret = pAlloc(ret);
        proc.STACK.back().tmp.push_back(ret);
    }
    else {
        ll ptr = hAlloc(2);
        ll ptr2 = hAlloc(0);

        hAccess(ptr) = ptr2;
        hAccess(pAdd(ptr, 1)) = fid;

        ret.type = tAny;
        ret.ptr = ptr;
        ret = pAlloc(ret);
    }

    
    return ret;
}
Pointer makeArray(LiteralArray arr) {
    Pointer ret;
    
    ll ptr = hAlloc(3);
    ll ptr2 = hAlloc(arr.elem.size());

    hAccess(ptr) = ptr2;
    hAccess(pAdd(ptr, 1)) = arr.elem.size();
    hAccess(pAdd(ptr, 2)) = arr.elem.size();

    for (int i = 0; i < arr.elem.size(); i++) {
        hAccess(pAdd(ptr2, i)) = sAccess(runExpr(arr.elem[i]).ptr);
    }

    ret.ptr = ptr;
    ret.lv = false;
    ret.type = Type{ OSB, "", {arr.etype} };
    ret = pAlloc(ret);
    proc.STACK.back().tmp.push_back(ret);
    return ret;
}
Pointer makeInstance(LiteralObject obj) {
    Pointer ret;
    int cid = findClass(obj.type.name);
    
    if (cid == -1) {
        panicf("makeInstance: cannot find class %s", obj.type.name);
    }

    int ptr = hAlloc(2);

    DefClass cl = _class[cid];
    std::vector<ll> data;

    for (int i = 0; i < cl.field.size(); i++) {
        auto f1 = cl.field[i];
        bool found = false;
        for (int j = 0; j < obj.init.list.size(); j++) {
            auto f2 = obj.init.list[j];

            if (strcmp(f1.name, f2.first) == 0) {
                found = true;
                
                data.push_back(sAccess(runExpr(f2.second).ptr));

                break;
            }
        }
        if (!found) {
            data.push_back(0);
        }
    }

    int p = hAlloc(cl.field.size());

    hAccess(ptr) = p;
    hAccess(pAdd(ptr, 1)) = cid;
    
    for (int i = 0; i < data.size(); i++) {
        hAccess(pAdd(p, i)) = data[i];
    }

    ret.type = obj.type;
    ret.ptr = ptr;
    ret = pAlloc(ret);
    proc.STACK.back().tmp.push_back(ret);
    return ret;
}

Pointer runExpr_1(Expr_1 e) {
    if (e.src == NULL) {
        return runExpr(e.dst);
    }

    auto src = runExpr(*e.src);
    auto dst = runExpr(e.dst);

    assign(dst, src);
    return nil;
}
Pointer runExpr(Expr e) {
    auto init = runExpr1(e.childs[0]);
    if (e.childs.size() == 1) {
        return init;
    }

    if (toBool(init)) {
        return pAlloc(Pointer{ tBool, false, sAlloc(true) });
    }

    for (int i = 1; i < e.childs.size(); i++) {
        if (toBool(runExpr1(e.childs[i]))) {
            return pAlloc(Pointer{ tBool, false, sAlloc(true) });
        }
    }

    return Pointer{ tBool, false, sAlloc(false) };
}
Pointer runExpr1(Expr1 e) {
    auto init = runExpr2(e.childs[0]);
    if (e.childs.size() == 1) {
        return init;
    }

    if (!toBool(init)) {
        return Pointer{ tBool, false, sAlloc(false) };
    }

    for (int i = 1; i < e.childs.size(); i++) {
        if (toBool(runExpr2(e.childs[i]))) {
            return Pointer{ tBool, false, sAlloc(false) };
        }
    }

    return Pointer{ tBool, false, sAlloc(true) };
}
Pointer runExpr2(Expr2 e) {
    auto init = runExpr3(e.childs[0]);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto v = runExpr3(e.childs[i]);

        if (isNum(ret.type) && isNum(v.type)) {
            ll a, b, c;
            a = toNum(ret);
            b = toNum(v);
            c = a | b;

            ret.type = tNum;
            ret.ptr = sAlloc(c);
        }
        else if (isByte(ret.type) && isByte(v.type)) {
            char a, b, c;
            a = toByte(ret);
            b = toByte(v);
            c = a | b;

            ret.type = tByte;
            ret.ptr = sAlloc(c);
        }
        else {
            panic("runExpr2: type mismatch");
        }
    }

    return ret;
}
Pointer runExpr3(Expr3 e) {
    auto init = runExpr4(e.childs[0]);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto v = runExpr4(e.childs[i]);

        if (isNum(ret.type) && isNum(v.type)) {
            ll a, b, c;
            a = toNum(ret);
            b = toNum(v);
            c = a ^ b;

            ret.type = tNum;
            ret.ptr = sAlloc(c);
        }
        else if (isByte(ret.type) && isByte(v.type)) {
            char a, b, c;
            a = toByte(ret);
            b = toByte(v);
            c = a ^ b;

            ret.type = tByte;
            ret.ptr = sAlloc(c);
        }
        else {
            panic("runExpr3: type mismatch");
        }
    }

    return ret;
}
Pointer runExpr4(Expr4 e) {
    auto init = runExpr5(e.childs[0]);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto v = runExpr5(e.childs[i]);

        if (isNum(ret.type) && isNum(v.type)) {
            ll a, b, c;
            a = toNum(ret);
            b = toNum(v);
            c = a & b;

            ret.type = tNum;
            ret.ptr = sAlloc(c);
        }
        else if (isByte(ret.type) && isByte(v.type)) {
            char a, b, c;
            a = toByte(ret);
            b = toByte(v);
            c = a & b;

            ret.type = tByte;
            ret.ptr = sAlloc(c);
        }
        else {
            panic("runExpr4: type mismatch");
        }
    }

    return ret;
}
Pointer runExpr5(Expr5 e) {
    auto init = runExpr6(e.childs[0].second);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto kind = e.childs[i].first;
        auto v = runExpr6(e.childs[i].second);

        if (isNum(ret.type) && isNum(v.type)) {
            ll a, b, c;
            a = toNum(ret);
            b = toNum(v);
            if (kind == EQ)
                c = (a == b);
            else
                c = (a != b);

            ret.type = tBool;
            ret.ptr = sAlloc(c);
        }
        else if (isByte(ret.type) && isByte(v.type)) {
            char a, b, c;
            a = toByte(ret);
            b = toByte(v);
            if (kind == EQ)
                c = (a == b);
            else
                c = (a != b);

            ret.type = tBool;
            ret.ptr = sAlloc(c);
        }
        else {
            panic("runExpr5: type mismatch");
        }
    }

    return ret;
}
Pointer runExpr6(Expr6 e) {
    auto init = runExpr7(e.childs[0].second);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto kind = e.childs[i].first;
        auto v = runExpr7(e.childs[i].second);

        if (isNum(ret.type) && isNum(v.type)) {
            ll a, b, c;
            a = toNum(ret);
            b = toNum(v);
            if (kind == LE)
                c = (a < b);
            else if (kind == GR)
                c = (a > b);
            else if (kind == LEQ)
                c = (a <= b);
            else
                c = (a >= b);
            
            ret.type = tBool;
            ret.ptr = sAlloc(c);
        }
        else if (isByte(ret.type) && isByte(v.type)) {
            char a, b, c;
            a = toByte(ret);
            b = toByte(v);
            if (kind == LE)
                c = (a < b);
            else if (kind == GR)
                c = (a > b);
            else if (kind == LEQ)
                c = (a <= b);
            else
                c = (a >= b);

            ret.type = tBool;
            ret.ptr = sAlloc(c);
        }
        else {
            panic("runExpr6: type mismatch");
        }
    }

    return ret;
}
Pointer runExpr7(Expr7 e) {
    auto init = runExpr8(e.childs[0].second);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto kind = e.childs[i].first;
        auto v = runExpr8(e.childs[i].second);

        if (isNum(ret.type) && isNum(v.type)) {
            ll a, b, c;
            a = toNum(ret);
            b = toNum(v);
            if (kind == LSH)
                c = (a << b);
            else
                c = (a >> b);

            ret.type = tNum;
            ret.ptr = sAlloc(c);
        }
        else if (isByte(ret.type) && isByte(v.type)) {
            char a, b, c;
            a = toByte(ret);
            b = toByte(v);
            if (kind == LSH)
                c = (a << b);
            else
                c = (a >> b);

            ret.type = tByte;
            ret.ptr = sAlloc(c);
        }
        else {
            panic("runExpr7: type mismatch");
        }
    }

    return ret;
}
Pointer runExpr8(Expr8 e) {
    auto init = runExpr9(e.childs[0].second);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto kind = e.childs[i].first;
        auto v = runExpr9(e.childs[i].second);

        if (isNum(ret.type) && isNum(v.type)) {
            ll a, b, c;
            a = toNum(ret);
            b = toNum(v);
            if (kind == ADD)
                c = (a + b);
            else
                c = (a - b);

            ret.type = tNum;
            ret.ptr = sAlloc(c);
        }
        else if (isByte(ret.type) && isByte(v.type)) {
            char a, b, c;
            a = toByte(ret);
            b = toByte(v);
            if (kind == ADD)
                c = (a + b);
            else
                c = (a - b);

            ret.type = tByte;
            ret.ptr = sAlloc(c);
        }
        else if (isStr(ret.type) && isStr(v.type)) {
            const char* a = toStr(ret), *b = toStr(v), * c;
            c = strAdd(a, b);

            ret = makeString(LiteralString{ c });
        }
        else if (isStr(ret.type) && isNum(v.type)) {
            const char* a = toStr(ret), * c;
            ll b = toNum(v);

            c = strAdd(a, b);

            ret = makeString(LiteralString{ c });
        }
        else if (isStr(ret.type) && isByte(v.type)) {
            const char* a = toStr(ret), * c;
            char b = toByte(v);

            c = strAdd(a, b);

            ret = makeString(LiteralString{ c });
        }
        else if (isStr(ret.type) && isBool(v.type)) {
            const char* a = toStr(ret), * c;
            bool b = toBool(v);

            c = strAdd(a, b);

            ret = makeString(LiteralString{ c });
        }
        else if (isStr(ret.type) && isReal(v.type)) {
            const char* a = toStr(ret), * c;
            double b = toReal(v);

            c = strAdd(a, b);

            ret = makeString(LiteralString{ c });
        }
        else {
            panic("runExpr8: type mismatch");
        }
    }

    return ret;
}
Pointer runExpr9(Expr9 e) {
    auto init = runExpr10(e.childs[0].second);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto kind = e.childs[i].first;
        auto v = runExpr10(e.childs[i].second);

        if (isNum(ret.type) && isNum(v.type)) {
            ll a, b, c;
            a = toNum(ret);
            b = toNum(v);
            if (kind == MUL)
                c = (a * b);
            else if (kind == DIV)
                c = (a / b);
            else
                c = (a % b);

            ret.type = tNum;
            ret.ptr = sAlloc(c);
        }
        else if (isByte(ret.type) && isByte(v.type)) {
            char a, b, c;
            a = toByte(ret);
            b = toByte(v);
            if (kind == MUL)
                c = (a * b);
            else if (kind == DIV)
                c = (a / b);
            else
                c = (a % b);

            ret.type = tByte;
            ret.ptr = sAlloc(c);
        }
        else {
            panic("runExpr9: type mismatch");
        }
    }

    return ret;
}
Pointer runExpr10(Expr10 e) {
    auto init = runExpr11(e.child);
    if (e.oper.size() == 0) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 0; i < e.oper.size(); i++) {
        auto kind = e.oper[i].first;
        auto type = e.oper[i].second;

        if (kind == NIL) {
            if (isPriType(ret.type)) panic("runExpr10: var is primitive type");

            ret.type = tBool;
            ret.ptr = sAlloc(access(ret) == 0);
        }
        else if (kind == VOID) {
            if (isPriType(ret.type)) panic("runExpr10: var is primitive type");

            ret.type = tBool;
            ret.ptr = sAlloc(access(ret) != 0);
        }
        else if (kind == LNOT) {
            ret.type = tBool;
            ret.ptr = sAlloc(!isAssAble(ret.type, type));
        }
        else if (kind == IS) {
            ret.type = tBool;
            ret.ptr = sAlloc(isAssAble(ret.type, type));
        }
        else {
            ret = converse(ret, type);
        }
    }

    return ret;
}
Pointer runExpr11(Expr11 e) {
    auto init = runExpr12(e.child);
    if (e.oper.size() == 0) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 0; i < e.oper.size(); i++) {
        auto kind = e.oper[i];

        if (kind == NOT) {
            ll x;
            if (isNum(ret.type)) {
                x = toNum(ret);
                ret.type = tNum;
            }
            else if (isByte(ret.type)) {
                x = toByte(ret);
                ret.type = tByte;
            }
            else {
                panic("runExpr11: type mismatch");
            }

            ret.ptr = sAlloc(~x);
        }
        else if (kind == LNOT) {
            if (!isBool(ret.type)) panic("runExpr11: type mismatch");

            bool b = toBool(ret);

            ret.type = tBool;
            ret.ptr = sAlloc((b) ? false : true);
        }
        else {
            ll x;
            if (isNum(ret.type)) {
                x = toNum(ret);
            }
            else if (isByte(ret.type)) {
                x = toByte(ret);
            }
            else {
                panic("runExpr11: type mismatch");
            }

            ret.type = tNum;
            ret.ptr = sAlloc(-x);
        }
    }

    return ret;
}
Pointer runExpr12(Expr12 e) {
    auto init = runFactor(e.f);
    if (e.childs.size() == 0) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 0; i < e.childs.size(); i++) {
        auto kind = e.childs[i].first;
        auto ptr = e.childs[i].second;

        if (kind == OBR) {
            ret = runFCall(ret, *(FCall*)ptr);
        }
        else if (kind == OSB) {
            ret = runIdx(ret, *(Idx*)ptr);
        }
        else {
            ret = runMember(ret, *(Word*)ptr);
        }
    }

    return ret;
}
Pointer runFactor(Factor f) {
    Pointer ret;

    switch (f.kind) {
    case OBR:
        ret = runExpr(*(Expr*)f.ptr);
        break;
    case LNUM:
        ret = Pointer{ tNum, false, sAlloc(((Number*)f.ptr)->value) };
        break;
    case LBYTE:
        ret = Pointer{ tByte, false, sAlloc(((ByteNumber*)f.ptr)->value) };
        break;
    case LREAL:
        ret = Pointer{ tReal, false, sAlloc(realBits(((Real*)f.ptr)->value)) };
        break;
    case LSTR:
        ret = makeString(*(LiteralString*)f.ptr);
        break;
    case OBL:
        ret = makeInstance(*(LiteralObject*)f.ptr);
        break;
    case WORD:
        ret = findLVar(((Word*)f.ptr)->word);
        if (ret.ptr == 0) {
            ret = findGVar(((Word*)f.ptr)->word);
        }

        if (ret.ptr == 0) {
            panicf("runFactor: cannot find var %s", ((Word*)f.ptr)->word);
        }

        break;
    case OSB:
        ret = makeArray(*(LiteralArray*)f.ptr);
        break;
    case LTRUE:
        ret = Pointer{ tBool, false, sAlloc(true) };
        break;
    case LFALSE:
        ret = Pointer{ tBool, false, sAlloc(false) };
        break;
    case THIS:
        ret = proc.STACK.back()._this;
        if (ret.ptr == 0) {
            panic("runFactor: cannot use \"this\"");
        }
        break;
    case FUNC:
        ret = makeClosure(*(int*)f.ptr, proc.STACK.back()._this);
        break;
    case NIL:
        ret = nil;
        break;
    }
    return ret;
}
Pointer converse(Pointer x, Type type) {
    if (isSameType(x.type, type)) {
        return Pointer{ x.type, false, sAlloc(sAccess(x.ptr)) };
    }
}
Pointer runMember(Pointer ptr, Word member) {
    if (findClass(ptr.type.name) != -1) {
        if (access(ptr) == 0) { nullPointerException(); }
        auto ins = toInstance(ptr);
        ptr = pAccess(ptr);

        if (ins.data.size() != ins.cl.field.size()) panic("runMember: wrong data");
        for (int i = 0; i < ins.cl.field.size(); i++) {
            if (strcmp(ins.cl.field[i].name, member.word) == 0) {
                Pointer ret = ptr;

                ret.type = ins.cl.field[i].type;
                ret.ptr = hAccess(ret.ptr);
                ret.ptr = pAdd(ret.ptr, i);
                return ret;
            }
        }
        for (int i = 0; i < ins.cl.method.size(); i++) {
            if (strcmp(ins.cl.method[i].name, member.word) == 0) {
                return makeClosure(ins.cl.method[i].idx, pAlloc(ptr));
            }
        }
        panicf("runMember: it has no member %s", member.word);
    }
    else if(findInterface(ptr.type.name) != -1) {
        auto iins = toIInstance(ptr);
        ptr = pAccess(ptr);

        for (int i = 0; i < iins.in.method.size(); i++) {
            if (strcmp(iins.in.method[i].name, member.word) == 0) {
                Pointer ret = ptr;

                ret = pAccess(ret);
                ret.ptr = hAccess(pAdd(ret.ptr, i + 2));

                return pAlloc(ret);
            }
        }

        panicf("runMember: it has no member %s", member.word);
    }

    panicf("runMember: wrong type %s", ptr.type.name);
}
Pointer makeString(LiteralString str) {
    int len = strlen(str.str);
    std::vector<ll> data;

    for (int i = 0; i < len; i++) data.push_back(str.str[i]);

    ll ptr = hAlloc(3);
    ll ptr2 = hAlloc(data.size());

    for (int i = 0; i < len; i++) {
        hAccess(pAdd(ptr2, i)) = data[i];
    }

    hAccess(ptr) = ptr2;
    hAccess(pAdd(ptr, 1)) = data.size();
    hAccess(pAdd(ptr, 2)) = data.size();

    proc.STACK.back().tmp.push_back(pAlloc(Pointer{ tStr, false, ptr }));
    return pAlloc(Pointer{ tStr, false, ptr });
}
Pointer runIdx(Pointer ptr, Idx idx) {
    auto id = runExpr(idx.id);
    if (!isNum(id.type)) {
        panic("runIdx: wrong index type");
    }

    ll i = toNum(id);
    Array arr = toArray(ptr);

    if (!(0 <= i && i < arr.len)) {
        panic("runIdx: index out of range");
    }

    ll p = hAccess(access(ptr));
    ll p2 = pAdd(p, i);

    return Pointer{ arr.eType, ptr.lv, p2 };
}
int findNF(const char* name) {
    for (int i = 1; i < _nf.size(); i++) {
        if (strcmp(_nf[i].name, name) == 0) {
            return i;
        }
    }
    return 0;
}
void clear() {
    for (int i = 0; i < proc.STACK.back().tmp.size(); i++) {
        dref(pAccess(proc.STACK.back().tmp[i]));
    }
    proc.STACK.back().tmp.clear();

    while (proc.ESP < mem.size() / 2) {
        mem.pop_back();
    }
}
void cleanScope() {
    clear();

    for (int i = 0; i < proc.STACK.back().vars.back().size(); i++) {
        auto duo = proc.STACK.back().vars.back()[i];
        dref(pAccess(duo.second));
    }
}
void cleanFrame() {
    for (int i = 0; i < proc.STACK.back().vars.size(); i++) {
        auto list = proc.STACK.back().vars[i];
        for (int j = 0; j < list.size(); j++) {
            auto duo = list[j];
            dref(pAccess(duo.second));
        }
    }
}

void enterScope() {
    proc.STACK.back().vars.emplace_back();
}
void exitScope() {
    cleanScope();
    proc.STACK.back().vars.pop_back();
}
void memStat() {
    static int h = 0;
    static int s = 0;

    bool changed = false;
    if (h < hmem.size()) {
        h = hmem.size();
        changed = true;
    }
    if (s < mem.size()) {
        s = mem.size();
        changed = true;
    }

    if (changed) {
        puts("--- mem size ---");
        printf("heap: %d\n", hmem.size());
        printf("stack: %d\n", mem.size());
        puts("----------------");
    }
}
void objStat() {
    int p = 1;
    puts("--- obj status ---");
    while (p < hmem.size()) {
        int cnt = hmem[p];
        int size = hmem[p + 1];

        if (cnt != 0) {
            printf("[%d] %d : (", cnt, p + 2);
            for (int i = 0; i < size - 2; i++) {
                printf("%d, ", hmem[p + i + 2]);
            }
            printf(")\n");
        }

        p += size;
    }
    puts("\n----------------");
}
void varStat() {
    for (int i = 0; i < proc.STACK.back().vars.size(); i++) {
        auto list = proc.STACK.back().vars[i];
        for (int j = 0; j < proc.STACK.back().vars[i].size(); j++) {
            auto var = proc.STACK.back().vars[i][j];

            printf("%s: ", var.first);
            if (isPriType(var.second.type)) {
                printf("%d\n", access(var.second));
            }
            else {
                if (var.second.type.kind == OSB) {
                    ll ptr = access(var.second);
                    if (ptr == 0) {
                        printf("null\n");
                        continue;
                    }

                    ll len = hAccess(pAdd(ptr, 1));
                    ll data = hAccess(ptr);

                    printType(var.second.type);
                    printf(" [ ");
                    for (int k = 0; k < len; i++) {
                        printf("%d, ", hAccess(pAdd(data, k)));
                    }
                    printf(" ]\n");
                }
                else if (var.second.type.kind == WORD) {
                    if (findClass(var.second.type.name) != -1) {
                        ll ptr = access(var.second);
                        if (ptr == 0) {
                            printf("null\n");
                            continue;
                        }

                        int cid = hAccess(pAdd(ptr, 1));
                        DefClass cl = _class[cid];
                        printf("class %s { ", cl.name);

                        ll data = hAccess(ptr);
                        for (int k = 0; k < cl.field.size(); k++) {
                            printf("%s : %d, ", cl.field[k].name, hAccess(pAdd(data, k)));
                        }
                        printf("}\n");
                    }
                    else {
                        int iid = findInterface(var.second.type.name);
                        DefInterface in = _interface[iid];
                        printf("INTERFACE %s at %d", in.name, -access(var.second));
                    }
                }
                else if (var.second.type.kind == FUNC) {
                    printf("FUNC at %d\n", -access(var.second));
                }
            }
        }
    }
}
#endif