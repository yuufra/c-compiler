#include "compiler.h"

Token* token;
char* user_input;

int main(int argc, char** argv){    
    if (argc != 2){
        fprintf(stderr, "usage: ./compiler code\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);
    Node* node = parse_expr();
    // fprintf(stderr, "token::");
    // print_tree(node, 0);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    printf("\tpop rax\n");
    printf("\tret\n");
    return 0;
}