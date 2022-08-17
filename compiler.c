#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){    
    if (argc != 2){
        fprintf(stderr, "usage: ./compiler code\n");
        return 1;
    }

    char* cur = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    int num = strtol(cur, &cur, 10);
    // fprintf(stderr, "cur: %s\n", cur);
    printf("\tmov rax, %d\n", num); // raxに返り値を入れる
    
    while(*cur){
        if (*cur == '+'){
            cur++;
            num = strtol(cur, &cur, 10);
            // fprintf(stderr, "cur: %s\n", cur);
            printf("\tadd rax, %d\n", num);
            continue;
        } else if (*cur == '-'){
            cur++;
            num = strtol(cur, &cur, 10);
            // fprintf(stderr, "cur: %s\n", cur);
            printf("\tsub rax, %d\n", num);
            continue;
        }

        fprintf(stderr, "invalid input\n");
        return 1;
    }

    printf("\tret\n");
    return 0;
}