#include <stdio.h>
#include <vector>

#include "token.hpp"
#include "keywordDict.hpp"
#include "parse.hpp"
#include "addScolon.hpp"
#include "RDP.hpp"
#include "run.hpp"

#define MAX_SOURCE_LENGTH 1000000

char buffer[MAX_SOURCE_LENGTH];

void init() {
    initDict();
    initTaboo();
}
char* readWholeFile(const char* path) {
    FILE *fp = fopen(path, "r");
    int size = fread(buffer, sizeof(char), MAX_SOURCE_LENGTH, fp);
    fclose(fp);
    buffer[size] = '\0';

    return buffer;
}

void printTokens(std::vector<Token> tokens) {
    for(int i = 0; i < tokens.size();i++) {
        printf("%s %s %llu\n", kDict.sprint(tokens[i].kind), tokens[i].str, tokens[i].value);
    }
}

int main(int argc, char* argv[]) {
    init();

    if (argc == 2) {
        char* res = readWholeFile(argv[1]);
        std::vector<Token> tokens = parse(res);
        Program prog = execRDP(tokens);

        run(prog);
    }

    return 0;
}