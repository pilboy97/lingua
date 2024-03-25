#ifndef __RDP_H__
#define __RDP_H__

#include <vector>
#include <algorithm>

#include "procStruct.hpp"
#include "code.hpp"
#include "token.hpp"
#include "keywordDict.hpp"
#include "panic.hpp"

static int header;
static std::vector<Token> code;

static std::vector<DefFunc> fn;

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
    else if(chk(BYTE)) {
        ret.kind = BYTE;
    }
    else if(chk(REAL)) {
        ret.kind = REAL;
    }
    else if(chk(BOOL)) {
        ret.kind = BOOL;
    }
    else if(chk(FUNC)) {
        ret.kind = FUNC;
        ret.add.push_back(Type{0});

        must(OBR);
        while (header < code.size() && code[header].kind != CBR) {
            ret.add.push_back(parseType());
            chk(COMMA);
        }
        must(CBR);
        if(chk(ARROW)) {
            ret.add.front() = parseType();
        }
    }
    else if(chk(OSB)){
        ret.kind = OSB;
        must(CSB);

        ret.add.push_back(parseType());
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
        while(header < code.size() && code[header].kind != CSB) {
            ret.captured.push_back(must(WORD).str);
            chk(COMMA);
        }
        must(CSB);
    }

    must(OBR);
    while(header < code.size() && code[header].kind != CBR) {
        const char* name = must(WORD).str;
        Type type = parseType();
        ret.frame.push_back(std::make_pair(name, type));
        chk(COMMA);
    }
    must(CBR);

    if(chk(ARROW)) {
        ret.ret = parseType();
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

        return Factor {OBR, neo};
    }
    else if(code[header].kind == LNUM) {
        Number *neo = new Number();
        neo->value = code[header++].value;

        return Factor {LNUM, neo};
    }
    else if(code[header].kind == LBYTE) {
        ByteNumber *neo = new ByteNumber();
        neo->value = code[header++].value;

        return Factor {LBYTE, neo};
    }
    else if(code[header].kind == LREAL) {
        Real *neo = new Real();
        neo->value = code[header++].real;

        return Factor {LREAL, neo};
    }
    else if(code[header].kind == LSTR) {
        LiteralString *neo = new LiteralString();
        neo->str = code[header++].str;

        return Factor {LSTR, neo};
    }
    else if(code[header].kind == WORD) {
        Word *neo = new Word();
        neo->word = code[header++].str;

        if(chk(OBL)) {
            header--;
            LiteralObject* p = new LiteralObject();
            p->type = Type{ WORD, neo->word, {} };
            p->init = parseInitList();

            return Factor{OBL, p};
        }
        else {
            return Factor {WORD, neo};
        }

    }
    else if(code[header].kind == OSB) {
        // array literal

        forward();
        must(CSB);

        LiteralArray *neo = new LiteralArray();
        neo->etype = parseType();

        must(OBL);
        while(header < code.size() && code[header].kind != CBL) {
            neo->elem.push_back(parseExpr());
            chk(COMMA);
        }
        must(CBL);

        return Factor{OSB, neo};
    }
    else if(chk(LTRUE)) {
        return Factor {LTRUE, NULL};
    }
    else if(chk(LFALSE)) {
        return Factor {LFALSE, NULL};
    }
    else if (chk(THIS)) {
        return Factor{ THIS, NULL };
    }
    else if (chk(SUPER)) {
        return Factor{SUPER, NULL};
    }
    else if(chk(FUNC)) {
        header--;

        DefFunc neo = parseLambda();
        fn.push_back(neo);

        int* ptr = new int;
        *ptr = fn.size() - 1;

        return Factor {FUNC, ptr};
    }
    else if(chk(NIL)) {
        return Factor{NIL, NULL};
    }
    else {
        panicUnexpectedToken();
    }

    return Factor{};
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
        else if(chk(DOT)) {
            stop = false;

            Word *neo = new Word();
            neo->word = must(WORD).str;
            ret.childs.push_back(std::make_pair(DOT, neo));
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
                ret.oper.push_back(std::make_pair(NIL, Type{}));
            } 
            else if(chk(LNOT)) {
                if(chk(NIL))
                    ret.oper.push_back(std::make_pair(VOID, Type{}));
                else
                    ret.oper.push_back(std::make_pair(LNOT, parseType()));
            }
            else {
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
        if(chk(LAND)) {
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
        if(chk(LOR)) {
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

    while(header < code.size() && code[header].kind != CBL) {
        ret.childs.push_back(parseStmt());
        must(SCOLON);
    }

    must(CBL);

    return ret;
}
ForStmt parseForStmt() {
    ForStmt ret;
    ret.init = NULL;
    ret.cond = NULL;
    ret.act = NULL;

    must(FOR);
    if (header < code.size() && code[header].kind != OBL) {
        must(OBR);
        Expr_1 *e = new Expr_1();
        *e = parseExpr_1();

        if(chk(SCOLON)) {
            Expr_1 *cond = new Expr_1(), * act = new Expr_1();

            *cond = parseExpr_1();
            must(SCOLON);
            *act = parseExpr_1();

            ret.init = e;
            ret.cond = cond;
            ret.act = act;
        } else {
            ret.init = NULL;
            ret.cond = e;
            ret.act = NULL;
        }
        must(CBR);
    }

    ret.body = parseBlockStmt();

    return ret;
}
IfStmt parseIfStmt() {
    IfStmt ret;
    ret._else = NULL;

    must(IF);

    must(OBR);
    Expr cond = parseExpr();
    must(CBR);

    BlockStmt block = parseBlockStmt();

    ret._if.push_back(std::make_pair(cond, block));

    while(true) {
        if(code[header].kind != ELSE) break;

        forward();
        if(chk(IF)) {
            Expr cond = parseExpr();
            BlockStmt block = parseBlockStmt();
            ret._if.push_back(std::make_pair(cond, block));
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

    if(header < code.size() && code[header].kind != ASS) {
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
    else if(code[header].kind == OBL) {
        BlockStmt *neo = new BlockStmt();
        *neo = parseBlockStmt();

        ret.kind = OBL;
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
        if (header < code.size() && code[header].kind != SCOLON) {
            Expr *e = new Expr();
            *e = parseExpr();
            neo->expr = e;
        }

        ret.kind = RETURN;
        ret.stmt = neo;
    }
    else if(chk(DEFER)) {
        DeferStmt *neo = new DeferStmt();
        neo->block = parseBlockStmt();

        ret.kind = DEFER;
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

    if(chk(COLON)) {
        ret.super = must(WORD).str;
    }
    else {
        ret.super = NULL;
    }

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
            std::vector<std::pair<const char*, Type>> frame;
            Type rtype;
            BlockStmt body;
            DefFunc func;

            method.isPrivate = false;
            if(chk(ADD))
                method.isPrivate = false;
            else if(chk(SUB))
                method.isPrivate = true;

            method.name = must(WORD).str;
            
            must(OBR);
            while(header < code.size() && code[header].kind != CBR) {
                const char* name = must(WORD).str;
                Type type = parseType();
                frame.push_back(std::make_pair(name, type));
                chk(COMMA);
            }
            must(CBR);

            rtype = Type{ 0 };
            if (chk(ARROW)) {
                rtype = parseType();
            }
            
            body = parseBlockStmt();
            must(SCOLON);

            func.ret = rtype;
            func.frame = frame;
            func.body = body;

            fn.push_back(func);
            method.idx = fn.size()-1;

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

    while(header < code.size() && code[header].kind != CBL) {
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
        
        member.ret = Type{ 0 };
        if (chk(ARROW)) {
            member.ret = parseType();
        }
        
        must(SCOLON);

        ret.method.push_back(member);
    }

    must(CBL);

    return ret;
}
DefFunc parseDefFunc() {
    DefFunc ret;
    ret.ret = Type{0};

    must(FUNC);
    ret.name = must(WORD).str;
    must(OBR);
    while(header < code.size() && code[header].kind != CBR) {
        const char* name = must(WORD).str;
        Type type = parseType();

        ret.frame.push_back(std::make_pair(name, type));
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

    while(header < code.size()) {
        if(code[header].kind == SCOLON) {
            header++;
        }
        else if(code[header].kind == FUNC) {
            DefFunc neo = parseDefFunc();
            int* ptr = new int;
            
            fn.push_back(neo);
            *ptr = fn.size() - 1;

            ret.childs.push_back(std::make_pair(FUNC, ptr));
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
    
    ret.fn = fn;
    return ret;
}

Program execRDP(std::vector<Token> code) {
    header = 0;
    ::code = code;

    return parseProgram();
}
#endif