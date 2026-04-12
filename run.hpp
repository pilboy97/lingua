#ifndef __RUN_HPP__
#define __RUN_HPP__

#include "procStruct.hpp"
#include "panic.hpp"

#include <vector>
#include <algorithm>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include <cstdint>

#define ll long long
#define INIT_MEM_SIZE 1024

bool DEBUG = false;
bool NOCLR = false;

typedef std::string string;

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
    ll org;
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

    std::vector<std::vector<DeferStmt>> defer;
    std::vector<Pointer> tmp;
    //std::vector<std::vector<std::pair<string, Pointer>>> vars;

    std::vector<std::unordered_map<string, Pointer>> vars;
};

typedef Pointer(*func)(std::vector<Pointer> args, jmp_buf jmp);
typedef void(*act)(Pointer ptr);

struct NFunc {
    Type type;
    string name;
    func fn;
    ll ptr;
};
struct Proc {
    ll SP;
    ll ESP;
    
    ll ERR;

    std::vector<Frame> STACK;
};

ll GP;
std::vector<ll> mem;
std::vector<ll> hmem;
std::vector<std::pair<int, int>> blocks;

std::unordered_map<string, int32_t> _class_cache;
std::vector<DefClass> _class;

std::unordered_map<string, int> _interface_cache;
std::vector<DefInterface> _interface;

std::unordered_map<string, int> _fn_cache;
std::vector<DefFunc> _fn;

std::vector<NFunc> _nf;

Type tVoid = Type{ 0 };
Type tAny = Type{ -1 };
Type tNum = Type{ NUM };
Type tByte = Type{ BYTE };
Type tBool = Type{ BOOL };
Type tReal = Type{ REAL };
Type tStr = Type{ OSB, "", {tByte} };
Type tError = { WORD,"Error",{} };

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
string toStr(Pointer ptr);
bool isStr(Type type);

std::vector<int> getSupers(int cid);
bool isSuper(int cid, int scid);

Pointer makeClosure(int fid, Pointer _this);
Pointer makeArray(LiteralArray arr, jmp_buf jmp);
Pointer makeInstance(LiteralObject obj, jmp_buf jmp);
Pointer makeString(LiteralString str);

Pointer converse(Pointer x, Type type);

void memStat();
void objStat();
void varStat();

void viewErr();
void raiseErrDivideByZero(jmp_buf jmp);
void raiseErrNullPointer(jmp_buf jmp);

void initProc();

void run(Program prog);

void runStmt(Stmt stmt, jmp_buf jmp);
void runBlockEnd(jmp_buf jmp);
void runBlockStmt(BlockStmt stmt, jmp_buf jmp);
void runIfStmt(IfStmt stmt, jmp_buf jmp);
void runForStmt(ForStmt stmt, jmp_buf jmp);
void runRetStmt(RetStmt stmt, jmp_buf jmp);

Pointer runFCall(Pointer ptr, FCall args, jmp_buf jmp);
Pointer runIdx(Pointer ptr, Idx idx, jmp_buf jmp);
Pointer runMember(Pointer ptr, Word word);

Pointer runExpr_1(Expr_1 e, jmp_buf jmp);
Pointer runExpr(Expr e, jmp_buf jmp);
Pointer runExpr1(Expr1 e, jmp_buf jmp);
Pointer runExpr2(Expr2 e, jmp_buf jmp);
Pointer runExpr3(Expr3 e, jmp_buf jmp);
Pointer runExpr4(Expr4 e, jmp_buf jmp);
Pointer runExpr5(Expr5 e, jmp_buf jmp);
Pointer runExpr6(Expr6 e, jmp_buf jmp);
Pointer runExpr7(Expr7 e, jmp_buf jmp);
Pointer runExpr8(Expr8 e, jmp_buf jmp);
Pointer runExpr9(Expr9 e, jmp_buf jmp);
Pointer runExpr10(Expr10 e, jmp_buf jmp);
Pointer runExpr11(Expr11 e, jmp_buf jmp);
Pointer runExpr12(Expr12 e, jmp_buf jmp);
Pointer runFactor(Factor f, jmp_buf jmp);

void defVar(DefVar var, jmp_buf jmp);
void defFunc(int fid);
void defClass(DefClass cl);
void defInterface(DefInterface in);

bool isAssAble(Type dst, Type src);
void assign(Pointer dst, Pointer src);

void nullPointerException();

void newGVar(string name, Pointer ptr);
void newLVar(string name, Pointer ptr);

Pointer findLVar(string name);
Pointer findGVar(string name);
int findNF(string name);
int findClass(string name);
int findInterface(string name);

string strAdd(string str, string str2);
string strAdd(string str, ll x);
string strAdd(string str, char x);
string strAdd(string str, bool x);
string strAdd(string str, double x);

void clear();
void cleanScope();
void cleanFrame();

void enterScope();
void exitScope();


