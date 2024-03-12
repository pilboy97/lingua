#ifndef __ADDSCOLON_HPP__
#define __ADDSCOLON_HPP__

#include <vector>

#include "code.hpp"

std::vector<int> taboo;

void initTaboo() {
    taboo.push_back(ADD);
    taboo.push_back(SUB);
    taboo.push_back(MUL);
    taboo.push_back(DIV);
    taboo.push_back(MOD);
    taboo.push_back(ASS);
    taboo.push_back(GR);
    taboo.push_back(LE);
    taboo.push_back(GEQ);
    taboo.push_back(LEQ);
    taboo.push_back(EQ);
    taboo.push_back(NEQ);
    taboo.push_back(NOT);
    taboo.push_back(AND);
    taboo.push_back(OR);
    taboo.push_back(XOR);
    taboo.push_back(RSH);
    taboo.push_back(LSH);
    taboo.push_back(LNOT);
    taboo.push_back(LAND);
    taboo.push_back(LOR);
    taboo.push_back(IS);
    taboo.push_back(DOT);
    taboo.push_back(OSB);
    taboo.push_back(OBR);
    taboo.push_back(OBL);
    taboo.push_back(ARROW);
    taboo.push_back(SEP);
    taboo.push_back(SCOLON);
}

bool isTaboo(int token) {
    for(int i = 0;i < taboo.size();i++) {
        if(taboo[i] == token) return true;
    }

    return false;
}



#endif