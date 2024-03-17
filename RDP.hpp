#ifndef __RDP_H__
#define __RDP_H__

#include <vector>
#include <algorithm>
#include "token.hpp"
#include "keywordDict.hpp"
#include "panic.hpp"

static int header;
static std::vector<Token> code;

void panicUnexpectedToken() {
    panicf("at line %d, %d:unexpected %s", code[header].line, code[header].col, kDict.sprint(code[header].kind));
}
void forward() {
    header++;
}
bool chk(int kind) {
    if(header >= code.size() || header < 0) panicf("wrong header %d", header);

    if(code[header].kind == kind) {
        forward();
        return true;
    }
    else {
        return false;
    }
    return false;
}
Token must(int kind) {
    if(header >= code.size()) 
        panic("unexpected EOF");
    if(code[header].kind != kind)
        panicf("at line %d, %d: unexpected %s: %s needed", code[header].line, code[header].col, kDict.sprint(code[header].kind), kDict.sprint(kind));

    return code[header++];
}

struct Type {
    int kind;
    const char *name;
    std::vector<Type> frame;
};

struct Factor {
    int kind;
    void *ptr;
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
    Expr *src;
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
struct UNumber { 
    unsigned long long value;
};
struct ByteNumber {
    unsigned char value;
};
struct Word {
    const char *word;
};
struct LiteralString {
    const char *str;
};
struct LiteralArray {
    Type type;
    std::vector<Expr> elem;
};
struct LiteralObject {
    Type type;
    std::vector<std::pair<const char*, Expr> > value;
};
struct Stmt {
    int kind;
    void *stmt;

    void del();
};
struct BlockStmt {
    std::vector<Stmt> childs;
};
struct IfStmt {
    std::vector<std::pair<Expr, BlockStmt> > _if;
    BlockStmt *_else;
};
struct ForStmt {
    Expr_1 *init;
    Expr_1 cond;
    Expr_1 *act;

