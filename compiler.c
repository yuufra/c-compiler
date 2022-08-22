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
    parse_program();
    // fprintf(stderr, "token::");
    // print_tree(node, 0);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, %d\n", locals->offset);

    for (int i=0; code[i] != NULL; i++){
        gen(code[i]);
        printf("\tpop rax\n");
    }

    printf("\tmov rsp, rbp\n");
    printf("\tpop rbp\n");
    printf("\tret\n"); // スタックをポップして関数の呼び出し元に戻る
    return 0;
}