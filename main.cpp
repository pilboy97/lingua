#include <stdio.h>
#include <vector>

#include "token.hpp"
#include "keywordDict.hpp"
#include "parse.hpp"
#include "addScolon.hpp"
#include "RDP.hpp"
#include "run.hpp"
#include "panic.hpp"

#define MAX_SOURCE_LENGTH 1000000

char buffer[MAX_SOURCE_LENGTH + 1];

void init() {
    initDict();
    initTaboo();
}
char* readWholeFile(const char* path) {
    FILE *fp = fopen(path, "r");
    if(fp == NULL) {
        panicf("cannot open file %s", path);
    }
    int size = fread(buffer, sizeof(char), MAX_SOURCE_LENGTH, fp);
    fclose(fp);
    buffer[size] = '\0';

    char* ptr = (char*)malloc(sizeof(char) * (size + 1));
    memcpy(ptr, buffer, sizeof(char) * (size + 1));

    return ptr;
}

void printTokens(std::vector<Token> tokens) {
    for(int i = 0; i < tokens.size();i++) {
        printf("%s %s %llu\n", kDict.sprint(tokens[i].kind), tokens[i].str, tokens[i].value);
    }
}

int main(int argc, char* argv[]) {
    init();

    if (argc != 2) {
        panic("wrong argument");
    }
    char* base = readWholeFile("base.lf");
    char* res = readWholeFile(argv[1]);

    Program prog = execRDP(parse(res));
    Program pbase = execRDP(parse(base));

    for (int i = 0; i < prog.childs.size(); i++) {
        pbase.childs.push_back(prog.childs[i]);
    }
    for (int i = 0; i < prog.fn.size(); i++) {
        pbase.fn.push_back(prog.fn[i]);
    }

    run(pbase);

    return 0;
}