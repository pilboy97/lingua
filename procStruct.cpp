#include "procStruct.hpp"

bool isNum(Type x) {
    return x.kind == NUM;
}
bool isByte(Type x) {
    return x.kind == BYTE;
}
bool isBool(Type x) {
    return x.kind == BOOL;
}
bool isReal(Type x) {
    return x.kind == REAL;
}
bool isArray(Type x) {
    return x.kind == OSB;
}
bool isFunc(Type x) {
    return x.kind == FUNC;
}
bool isInstance(Type x) {
    return x.kind == WORD;
}
bool isIInstance(Type x) {
    return x.kind == INTERFACE;
}
bool isSameType(Type x, Type y) {
    if (x.kind != y.kind) return false;
    if (x.kind == OSB) {
        return isSameType(x.add[0], y.add[0]);
    }
    if (x.kind == FUNC) {
        if (x.add.size() != y.add.size()) return false;
        for (int i = 0; i < x.add.size(); i++) {
            if (!isSameType(x.add[i], y.add[i])) {
                return false;
            }
        }
        return true;
    }
    if (x.kind == WORD) {
        return strcmp(x.name, y.name) == 0;
    }

    return true;
}
bool isPriType(Type x) {
    return x.kind == 0 || x.kind == NUM || x.kind == BYTE || x.kind == BOOL || x.kind == REAL;
}
Type fnToType(DefFunc x) {
    Type ret = { FUNC, "", {} };

    ret.add.push_back(x.ret);
    for (int i = 0; i < x.frame.size(); i++) {
        ret.add.push_back(x.frame[i].second);
    }
    
    return ret;
}
void printExpr_1(Expr_1 e) {
    if (e.src == NULL) printExpr(e.dst);
    else {
        printExpr(e.dst);
        printf(" = ");
        printExpr(*e.src);
    }
}
void printExpr(Expr e) {
    if (e.childs.size() == 1) {
        printExpr1(e.childs[0]);
    }
    else {
        printExpr1(e.childs[0]);
        for (int i = 1; i < e.childs.size(); i++) {
            printf(" or ");
            printExpr1(e.childs[i]);
        }
    }
}
void printExpr1(Expr1 e) {
    if (e.childs.size() == 1) {
        printExpr2(e.childs[0]);
    }
    else {
        printExpr2(e.childs[0]);
        for (int i = 1; i < e.childs.size(); i++) {
            printf(" and ");
            printExpr2(e.childs[i]);
        }
    }
}
void printExpr2(Expr2 e) {
    if (e.childs.size() == 1) {
        printExpr3(e.childs[0]);
    }
    else {
        printExpr3(e.childs[0]);
        for (int i = 1; i < e.childs.size(); i++) {
            printf(" | ");
            printExpr3(e.childs[i]);
        }
    }
}
void printExpr3(Expr3 e) {
    if (e.childs.size() == 1) {
        printExpr4(e.childs[0]);
    }
    else {
        printExpr4(e.childs[0]);
        for (int i = 1; i < e.childs.size(); i++) {
            printf(" ^ ");
            printExpr4(e.childs[i]);
        }
    }
}
void printExpr4(Expr4 e) {
    if (e.childs.size() == 1) {
        printExpr5(e.childs[0]);
    }
    else {
        printExpr5(e.childs[0]);
        for (int i = 1; i < e.childs.size(); i++) {
            printf(" & ");
            printExpr5(e.childs[i]);
        }
    }
}
void printExpr5(Expr5 e) {
    if (e.childs.size() == 1) {
        printExpr6(e.childs[0].second);
    }
    else {
        printExpr6(e.childs[0].second);
        for (int i = 1; i < e.childs.size(); i++) {
            switch (e.childs[0].first) {
            case EQ:
                printf(" == ");
                break;
            case NEQ:
                printf(" != ");
                break;
            }
            printExpr6(e.childs[i].second);
        }
    }
}
void printExpr6(Expr6 e) {
    if (e.childs.size() == 1) {
        printExpr7(e.childs[0].second);
    }
    else {
        printExpr7(e.childs[0].second);
        for (int i = 1; i < e.childs.size(); i++) {
            switch (e.childs[0].first) {
            case GR:
                printf(" > ");
                break;
            case LE:
                printf(" < ");
                break;
            case GEQ:
                printf(" >= ");
                break;
            case LEQ:
                printf(" <= ");
                break;
            }
            printExpr7(e.childs[i].second);
        }
    }
}
void printExpr7(Expr7 e) {
    if (e.childs.size() == 1) {
        printExpr8(e.childs[0].second);
    }
    else {
        printExpr8(e.childs[0].second);
        for (int i = 1; i < e.childs.size(); i++) {
            switch (e.childs[0].first) {
            case LSH:
                printf(" << ");
                break;
            case RSH:
                printf(" >> ");
                break;
            }
            printExpr8(e.childs[i].second);
        }
    }
}
void printExpr8(Expr8 e) {
    if (e.childs.size() == 1) {
        printExpr9(e.childs[0].second);
    }
    else {
        printExpr9(e.childs[0].second);
        for (int i = 1; i < e.childs.size(); i++) {
            switch (e.childs[0].first) {
            case ADD:
                printf(" + ");
                break;
            case SUB:
                printf(" - ");
                break;
            }
            printExpr9(e.childs[i].second);
        }
    }
}
void printExpr9(Expr9 e) {
    if (e.childs.size() == 1) {
        printExpr10(e.childs[0].second);
    }
    else {
        printExpr10(e.childs[0].second);
        for (int i = 1; i < e.childs.size(); i++) {
            switch (e.childs[0].first) {
            case MUL:
                printf(" * ");
                break;
            case DIV:
                printf(" / ");
                break;
            case MOD:
                printf(" % ");
                break;
            }
            printExpr10(e.childs[i].second);
        }
    }
}
void printExpr10(Expr10 e) {
    printExpr11(e.child);
    for (int i = 0; i < e.oper.size(); i++) {
        switch (e.oper[0].first) {
        case NIL:
            printf(" is null");
            break;
        case VOID:
            printf(" is not null");
            break;
        case LNOT:
            printf(" is not ");
            printType(e.oper[i].second);
            break;
        case IS:
            printf(" is ");
            printType(e.oper[i].second);
            break;
        case AS:
            printf(" as ");
            printType(e.oper[i].second);
            break;
        }
    }
}
void printExpr11(Expr11 e) {
    for (int i = 0; i < e.oper.size(); i++) {
        switch (e.oper[i])
        {
        case NOT:
            printf("!");
        case LNOT:
            printf("not");
        case SUB:
            printf("~");
        }
    }

    printExpr12(e.child);
}
void printExpr12(Expr12 e) {
    printFactor(e.f);

    for (int i = 0; i < e.childs.size(); i++) {
        Idx* ptr1;
        FCall* ptr2;
        Word* ptr3;

        switch (e.childs[i].first)
        {
        case OSB:
            ptr1 = (Idx*)e.childs[i].second;
            printf("[ ");
            printExpr(ptr1->id);
            printf(" ]");
            break;
        case OBR:
            ptr2 = (FCall*)e.childs[i].second;
            printf("( ");
            if(ptr2->list.size() >= 1) {
                printExpr(ptr2->list[0]);
                for (int i = 1; i < ptr2->list.size(); i++) {
                    printf(", ");
                    printExpr(ptr2->list[i]);
                }
            }
            printf(" )");
            break;
        case DOT:
            ptr3 = (Word*)e.childs[i].second;
            printf(".%s", ptr3->word);
            break;
        }
    }
}
void printFactor(Factor f) {
    Expr* ptr1;
    Number* ptr2;
    ByteNumber* ptr3;
    Real* ptr4;
    LiteralString* ptr5;
    Word* ptr6;
    LiteralObject* ptr7;
    LiteralArray* ptr8;

    switch (f.kind) {
    case OBR:
        ptr1 = (Expr*)f.ptr;
        printf("( ");
        printExpr(*ptr1);
        printf(" )");
        break;
    case LNUM:
        ptr2 = (Number*)f.ptr;
        printf("%lld", ptr2->value);
        break;
    case LBYTE:
        ptr3 = (ByteNumber*)f.ptr;
        printf("%d", ptr3->value);
        break;
    case LREAL:
        ptr4 = (Real*)f.ptr;
        printf("%lf", ptr4->value);
        break;
    case LSTR:
        ptr5 = (LiteralString*)f.ptr;
        printf("%s", ptr5->str);
        break;
    case WORD:
        ptr6 = (Word*)f.ptr;
        printf("%s", ptr6->word);
        break;
    case OBL:
        ptr7 = (LiteralObject*)f.ptr;
        printType(ptr7->type);
        printf("{...}");
        break;
    case OSB:
        ptr8 = (LiteralArray*)f.ptr;
        printf("[]");
        printType(ptr8->etype);
        printf("{...}");
        break;
    case LTRUE:
        printf("true");
        break;
    case LFALSE:
        printf("false");
        break;
    case THIS:
        printf("this");
        break;
    case FUNC:
        printf("func(...){...}");
        break;
    case NIL:
        printf("nil");
        break;
    default:
        break;
    }
}
void printType(Type x) {
    int i;
    switch (x.kind)
    {
    case NUM:
        printf("num");
        break;
    case BYTE:
        printf("byte");
        break;
    case BOOL:
        printf("bool");
        break;
    case REAL:
        printf("real");
        break;
    case OSB:
        printf("[]");
        printType(x.add[0]);
        break;
    case WORD:
        printf("%s", x.name);
        break;
    case FUNC:
        printf("func (");
        if (x.add.size() > 1) {
            printType(x.add[1]);
            for (i = 2; i < x.add.size(); i++) {
                printf(", ");
                printType(x.add[i]);
            }
        }
        printf(")");
        if (x.add[0].kind != 0) {
            printf(" -> ");
            printType(x.add[0]);
        }
    default:
        break;
    }
}
void printStmt(Stmt s) {
    switch (s.kind)
    {
    case FOR:
        printForStmt(*(ForStmt*)s.stmt);
        break;
    case IF:
        printIfStmt(*(IfStmt*)s.stmt);
        break;
    case RETURN:
        printRetStmt(*(RetStmt*)s.stmt);
        break;
    case VAR:
        printDefVar(*(DefVar*)s.stmt);
        break;
    case OBL:
        puts("in block: ");
        break;
    case SCOLON:
        printf("Expression: ");
        printExpr_1(*(Expr_1*)s.stmt);
        puts("");
        break;
    case BREAK:
        printf("break\n");
        break;
    case CONTINUE:
        printf("continue\n");
        break;
    default:
        break;
    }
}
void printForStmt(ForStmt s) {
    if (s.init != NULL && s.cond != NULL && s.act != NULL) {
        printf("for ( ");
        printExpr_1(*s.init);
        printf("; ");
        printExpr_1(*s.cond);
        printf("; ");
        printExpr_1(*s.act);
        printf(" )\n");
    }
    else if (s.cond != NULL) {
        printf("for ( ");
        printExpr_1(*s.cond);
        printf(" )\n");
    }
    else {
        printf("for \n");
    }
}
void printIfStmt(IfStmt s) {
    for (int i = 0; i < s._if.size(); i++) {
        printf("if ( ");
        printExpr(s._if[i].first);
        printf(" )\n");
    }
    if (s._else != NULL) {
        printf("else\n");
    }
}
void printRetStmt(RetStmt s) {
    printf("return ");
    if(s.expr != NULL)
        printExpr(*s.expr);
    puts("");
}
void printDefVar(DefVar s) {
    if (s.init != NULL && s.type != NULL) {
        printf("var %s ", s.name);
        printType(*s.type);
        printf(" = ");
        printExpr(*s.init);
    }
    else if (s.init != NULL) {
        printf("var %s ", s.name);
        printf(" = ");
        printExpr(*s.init);
    }
    else {
        printf("var %s ", s.name);
        printType(*s.type);
    }
    puts("");
}
void printBlockStmt(BlockStmt s) {
    printf("in block statement");
}