void ref(Pointer ptr) {
    if (NOCLR) return;
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
        auto& fn = _fn[cl.fid];

        ll p = hAccess(ptr.ptr);
        if (p == 0) return;
        for (int i = 0; i < cl.cap.size(); i++) {
            ref(Pointer{ fn.cType[i], false, hAccess(pAdd(p, i)) });
        }
    }
    else if (isObject(ptr.type)) {
        if (findClass(ptr.type.name) != -1) {
            auto in = toInstance(pAlloc(ptr));
            auto& cl = in.cl;

            ll p = hAccess(ptr.ptr);
            if (p == 0) return;
            for (int i = 0; i < cl.field.size(); i++) {
                ref(Pointer{ cl.field[i].type, false, hAccess(pAdd(p, i)) });
            }
        }
        else if (findInterface(ptr.type.name) != -1) {
            auto iin = toIInstance(pAlloc(ptr));
            auto& in = iin.in;
            auto& cl = iin.cl;

            ll p = hAccess(ptr.ptr);
            if (p == 0) return;

            ref(Pointer{ Type{WORD, cl.name, {}}, false, hAccess(pAdd(p, 1)) });
            for (int i = 0; i < in.method.size(); i++) {
                ref(Pointer{ Type{FUNC}, false, hAccess(pAdd(p, i + 2)) });
            }
        }
    }
    else panic("ref: wrong type");
}
void dref(Pointer ptr) {
    if (NOCLR) return;
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
        auto& fn = _fn[cl.fid];

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
    else if (isObject(ptr.type)) {
        if (findClass(ptr.type.name) != -1) {
            auto in = toInstance(pAlloc(ptr));
            auto& cl = in.cl;

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
        else if (findInterface(ptr.type.name) != -1) {
            auto iin = toIInstance(pAlloc(ptr));
            auto& in = iin.in;
            auto& cl = iin.cl;

            ll p = hAccess(ptr.ptr);
            if (p == 0) return;
            dref(Pointer{ Type{WORD, cl.name, {}}, false, hAccess(pAdd(p, 1)) });
            for (int i = 0; i < in.method.size(); i++) {
                dref(Pointer{ Type{FUNC}, false, hAccess(pAdd(p, i + 2)) });
            }
            for (int i = 0; i < in.method.size(); i++) {
                hAccess(pAdd(p, i)) = 0;
            }

            hAccess(pAdd(p, -2)) = 0;
        }
    }
    else panic("dref: wrong type");
}

bool isImplOf(DefInterface x, DefClass y) {
    for (int j = 0; j < x.method.size(); j++) {
        auto& m2 = x.method[j];
        auto& f2 = x.method[j];
        bool found = false;
        for (int i = 0; i < y.method.size(); i++) {
            auto& m = y.method[i];
            auto& f = _fn[m.idx];

            if (m.name == m2.name) {
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

string toStr(Pointer x) {
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

Pointer nfPrint(std::vector<Pointer> args, jmp_buf jmp) {
    Pointer ptr = args[0];
 
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
Pointer nflen(std::vector<Pointer> args, jmp_buf jmp) {
    if (args.size() != 1) panic("len: argument mismatch");

    Pointer ptr = args[0];

    if (!isArray(ptr.type)) panic("len: argument mismatch: not Array");

    ptr = pAccess(ptr);
    ll len = hAccess(pAdd(ptr.ptr, 1));

    return Pointer{tNum, false, sAlloc(len)};
}
Pointer nfcap(std::vector<Pointer> args, jmp_buf jmp) {
    if (args.size() != 1) panic("cap: argument mismatch");

    Pointer ptr = args[0];

    if (!isArray(ptr.type)) panic("cap: argument mismatch: not Array");

    ptr = pAccess(ptr);
    ll cap = hAccess(pAdd(ptr.ptr, 2));

    return Pointer{tNum, false, sAlloc(cap)};
}
Pointer nfappend(std::vector<Pointer> args, jmp_buf jmp) {
    if (args.size() != 2) panic("append: argument mismatch");

    Pointer ptr = args[0];
    Pointer elem = args[1];

    elem = converse(elem, ptr.type.add[0]);
    ref(pAccess(elem));

    if (!isArray(ptr.type)) panic("append: argument mismatch: not Array");
    if (!isAssAble(ptr.type.add[0], elem.type)) panic("append: argument mismatch: type mismatch");

    Array arr = toArray(ptr);
    int len = arr.len;
    int cap = arr.cap;

    ptr = pAccess(ptr);
    if (len + 1 >= cap) {
        hAccess(pAdd(ptr.ptr, 1)) = len + 1;
        hAccess(pAdd(ptr.ptr, 2)) = (cap == 0) ? 2 : cap * 2;

        ll data = hAlloc((cap == 0) ? 2 : cap * 2);
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
Pointer nfshrink(std::vector<Pointer> args, jmp_buf jmp) {
    if (args.size() != 1) panic("shrink: argument mismatch");

    Pointer ptr = args[0];
    Array arr = toArray(ptr);

    ptr = pAccess(ptr);

    if (!isArray(ptr.type)) panic("shrink: argument mismatch: not Array");

    ll ndata = hAlloc(arr.len);
    for(int i = 0;i < arr.len;i++) {
        hAccess(pAdd(ndata, i)) = arr.data[i];
    }

    hAccess(ptr.ptr) = ndata;
    hAccess(pAdd(ptr.ptr, 2)) = arr.len;
    return nil;
}
Pointer nfRaise(std::vector<Pointer> args, jmp_buf jmp) {
    Pointer err = args[0];
    int iid = findInterface("Error");
    if (iid == -1) panic("cannot find interface Error");


    if (!isAssAble(tError, err.type)) {
        panic("error should implements Error");
    }

    err = converse(err, tError);
    proc.ERR = access(err);
    ref(pAccess(err));

    longjmp(jmp, -1);

    return nil;
}
Pointer nfRecover(std::vector<Pointer> args, jmp_buf jmp) {
    if (proc.ERR == 0) return nil;
    Pointer err = Pointer{ tError, false, sAlloc(proc.ERR) };

    proc.STACK.back().tmp.push_back(err);

    proc.ERR = 0;
    return err;
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
    hAccess(pAdd(ptr, 1)) = ptr2;
    
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
    mem = std::vector<ll>(INIT_MEM_SIZE + 1);
    hmem = std::vector<ll>(INIT_MEM_SIZE + 3);

    hmem[1] = 0;
    hmem[2] = INIT_MEM_SIZE + 2;

    blocks.emplace_back(1, INIT_MEM_SIZE + 2);

    initProc();

    ll pPrint = makeClosure(-1, nil).ptr;
    ll pLen = makeClosure(-2, nil).ptr;
    ll pCap = makeClosure(-3, nil).ptr;
    ll pAppend = makeClosure(-4, nil).ptr;
    ll pShrink = makeClosure(-5, nil).ptr;
    ll pRaise = makeClosure(-6, nil).ptr;
    ll pRecover = makeClosure(-7, nil).ptr;

    _fn = prog.fn;
    _nf.push_back(NFunc{});
    _nf.push_back(NFunc{Type{FUNC, "", {tVoid,tStr}}, "print", nfPrint, pPrint});
    _nf.push_back(NFunc{ Type{FUNC, "", {tNum, tAny}}, "len", nflen, pLen });
    _nf.push_back(NFunc{ Type{FUNC, "", {tNum, tAny}}, "cap", nfcap, pCap });
    _nf.push_back(NFunc{ Type{FUNC, "", {tVoid, tAny, tAny}}, "append", nfappend, pAppend });
    _nf.push_back(NFunc{ Type{FUNC, "", {tVoid, tAny}} , "shrink", nfshrink, pShrink});
    _nf.push_back(NFunc{ Type{FUNC, "", {tVoid, tError}} , "raise", nfRaise, pRaise });
    _nf.push_back(NFunc{ Type{FUNC, "", {tError}} , "recover", nfRecover, pRecover });

    for (int i = 0; i < prog.childs.size(); i++) {
        auto& duo = prog.childs[i];
        auto& kind = duo.first;
        auto& def = duo.second;

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
    int jv;
    if ((jv = setjmp(jmp)) == 0) {
        runFCall(findGVar("main"), FCall{}, jmp);
    }
    else {
        viewErr();
    }
}

void initProc() {
    proc.SP = 1;
    proc.ESP = 2;
    
    proc.STACK.clear();
    proc.STACK.push_back(Frame{});
    proc.STACK.back().vars.push_back(std::unordered_map<string, Pointer>());
}
void debug() {
    objStat();
    varStat();
}
void defVar(DefVar var, jmp_buf jmp) {
    auto& name = var.name;
    if (findLVar(name).ptr != 0) panicf("defVar: %s is already exists", name.c_str());

    auto& vars = proc.STACK.back().vars;

    if (var.init == NULL && var.type == NULL) {
        panic("defVar: cannot decide variable type");
    }

    if (var.init == NULL) {
        Pointer ptr = { *var.type, true, sAlloc(0) };
        newLVar(name, ptr);
        return;
    }

    if (var.type == NULL) {
        Pointer ptr = runExpr(*var.init, jmp);
        Pointer ret = { ptr.type, ptr.lv, sAlloc(0) };
        assign(ret, ptr);
        newLVar(name, ret);
        return;
    }

    Pointer ptr = runExpr(*var.init, jmp);
    if (!isAssAble(*var.type, ptr.type)) {
        panic("defVar: cannot initialize variable: type mismatch");
    }

    Pointer ret = Pointer{ *var.type, true, sAlloc(1) };
    assign(ret, ptr);
    newLVar(name, ret);
}
bool isAssAble(Type dst, Type src) {
    if(dst.kind == -1) return true;
    if (isPriType(dst) && isPriType(src)) return true;
    if (isSameType(dst, src)) return true;
    
    int cid = findClass(src.name);
    int cid2 = findClass(dst.name);
    int iid = findInterface(dst.name);
    
    if(cid != -1 && cid2 != -1) {
        auto supers = getSupers(cid);
        for(int i = 0;i < supers.size();i++) {
            if(supers[i] == cid2) {
                return true;
            }
        }
        return false;
    }
    else if(iid != -1 && cid != -1) {
        return isImplOf(_interface[iid], _class[cid]);
    }

    return false;
}
void assign(Pointer dst, Pointer src) {
    if (!isAssAble(dst.type, src.type))
        panic("assign: cannot assign: type mismatch");

    dref(pAccess(dst));
    ref(pAccess(src));

    if (isSameType(dst.type, src.type)) {
        access(dst) = access(src);
    }
    else {
        Pointer neo = converse(src, dst.type);
        
        if (access(neo) == 0) panic("assign: cannot converse dst type");

        access(dst) = access(neo);
    }
}
Pointer findLVar(string name) {
    if (name.length() == 0) return nil;

    auto& frame = proc.STACK.back().vars;
    // for (int i = frame.size() - 1; i >= 0; i--) {
    //     for (int j = 0; j < frame[i].size(); j++) {
    //         auto& duo = frame[i][j];
    //         auto& vname = duo.first;
    //         auto& vptr = duo.second;

    //         if (strcmp(name, vname) == 0) {
    //             return vptr;
    //         }
    //     }

    for (int i = frame.size() - 1; i >= 0; i--) {
        auto it = frame[i].find(name);
        if (it == frame[i].end()) continue;

        return it->second;
    }

    return nil;
}
Pointer findGVar(string name) {
    if (name.length() == 0) return nil;

    auto& frame = proc.STACK.front().vars;
    for (int i = frame.size() - 1; i >= 0; i--) {
        auto it = frame[i].find(name);
        if (it == frame[i].end()) continue;

        return it->second;
    }

    int fid = findNF(name);
    if (fid > 0)
        return Pointer{ _nf[fid].type, false, _nf[fid].ptr};
    else return nil;
}
int findClass(string name) {
    if (name.length() == 0) return -1;

    if (_class_cache.find(name) != _class_cache.end()) return _class_cache[name];

    for (int i = 0; i < _class.size(); i++) {
        if (_class[i].name == name) {
            _class_cache.insert(std::make_pair(name, i));

            return i;
        }
    }
    return -1;
}
int findInterface(string name) {
    if (name.length() == 0) return -1;

    if (_interface_cache.find(name) != _interface_cache.end()) return _interface_cache[name];

    for (int i = 0; i < _interface.size(); i++) {
        if (_interface[i].name == name) {
            _interface_cache.insert(std::make_pair(name, i));

            return i;
        }
    }
    return -1;
}

void newGVar(string name, Pointer ptr) {
    if (findGVar(name).ptr != 0) panicf("newGVar: %s is already exists", name.c_str());
    if(findClass(name) != -1)  panicf("newGVar: %s is already exists", name.c_str());
    if (findInterface(name) != -1)  panicf("newGVar: %s is already exists", name.c_str());

    auto& frame = proc.STACK.front();
    frame.vars.back().insert(std::make_pair(name, Pointer{ ptr.type, true, ptr.ptr }));
    ref(pAccess(ptr));
}
void newLVar(string name, Pointer ptr) {
    if (findLVar(name).ptr != 0) panicf("newLVar: %s is already exists", name.c_str());

    auto& frame = proc.STACK.back();
    frame.vars.back().insert(std::make_pair(name, Pointer{ ptr.type, true, ptr.ptr }));
    ref(pAccess(ptr));
}
void defClass(DefClass def) {
    if (findGVar(def.name).ptr != 0) panicf("defClass: %s is already exists", def.name.c_str());
    if (findClass(def.name) != -1)  panicf("defClass: %s is already exists", def.name.c_str());
    if (findInterface(def.name) != -1)  panicf("defClass: %s is already exists", def.name.c_str());

    _class.push_back(def);
    _class_cache.insert(std::make_pair(def.name, _class.size() - 1));
}
void defInterface(DefInterface def) {
    if (findGVar(def.name).ptr != 0) panicf("defInterface: %s is already exists", def.name.c_str());
    if (findClass(def.name) != -1)  panicf("defInterface: %s is already exists", def.name.c_str());
    if (findInterface(def.name) != -1)  panicf("defInterface: %s is already exists", def.name.c_str());

    _interface.push_back(def);
    _interface_cache.insert(std::make_pair(def.name, _interface.size() - 1));
}
void defFunc(int fid) {
    DefFunc& def = _fn[fid];

    if (findGVar(def.name).ptr != 0) panicf("defFunc: %s is already exists", def.name.c_str());
    if (findClass(def.name) != -1)  panicf("defFunc: %s is already exists", def.name.c_str());
    if (findInterface(def.name) != -1)  panicf("defFunc: %s is already exists", def.name.c_str());

    Pointer cl = makeClosure(fid, nil);

    newGVar(def.name, cl);
}

void runStmt(Stmt stmt, jmp_buf jmp) {
    int ESP = proc.ESP;

    if (DEBUG) {
        puts("");
        puts("------------");
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
        defVar(*(DefVar*)stmt.stmt, jmp);
        ESP = proc.ESP;
        break;
    case SCOLON:
        runExpr_1(*(Expr_1*)stmt.stmt, jmp);
        break;
    case DEFER:
        proc.STACK.back().defer.back().push_back(*(DeferStmt*)stmt.stmt);
        break;
    }
    clear();
    proc.ESP = ESP;
    if (DEBUG) {
        debug();
        printf("END STATEMENT\n");
        puts("------------");
        puts("");
    }
}
void runBlockEnd(jmp_buf jmp) {
    for(int i = proc.STACK.back().defer.back().size() - 1;i >= 0;i--) {
        auto& block = proc.STACK.back().defer.back()[i].block;

        jmp_buf jp;
        int jv;
        
        if((jv = setjmp(jp)) == 0) {
            runBlockStmt(block, jp);
        }
        else if(jv == 1) {
            panic("runBlockEnd: wrong return"); 
        }
        else if(jv == 2) {
            panic("runBlockEnd: wrong break");
        }
        else if(jv == 3) {
            panic("runBlockEnd: wrong continue");
        }
        else {
            longjmp(jmp, -1);
        }
    }
}

// void runBlockStmt(BlockStmt stmt, jmp_buf jmp) {
//     enterScope();
//     for (int i = 0; i < stmt.childs.size(); i++) {
//         auto& s = stmt.childs[i];
        
//         jmp_buf jp;
//         int jv;

//         if((jv = setjmp(jp)) == 0)
//             runStmt(s, jp);
//         else {
//             runBlockEnd(jmp);
//             exitScope();
//             if(jv != -1)
//                 longjmp(jmp, jv);
//             else {
//                 if (proc.ERR != 0)
//                     longjmp(jmp, -1);
//             }
//             return;
//         }
//     }
//     runBlockEnd(jmp);
//     exitScope();
// }

void runBlockStmt(BlockStmt stmt, jmp_buf jmp) {
    enterScope();
    
    jmp_buf jp;
    int jv;
    int i = 0;
    
    if ((jv = setjmp(jp)) == 0) {
        // 정상 실행: 모든 statement를 jp 하나로 처리
        for (; i < stmt.childs.size(); i++) {
            runStmt(stmt.childs[i], jp);
        }
    }
    else {
        // break/continue/return/error 발생
        runBlockEnd(jmp);
        exitScope();
        longjmp(jmp, jv);
        return;
    }
    
    runBlockEnd(jmp);
    exitScope();
}

void runIfStmt(IfStmt stmt, jmp_buf jmp) {
    for (int i = 0; i < stmt._if.size(); i++) {
        auto& cond = stmt._if[i].first;
        auto& body = stmt._if[i].second;

        if (toBool(runExpr(cond, jmp))) {
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
        runExpr_1(*stmt.init, jmp);
    }

    while (true) {
        if (stmt.cond != NULL && !toBool(runExpr_1(*stmt.cond, jmp))) {
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
            //continue;
        }

        if (stmt.act != NULL) {
            runExpr_1(*stmt.act, jmp);
        }
    }
}
Pointer runFCall(Pointer ptr, FCall args, jmp_buf jmp) {
    auto fn = toClosure(ptr);
    ptr = pAccess(ptr);

    std::vector<Pointer> arglist;

    for (int i = 0; i < args.list.size(); i++) {
        arglist.push_back(runExpr(args.list[i], jmp));
    }

    if (fn.fid >= 0) {
        auto& def = _fn[fn.fid];

        std::vector<ll> alist;
        std::vector<ll> clist;

        if (def.frame.size() != args.list.size()) panic("runFCall: arguments mismatch");

        for (int i = 0; i < args.list.size(); i++) {
            ll ptr = access(converse(arglist[i], def.frame[i].second));
            if (!isPriType(def.frame[i].second) && ptr == 0)
                panic("runFCall: arguments mismatch");

            alist.push_back(ptr);
        }

        proc.STACK.push_back(Frame());
        proc.STACK.back().vars.emplace_back();
        proc.STACK.back().defer.emplace_back();

        proc.STACK.back().rType = def.ret;

        sAlloc(proc.SP);
        proc.SP = proc.ESP - 1;

        proc.STACK.back()._this = Pointer{ def.cType[0], true, sAlloc(fn.cap[0]) };

        if (fn.cap.size() != def.captured.size() + 1) panic("runFCall: cannot call function: captured mismatch");

        for (int i = 1; i < fn.cap.size(); i++) {
            auto& c = fn.cap[i];
            auto& f = def;

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

            longjmp(jp, 1);
        }
        else if (jv == 1) {
            Pointer ret;

            if (proc.STACK.back().rType.kind != 0) {
                auto& type = proc.STACK.back().rType;
                auto& value = proc.STACK.back().rValue;

                ret = Pointer{ type, false, value };
            }
            else {
                ret = nil;
            }

            cleanFrame();

            proc.ESP = proc.SP;
            proc.SP = mem[proc.SP];
            proc.STACK.pop_back();

            if (ret.ptr != 0) {
                ret = pAlloc(ret);
                proc.STACK.back().tmp.push_back(ret);
            }
            
            return ret;
        }
        else if (jv == 2) {
            panic("runFCall: unexpected break");
        }
        else if (jv == 3){
            panic("runFCall: unexpected continue");
        }
        else {
            longjmp(jmp, -1);
        }

        return nil;
    }
    else {
        NFunc nf = _nf[-fn.fid];
        if (nf.type.add.size() - 1 != arglist.size()) {
            panic("runFCall: argument mismatch");
        }
        for (int i = 1; i < nf.type.add.size(); i++) {
            arglist[i - 1] = converse(arglist[i - 1], nf.type.add[i]);
        }
        return (*nf.fn)(arglist, jmp);
    }
}
void runRetStmt(RetStmt stmt, jmp_buf jmp) {
    Pointer ret;
    if (stmt.expr == NULL) {
        ret = nil;
    }
    else {
        ret = pAccess(runExpr(*stmt.expr, jmp));
    }
    
    if (!isAssAble(proc.STACK.back().rType, ret.type)) {
        panic("runRetStmt: type mismatch");
    }
    ret = converse(pAlloc(ret), proc.STACK.back().rType);
    proc.STACK.back().rValue = access(ret);

    ref(pAccess(ret));

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
string strAdd(string str, string str2) {
    return str+str2;
}
string strAdd(string str, ll x) {
    char buf[64];
    sprintf(buf, "%lld", x);

    return strAdd(str, string(buf));
}
string strAdd(string str, char x) {
    char buf[64];
    sprintf(buf, "%c", x);

    return strAdd(str, string(buf));
}
string strAdd(string str, bool x) {
    if (x)
        return str + string("true");
    else
        return str + string("false");
}
string strAdd(string str, double x) {
    char buf[64];
    sprintf(buf, "%f", x);

    return strAdd(str, string(buf));
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
        if (!(0 <= fid && fid < _fn.size()))
            panic("toClosure: wrong fid");

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
    if (!(0 <= iid && iid < _interface.size())) 
        panic("toIInstance: wrong iid");

    ret.in = _interface[iid];

    int p = hAccess(ptr.ptr);
    for (int i = 0; i < ret.in.method.size() + 2; i++) {
        ret.data.push_back(hAccess(pAdd(p, i)));
    }

    ret.cl = _class[ret.data[0]];
    ret.org = ret.data[1];

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
                panicf("makeClosure: cannot capture variable: cannot find %s", fn.captured[i].c_str());
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
Pointer makeArray(LiteralArray arr, jmp_buf jmp) {
    Pointer ret;
    
    ll ptr = hAlloc(3);
    ll ptr2;
    int cap;

    if(arr.elem.size() > 0) {
        ptr2 = hAlloc(arr.elem.size());
        cap = arr.elem.size();
    }
    else {
        ptr2 = hAlloc(1);
        cap = 1;
    }

    hAccess(ptr) = ptr2;
    hAccess(pAdd(ptr, 1)) = arr.elem.size();
    hAccess(pAdd(ptr, 2)) = cap;

    for (int i = 0; i < arr.elem.size(); i++) {
        hAccess(pAdd(ptr2, i)) = sAccess(runExpr(arr.elem[i], jmp).ptr);
    }

    ret.ptr = ptr;
    ret.lv = false;
    ret.type = Type{ OSB, "", {arr.etype} };
    ret = pAlloc(ret);
    proc.STACK.back().tmp.push_back(ret);
    return ret;
}
Pointer makeInstance(LiteralObject obj, jmp_buf jmp) {
    Pointer ret;
    int cid = findClass(obj.type.name);
    
    if (cid == -1) {
        panicf("makeInstance: cannot find class %s", obj.type.name.c_str());
    }

    int ptr = hAlloc(2);

    std::vector<ll> data;

    auto supers = getSupers(cid);

    for(int k = 0;k < supers.size();k++) {
        auto& cl = _class[supers[k]];
        for (int i = 0; i < cl.field.size(); i++) {
            auto& f1 = cl.field[i];
            bool found = false;
            for (int j = 0; j < obj.init.list.size(); j++) {
                auto& f2 = obj.init.list[j];

                if (f1.name == f2.first) {
                    found = true;
                    auto e = runExpr(f2.second, jmp);
                    if(f1.type.kind == WORD && e.type.kind == WORD && findInterface(f1.type.name) != -1 && findClass(e.type.name) != -1) {
                        Pointer ne = Pointer{f1.type, false, sAlloc(0)};
                        assign(ne, e);
                        
                        data.push_back(access(ne));
                    }
                    else {
                        data.push_back(access(e));
                    }

                    ref(pAccess(e));

                    break;
                }
            }
            if (!found) {
                data.push_back(0);
            }
        }
    }
    
    int p = hAlloc(data.size());

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

Pointer runExpr_1(Expr_1 e, jmp_buf jmp) {
    if (e.src == NULL) {
        return runExpr(e.dst, jmp);
    }

    auto src = runExpr(*e.src, jmp);
    auto dst = runExpr(e.dst, jmp);

    assign(dst, src);
    return nil;
}
Pointer runExpr(Expr e, jmp_buf jmp) {
    auto init = runExpr1(e.childs[0], jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    if (toBool(init)) {
        return pAlloc(Pointer{ tBool, false, sAlloc(true) });
    }

    for (int i = 1; i < e.childs.size(); i++) {
        if (toBool(runExpr1(e.childs[i], jmp))) {
            return pAlloc(Pointer{ tBool, false, sAlloc(true) });
        }
    }

    return Pointer{ tBool, false, sAlloc(false) };
}
Pointer runExpr1(Expr1 e, jmp_buf jmp) {
    auto init = runExpr2(e.childs[0], jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    if (!toBool(init)) {
        return Pointer{ tBool, false, sAlloc(false) };
    }

    for (int i = 1; i < e.childs.size(); i++) {
        if (!toBool(runExpr2(e.childs[i], jmp))) {
            return Pointer{ tBool, false, sAlloc(false) };
        }
    }

    return Pointer{ tBool, false, sAlloc(true) };
}
Pointer runExpr2(Expr2 e, jmp_buf jmp) {
    auto init = runExpr3(e.childs[0], jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto v = runExpr3(e.childs[i], jmp);

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
Pointer runExpr3(Expr3 e, jmp_buf jmp) {
    auto init = runExpr4(e.childs[0], jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto v = runExpr4(e.childs[i], jmp);

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
Pointer runExpr4(Expr4 e, jmp_buf jmp) {
    auto init = runExpr5(e.childs[0], jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto v = runExpr5(e.childs[i], jmp);

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
Pointer runExpr5(Expr5 e, jmp_buf jmp) {
    auto init = runExpr6(e.childs[0].second, jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto& kind = e.childs[i].first;
        auto v = runExpr6(e.childs[i].second, jmp);

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
Pointer runExpr6(Expr6 e, jmp_buf jmp) {
    auto init = runExpr7(e.childs[0].second, jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto& kind = e.childs[i].first;
        auto v = runExpr7(e.childs[i].second, jmp);

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
Pointer runExpr7(Expr7 e, jmp_buf jmp) {
    auto init = runExpr8(e.childs[0].second, jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto& kind = e.childs[i].first;
        auto v = runExpr8(e.childs[i].second, jmp);

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
Pointer runExpr8(Expr8 e, jmp_buf jmp) {
    auto init = runExpr9(e.childs[0].second, jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto& kind = e.childs[i].first;
        auto v = runExpr9(e.childs[i].second, jmp);

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
            string a = toStr(ret), b = toStr(v), c;
            c = strAdd(a, b);

            ret = makeString(LiteralString{ c });
        }
        else if (isStr(ret.type) && isNum(v.type)) {
            string a = toStr(ret), c;
            ll b = toNum(v);

            c = strAdd(a, b);

            ret = makeString(LiteralString{ c });
        }
        else if (isStr(ret.type) && isByte(v.type)) {
            string a = toStr(ret), c;
            char b = toByte(v);

            c = strAdd(a, b);

            ret = makeString(LiteralString{ c });
        }
        else if (isStr(ret.type) && isBool(v.type)) {
            string a = toStr(ret), c;
            bool b = toBool(v);

            c = strAdd(a, b);

            ret = makeString(LiteralString{ c });
        }
        else if (isStr(ret.type) && isReal(v.type)) {
            string a = toStr(ret), c;
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
Pointer runExpr9(Expr9 e, jmp_buf jmp) {
    auto init = runExpr10(e.childs[0].second, jmp);
    if (e.childs.size() == 1) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 1; i < e.childs.size(); i++) {
        auto& kind = e.childs[i].first;
        auto v = runExpr10(e.childs[i].second, jmp);

        if (isNum(ret.type) && isNum(v.type)) {
            ll a, b, c;
            a = toNum(ret);
            b = toNum(v);
            if (kind == MUL)
                c = (a * b);
            else if (kind == DIV) {
                if (b == 0) {
                    raiseErrDivideByZero(jmp);
                }
                c = (a / b);
            }
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
            else if (kind == DIV) {
                if (b == 0) {
                    raiseErrDivideByZero(jmp);
                }
                c = (a / b);
            }
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
Pointer runExpr10(Expr10 e, jmp_buf jmp) {
    auto init = runExpr11(e.child, jmp);
    if (e.oper.size() == 0) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 0; i < e.oper.size(); i++) {
        auto& kind = e.oper[i].first;
        auto& type = e.oper[i].second;

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
            if (isAssAble(ret.type, type)) {
                ret.ptr = sAlloc(true);
            }
            else if (converse(ret, type).type.kind != -1) {
                ret.ptr = sAlloc(true);
            }
            else {
                ret.ptr = sAlloc(false);
            }
        }
        else {
            ret = converse(ret, type);
        }
    }

    return ret;
}
Pointer runExpr11(Expr11 e, jmp_buf jmp) {
    auto init = runExpr12(e.child, jmp);
    if (e.oper.size() == 0) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 0; i < e.oper.size(); i++) {
        auto& kind = e.oper[i];

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
Pointer runExpr12(Expr12 e, jmp_buf jmp) {
    auto init = runFactor(e.f, jmp);
    if (e.childs.size() == 0) {
        return init;
    }

    auto ret = Pointer{ init.type, false, sAlloc(access(init)) };
    for (int i = 0; i < e.childs.size(); i++) {
        auto& kind = e.childs[i].first;
        auto& ptr = e.childs[i].second;

        if (kind == OBR) {
            ret = runFCall(ret, *(FCall*)ptr, jmp);
        }
        else if (kind == OSB) {
            ret = runIdx(ret, *(Idx*)ptr, jmp);
        }
        else {
            if (access(ret) == 0) {
                raiseErrNullPointer(jmp);
            }
            ret = runMember(ret, *(Word*)ptr);
        }
    }

    return ret;
}
Pointer runFactor(Factor f, jmp_buf jmp) {
    Pointer ret;

    int cid;

    switch (f.kind) {
    case OBR:
        ret = runExpr(*(Expr*)f.ptr, jmp);
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
        ret = makeInstance(*(LiteralObject*)f.ptr, jmp);
        break;
    case WORD:
        ret = findLVar(((Word*)f.ptr)->word);
        if (ret.ptr == 0) {
            ret = findGVar(((Word*)f.ptr)->word);
        }

        if (ret.ptr == 0) {
            panicf("runFactor: cannot find var %s", ((Word*)f.ptr)->word.c_str());
        }

        break;
    case OSB:
        ret = makeArray(*(LiteralArray*)f.ptr, jmp);
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
    case SUPER:
        ret = proc.STACK.back()._this;
        if (ret.ptr == 0) {
            panic("runFactor: cannot use super");
        }

        cid = findClass(ret.type.name);
        if(_class[cid].super.length() == 0) {
            panic("runFactor: this class has no super class");
        }

        ret.type.kind = WORD;
        ret.type.name = _class[cid].super;

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
    if (type.kind == -1)
        return Pointer{ x.type, false, x.ptr };
    if (isSameType(x.type, type)) {
        return Pointer{ x.type, false, x.ptr };
    }
    else if (isPriType(x.type) && isPriType(type)) {
        ll ret;
        if (x.type.kind == REAL) {
            double value = restore(access(x));

            ret = (ll)value;
        }
        else if (type.kind == REAL) {
            double value = access(x);

            ret = realBits(value);
        }
        else {
            ret = access(x);
        }

        return Pointer{ type, false, sAlloc(ret) };
    }
    else if (x.type.kind == WORD && type.kind == WORD) {
        int iid = findInterface(type.name);
        int iid2 = findInterface(x.type.name);
        int cid = findClass(x.type.name);
        int cid2 = findClass(type.name);

        if (cid != -1 && iid != -1) {
            DefInterface in = _interface[iid];
            DefClass cl = _class[cid];

            std::vector<ll> data;

            data.push_back(findClass(x.type.name));
            data.push_back(access(x));

            for (int i = 0; i < in.method.size(); i++) {
                auto& m = in.method[i];
                for (int j = 0; j < cl.method.size(); j++) {
                    auto& m2 = cl.method[j];

                    if (m.name == m2.name) {
                        data.push_back(access(makeClosure(m2.idx, x)));
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

            Pointer ret = { type, false, sAlloc(ptr) };
            proc.STACK.back().tmp.push_back(ret);
            return ret;
        }
        else if (cid != -1 && cid2 != -1) {
            if (isSuper(cid, cid2)) {
                return Pointer{ type, false, x.ptr };
            }
        }
        else if (iid2 != -1 && cid2 != -1) {
            auto iin = toIInstance(x);
            if (isSuper(iin.data[0], cid2)) {
                return Pointer{ type, false, sAlloc(iin.org) };
            }
            else return nil;
        }
    }
    return nil;
}
Pointer runMember(Pointer ptr, Word member) {
    int iid, cid;
    cid = findClass(ptr.type.name);
    iid = findInterface(ptr.type.name);

    if (cid != -1) {
        if (access(ptr) == 0) { nullPointerException(); }
        ptr = pAccess(ptr);

        auto supers = getSupers(cid);
        std::reverse(supers.begin(), supers.end());

        int sum = 0;
        for(int i = 0;i < supers.size();i++)
            sum += _class[supers[i]].field.size();

        for (int j = 0;j < supers.size();j++) {
            auto& cl = _class[supers[j]];
            sum -= cl.field.size();
            for (int i = 0; i < cl.field.size(); i++) {
                if (cl.field[i].name == member.word) {
                    Pointer ret = ptr;

                    ret.type = cl.field[i].type;
                    ret.ptr = hAccess(ret.ptr);
                    ret.ptr = pAdd(ret.ptr, i + sum);
                    return ret;
                }
            }
        }
        for (int j = 0;j < supers.size();j++) {
            auto& cl = _class[supers[j]];
            for (int i = 0; i < cl.method.size(); i++) {
                if (cl.method[i].name == member.word) {
                    return makeClosure(cl.method[i].idx, pAlloc(ptr));
                }
            }
        }
        panicf("runMember: it has no member %s", member.word.c_str());
    }
    else if(iid != -1) {
        auto iins = toIInstance(ptr);
        ptr = pAccess(ptr);

        for (int i = 0; i < iins.in.method.size(); i++) {
            if (iins.in.method[i].name == member.word) {
                Pointer ret = ptr;

                ret = pAccess(ret);
                ret.ptr = hAccess(pAdd(ret.ptr, i + 2));

                return pAlloc(ret);
            }
        }

        panicf("runMember: it has no member %s", member.word.c_str());
    }

    panicf("runMember: wrong type %s", ptr.type.name.c_str());
}
Pointer makeString(LiteralString str) {
    int len = str.str.length();
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
Pointer runIdx(Pointer ptr, Idx idx, jmp_buf jmp) {
    auto id = runExpr(idx.id, jmp);
    if (!isNum(id.type)) {
        panic("runIdx: wrong index type");
    }

    ll Id = toNum(id);

    ptr = pAccess(ptr);
    ll p = hAccess(ptr.ptr);
    ll len = hAccess(pAdd(ptr.ptr, 1));
    ll p2 = pAdd(p, Id);

    if (!(0 <= Id && Id < len)) {
        panic("runIdx: index out of range");
    }


    return Pointer{ ptr.type.add[0], ptr.lv, p2 };
}
int findNF(string name) {
    for (int i = 1; i < _nf.size(); i++) {
        if (_nf[i].name == name) {
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

    //for (int i = 0; i < proc.STACK.back().vars.back().size(); i++) {
    for (auto it = proc.STACK.back().vars.back().begin();it != proc.STACK.back().vars.back().begin(); it++) {
        auto& duo = *it;
        dref(pAccess(duo.second));
    }
}
void cleanFrame() {
    for (int i = 0; i < proc.STACK.back().vars.size(); i++) {
        auto& list = proc.STACK.back().vars[i];
        //for (int j = 0; j < list.size(); j++) {
        for (auto it = list.begin();it != list.end();it++) {
            auto& duo = *it;
            dref(pAccess(duo.second));
        }
    }
}

void enterScope() {
    proc.STACK.back().vars.emplace_back();
    proc.STACK.back().defer.emplace_back();
}
void exitScope() {
    cleanScope();
    proc.STACK.back().vars.pop_back();
    proc.STACK.back().defer.pop_back();
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
        auto& list = proc.STACK.back().vars[i];
        //for (int j = 0; j < proc.STACK.back().vars[i].size(); j++) {
        for (auto it = proc.STACK.back().vars[i].begin(); it != proc.STACK.back().vars[i].end();it++) {
            auto& var = *it;

            printf("%s: ", var.first.c_str());
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
                        printf("%lld, ", hAccess(pAdd(data, k)));
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
                        printf("class %s { ", cl.name.c_str());

                        ll data = hAccess(ptr);
                        for (int k = 0; k < cl.field.size(); k++) {
                            printf("%s : %d, ", cl.field[k].name.c_str(), hAccess(pAdd(data, k)));
                        }
                        printf("}\n");
                    }
                    else {
                        int iid = findInterface(var.second.type.name);
                        DefInterface in = _interface[iid];
                        printf("INTERFACE %s at %d", in.name.c_str(), -access(var.second));
                    }
                }
                else if (var.second.type.kind == FUNC) {
                    printf("FUNC at %d\n", -access(var.second));
                }
            }
        }
    }
}
std::vector<int> getSupers(int cid) {
    std::vector<int> ret;
    ret.push_back(cid);
    while(_class[cid].super.length() != 0) {
        cid = findClass(_class[cid].super);
        ret.push_back(cid);
    }

    std::reverse(ret.begin(), ret.end());

    return ret;
}
bool isSuper(int cid, int scid) {
    auto supers = getSupers(cid);
    for (int i = 0; i < supers.size(); i++) {
        if (supers[i] == scid)
            return true;
    }
    return false;
}
void viewErr() {
    ll err = proc.ERR;

    if (err == 0) {
        return;
    }

    ll ptr = hAccess(err);
    ll closure = hAccess(pAdd(ptr, 2));

    jmp_buf jmp;
    Pointer efunc = Pointer{ Type{FUNC, "", {tStr}}, false, sAlloc(closure) };
    Pointer str = runFCall(efunc, FCall{}, jmp);
    
    puts("unhandled error");
    nfPrint(std::vector<Pointer>{ str }, jmp);
    puts("");
}
void raiseErrNullPointer(jmp_buf jmp) {
    Pointer ptr = findGVar("makeErrNullPointer");
    Pointer err = runFCall(ptr, FCall{}, jmp);

    nfRaise(std::vector<Pointer>{ err }, jmp);
}
void raiseErrDivideByZero(jmp_buf jmp) {
    Pointer ptr = findGVar("makeErrDivideByZero");
    Pointer err = runFCall(ptr, FCall{}, jmp);

    nfRaise(std::vector<Pointer>{ err }, jmp);
}
#endif