    BlockStmt body;
};
struct RetStmt {
    Expr *expr;
};
struct DefVar {
    const char *name;
    Type *type;
    Expr *init;
};
struct Method {
    bool isPrivate;
    const char *name;
    std::vector<std::pair<const char *, Type> > frame;
    BlockStmt body;
    Type ret;
};
struct Field {
    bool isPrivate;
    const char *name;
    Type type;
};
struct IMember {
    bool isPrivate;
    const char *name;
    std::vector<Type> frame;
    Type ret;
};
struct DefInterface {
    const char *name;
    std::vector<IMember> method;
};
struct DefClass {
    const char *name;
    std::vector<Field> field;
    std::vector<Method> method;
};
struct DefFunc {
    const char *name;
    std::vector<std::pair<const char *, Type> > frame;
    std::vector<const char * > captured;
    Type ret;
    BlockStmt body;
};
struct Program {
    std::vector<std::pair<int, void*> > childs;
};

FCall parseFCall();
Idx parseIdx();
InitList parseInitList();
Expr parseExpr();

Type parseType() {
    Type ret;
    ret.kind = 0;

    if(chk(NUM)) {
        ret.kind = NUM;
    }
    else if(chk(UNUM)) {
        ret.kind = UNUM;
    }
    else if(chk(BYTE)) {
        ret.kind = BYTE;
    }
    else if(chk(BOOL)) {
        ret.kind = BOOL;
    }
    else if(chk(FUNC)) {
        ret.kind = FUNC;
        ret.frame.push_back((Type){.kind=0});

        must(OBR);
        while(code[header].kind != CBR) {
            ret.frame.push_back(parseType());
            chk(COMMA);
        }
        must(CBR);
        if(chk(ARROW)) {
            ret.frame.front() = parseType();
        }
    }
    else if(chk(OSB)){
        ret.kind = OSB;
        must(CSB);

        ret.frame.push_back(parseType());
    }
    else {
        ret.kind = WORD;
        ret.name = must(WORD).str;
    }

    return ret;
}
BlockStmt parseBlockStmt();
DefFunc parseLambda() {
    DefFunc ret;

    must(FUNC);
    if(chk(OSB)) {
        while(code[header].kind != CSB) {
            ret.captured.push_back(must(WORD).str);
            chk(COMMA);
        }
        must(CSB);
    }

    ret.frame.push_back(std::make_pair("", (Type){.kind=0}));

    must(OBR);
    while(code[header].kind != CBR) {
        ret.frame.push_back(std::make_pair(must(WORD).str, parseType()));
        chk(COMMA);
    }
    must(CBR);

    if(chk(ARROW)) {
        ret.frame[0].second = parseType();
    }

    ret.body = parseBlockStmt();

    return ret;
}
Factor parseFactor() {
    if(code[header].kind == OBR) {
        forward();
        Expr *neo = new Expr();
        *neo = parseExpr();

        must(CBR);

        return (Factor) {.kind=OBR, .ptr=neo};
    }
    else if(code[header].kind == LNUM) {
        Number *neo = new Number();
        neo->value = code[header++].value;

        return (Factor) {.kind=LNUM, .ptr=neo};
    }
    else if(code[header].kind == LUNUM) {
        UNumber *neo = new UNumber();
        neo->value = code[header++].value;

        return (Factor) {.kind=LUNUM, .ptr=neo};
    }
    else if(code[header].kind == LBYTE) {
        ByteNumber *neo = new ByteNumber();
        neo->value = code[header++].value;

        return (Factor) {.kind=LBYTE, .ptr=neo};
    }
    else if(code[header].kind == LSTR) {
        LiteralString *neo = new LiteralString();
        neo->str = code[header].str;

        return (Factor) {.kind=LSTR, .ptr=neo};
    }
    else if(code[header].kind == WORD) {
        Word *neo = new Word();
        neo->word = code[header++].str;

        return (Factor) {.kind=WORD, .ptr=neo};
    }
    else if(code[header].kind == OSB) {
        // array literal
        forward();
        must(CSB);

        LiteralArray *neo = new LiteralArray();
        neo->type = parseType();

        must(OBL);
        while(code[header].kind != CBL) {
            neo->elem.push_back(parseExpr());
            chk(COMMA);
        }
        must(CBL);

        return (Factor) {.kind=OSB, .ptr = neo};
    }
    else if(chk(LTRUE)) {
        return (Factor) {.kind=LTRUE, .ptr=NULL};
    }
    else if(chk(LFALSE)) {
        return (Factor) {.kind=LFALSE, .ptr=NULL};
    }
    else if(chk(FUNC)) {
        header--;

        DefFunc *neo = new DefFunc();
        *neo =  parseLambda();

        return (Factor) {.kind=FUNC, .ptr=neo};
    }
    else if(chk(NIL)) {
        return (Factor){.kind=NIL};
    }
    else {
        panicUnexpectedToken();
    }

    return (Factor){};
}
Expr12 parseExpr12() {
    Expr12 ret;

    ret.f = parseFactor();
    
    bool stop;
    do {
        stop = true;

        if(code[header].kind == OBR) {
            stop = false;
            
            FCall *fc = new FCall();
            *fc = parseFCall();
            ret.childs.push_back(std::make_pair(OBR, fc));
        }
        else if(code[header].kind == OSB) {
            stop = false;

            Idx *idx = new Idx();
            *idx = parseIdx();
            ret.childs.push_back(std::make_pair(OSB, idx));
        }
        else if(code[header].kind == OBL) {
            stop = false;

            InitList *list = new InitList();
            *list = parseInitList();
            ret.childs.push_back(std::make_pair(OBL, list));
        }
        else if(chk(DOT)) {
            stop = false;

            Word *neo = new Word();
            neo->word = must(WORD).str;
            ret.childs.push_back(std::make_pair(WORD, neo));
        }
    } while(!stop);

    return ret;
}
Expr11 parseExpr11() {
    Expr11 ret;

    while (true) {
        if(chk(NOT)) {
            ret.oper.push_back(NOT);
        }
        else if(chk(LNOT)) {
            ret.oper.push_back(LNOT);
        }
        else if(chk(SUB)) {
            ret.oper.push_back(SUB);
        }
        else break;
    }

    ret.child = parseExpr12();

    return ret;
}
Expr10 parseExpr10() {
    Expr10 ret;
    ret.child = parseExpr11();

    while(true) {
        if(chk(IS)) {
            if(chk(NIL)) {
                ret.oper.push_back(std::make_pair(NIL, (Type){}));
            } else if(chk(LNOT)) {
                must(NIL);
                ret.oper.push_back(std::make_pair(LNOT, (Type){}));
            } else {
                ret.oper.push_back(std::make_pair(IS, parseType()));
            }
        }
        else if(chk(AS)) {
            ret.oper.push_back(std::make_pair(AS, parseType()));
        }
        else break;
    }

    return ret;
}
Expr9 parseExpr9() {
    Expr9 ret;
    ret.childs.push_back(std::make_pair(SCOLON, parseExpr10()));

    while(true) {
        if(chk(MUL)) {
            ret.childs.push_back(std::make_pair(MUL, parseExpr10()));
        }
        else if(chk(DIV)) {
            ret.childs.push_back(std::make_pair(DIV, parseExpr10()));
        }
        else if(chk(MOD)) {
            ret.childs.push_back(std::make_pair(MOD, parseExpr10()));
        }
        else break;
    }

    return ret;
}
Expr8 parseExpr8() {
    Expr8 ret;
    ret.childs.push_back(std::make_pair(SCOLON, parseExpr9()));

    while(true) {
        if(chk(ADD)) {
            ret.childs.push_back(std::make_pair(ADD, parseExpr9()));
        }
        else if(chk(SUB)) {
            ret.childs.push_back(std::make_pair(SUB, parseExpr9()));
        }
        else break;
    }

    return ret;
}
Expr7 parseExpr7() {
    Expr7 ret;
    ret.childs.push_back(std::make_pair(SCOLON, parseExpr8()));

    while(true) {
        if(chk(LSH)) {
            ret.childs.push_back(std::make_pair(LSH, parseExpr8()));
        }
        else if(chk(RSH)) {
            ret.childs.push_back(std::make_pair(RSH, parseExpr8()));
        }
        else break;
    }

    return ret;
}
Expr6 parseExpr6() {
    Expr6 ret;
    ret.childs.push_back(std::make_pair(SCOLON, parseExpr7()));

    while(true) {
        if(chk(GR)) {
            ret.childs.push_back(std::make_pair(GR, parseExpr7()));
        }
        else if(chk(LE)) {
            ret.childs.push_back(std::make_pair(LE, parseExpr7()));
        }
        else if(chk(GEQ)) {
            ret.childs.push_back(std::make_pair(GEQ, parseExpr7()));
        }
        else if(chk(LEQ)) {
            ret.childs.push_back(std::make_pair(LEQ, parseExpr7()));
        }
        else break;
    }

    return ret;
}
Expr5 parseExpr5() {
    Expr5 ret;
    ret.childs.push_back(std::make_pair(SCOLON, parseExpr6()));

    while(true) {
        if(chk(EQ)) {
            ret.childs.push_back(std::make_pair(EQ, parseExpr6()));
        }
        else if(chk(NEQ)) {
            ret.childs.push_back(std::make_pair(NEQ, parseExpr6()));
        }
        else break;
    }

    return ret;
}
Expr4 parseExpr4() {
    Expr4 ret;
    ret.childs.push_back(parseExpr5());

    while(true) {
        if(chk(AND)) {
            ret.childs.push_back(parseExpr5());
        }
        else break;
    }

    return ret;
}
Expr3 parseExpr3() {
    Expr3 ret;
    ret.childs.push_back(parseExpr4());

    while(true) {
        if(chk(XOR)) {
            ret.childs.push_back(parseExpr4());
        }
        else break;
    }

    return ret;
}
Expr2 parseExpr2() {
    Expr2 ret;
    ret.childs.push_back(parseExpr3());

    while(true) {
        if(chk(OR)) {
            ret.childs.push_back(parseExpr3());
        }
        else break;
    }

    return ret;
}
Expr1 parseExpr1() {
    Expr1 ret;
    ret.childs.push_back(parseExpr2());

    while(true) {
        if(chk(AND)) {
            ret.childs.push_back(parseExpr2());
        }
        else break;
    }

    return ret;
}
Expr parseExpr() {
    Expr ret;
    ret.childs.push_back(parseExpr1());

    while(true) {
        if(chk(OR)) {
            ret.childs.push_back(parseExpr1());
        }
        else break;
    }

    return ret;
}
Idx parseIdx() {
    Idx ret;
    must(OSB);

    ret.id = parseExpr();

    must(CSB);

    return ret;
}
InitList parseInitList() {
    InitList ret;

    must(OBL);

    while(header < code.size() && code[header].kind != CBL) {
        const char *name = must(WORD).str;
        must(COLON);
        Expr e = parseExpr();

        ret.list.push_back(std::make_pair(name, e));
        chk(COMMA);
    }

    must(CBL);

    return ret;
}
FCall parseFCall() {
    FCall ret;

    must(OBR);

    while(header < code.size() && code[header].kind != CBR) {
        ret.list.push_back(parseExpr());
        chk(COMMA);
    }

    must(CBR);

    return ret;
}
Expr_1 parseExpr_1() {
    Expr_1 ret;
    ret.dst = parseExpr();
    ret.src = NULL;
    if(chk(ASS)) {
        ret.src = new Expr();
        *ret.src = parseExpr();
    }

    return ret;
}
Stmt parseStmt();
BlockStmt parseBlockStmt() {
    BlockStmt ret;

    must(OBL);

    while(code[header].kind != CBL) {
        ret.childs.push_back(parseStmt());
        must(SCOLON);
    }

    must(CBL);

    return ret;
}
ForStmt parseForStmt() {
    ForStmt ret;

    must(FOR);
    if (code[header].kind != OBL) {
        Expr_1 *e = new Expr_1();
        *e = parseExpr_1();

        if(chk(SCOLON)) {
            Expr_1 cond, *act = new Expr_1();

            cond = parseExpr_1();
            must(SCOLON);
            *act = parseExpr_1();

            ret.init = e;
            ret.cond = cond;
            ret.act = act;
        } else {
            ret.init = NULL;
            ret.cond = *e;
            ret.act = NULL;
        }
    }

    ret.body = parseBlockStmt();

    return ret;
}
IfStmt parseIfStmt() {
    IfStmt ret;
    ret._else = NULL;

    must(IF);
    ret._if.push_back(std::make_pair(parseExpr(), parseBlockStmt()));

    while(true) {
        if(code[header].kind != ELSE) break;

        forward();
        if(chk(IF)) {
            ret._if.push_back(std::make_pair(parseExpr(), parseBlockStmt()));
        }
        else {
            ret._else = new BlockStmt();
            *ret._else = parseBlockStmt();
        }
    }

    return ret;
}
DefVar parseDefVar() {
    DefVar ret;
    ret.init = NULL;
    ret.type = NULL;

    must(VAR);
    ret.name = must(WORD).str;

    if(code[header].kind != ASS) {
        ret.type = new Type();
        *ret.type = parseType();
    }
    
    if(chk(ASS)) {
        ret.init = new Expr();
        *ret.init = parseExpr();
    }

    return ret;
}
Stmt parseStmt() {
    Stmt ret;

    if(code[header].kind == VAR) {
        DefVar *neo = new DefVar();
        *neo = parseDefVar();

        ret.kind = VAR;
        ret.stmt = neo;
    }
    else if(code[header].kind == IF) {
        IfStmt *neo = new IfStmt();
        *neo = parseIfStmt();

        ret.kind = IF;
        ret.stmt = neo;
    }
    else if(code[header].kind == FOR) {
        ForStmt *neo = new ForStmt();
        *neo = parseForStmt();

        ret.kind = FOR;
        ret.stmt = neo;
    }
    else if(chk(BREAK)) {
        ret.kind = BREAK;        
    }
    else if(chk(CONTINUE)) {
        ret.kind = CONTINUE;
    }
    else if(chk(RETURN)) {
        RetStmt *neo = new RetStmt();
        neo->expr = NULL;
        if (code[header].kind != SCOLON) {
            Expr *e = new Expr();
            *e = parseExpr();
            neo->expr = e;
        }

        ret.kind = RETURN;
        ret.stmt = neo;
    }
    else {
        Expr_1 *neo = new Expr_1();
        *neo = parseExpr_1();

        ret.kind = SCOLON;
        ret.stmt = neo;
    }
    return ret;
}
DefClass parseDefClass() {
    DefClass ret;

    must(CLASS);
    ret.name = must(WORD).str;
    must(OBL);
    
    while(header < code.size() && code[header].kind != CBL && code[header].kind != SEP) {
        Field field;

        field.isPrivate = false;
        if(chk(ADD)) {
            field.isPrivate = false;
        }
        else if(chk(SUB)) {
            field.isPrivate = true;
        }
        
        field.name = must(WORD).str;
        field.type = parseType();

        must(SCOLON);

        ret.field.push_back(field);
    }

    if(chk(SEP)) {
        while(header < code.size() && code[header].kind != CBL) {
            Method method;

            method.isPrivate = false;
            if(chk(ADD))
                method.isPrivate = false;
            else if(chk(SUB))
                method.isPrivate = true;

            method.name = must(WORD).str;
            printf("name: %s", method.name);
        
            must(OBR);
            while(header < code.size() && code[header].kind != CBR) {
                method.frame.push_back(std::make_pair(must(WORD).str, parseType()));
                chk(COMMA);
            }
            must(CBR);
            
            method.body = parseBlockStmt();
            must(SCOLON);

            ret.method.push_back(method);
        }
    }

    must(CBL);

    return ret;
}
DefInterface parseDefInterface() {
    DefInterface ret;

    must(INTERFACE);
    ret.name = must(WORD).str;

    must(OBL);

    while(code[header].kind != CBL) {
        IMember member;

        member.isPrivate = false;
        if(chk(ADD)) {
            member.isPrivate = false;
        }
        else if(chk(SUB)) {
            member.isPrivate = true;
        }
        
        member.name = must(WORD).str;
        must(OBR);
        while(header < code.size() && code[header].kind != CBR) {
            member.frame.push_back(parseType());
            chk(COMMA);
        }
        must(CBR);
        must(SCOLON);
    }

    must(CBL);

    return ret;
}
DefFunc parseDefFunc() {
    DefFunc ret;
    ret.ret = (Type){.kind=0};

    must(FUNC);
    ret.name = must(WORD).str;
    must(OBR);
    while(header < code.size() && code[header].kind != CBR) {
        ret.frame.push_back(std::make_pair(must(WORD).str, parseType()));
        chk(COMMA);
    }
    must(CBR);

    if(chk(ARROW)) {
        ret.ret = parseType();
    }

    ret.body = parseBlockStmt();
    return ret;
}

Program parseProgram() {
    Program ret;

    while(header < ::code.size()) {
        if(code[header].kind == SCOLON) {
            header++;
        }
        else if(code[header].kind == FUNC) {
            DefFunc *neo = new DefFunc();
            *neo = parseDefFunc();
            ret.childs.push_back(std::make_pair(FUNC, neo));
            must(SCOLON);
        }
        else if(code[header].kind == CLASS) {
            DefClass *neo = new DefClass();
            *neo = parseDefClass();
            ret.childs.push_back(std::make_pair(CLASS, neo));
            must(SCOLON);
        }
        else if(code[header].kind == INTERFACE) {
            DefInterface *neo = new DefInterface();
            *neo = parseDefInterface();
            ret.childs.push_back(std::make_pair(INTERFACE, neo));
            must(SCOLON);
        }
        else {
            panicUnexpectedToken();
        }
    }
    
    return ret;
}

Program execRDP(std::vector<Token> code) {
    header = 0;
    ::code = code;

    return parseProgram();
}
#endif