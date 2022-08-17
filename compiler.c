#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){    
    // intel記法はmovで右から左に代入
    // レジスタに%はつけない

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("\tmov rax, %d\n", atoi(argv[1])); // raxに返り値を入れる
    printf("\tret\n");
    return 0;
}