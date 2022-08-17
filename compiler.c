#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

typedef enum {
    TK_RESERVED, // 予約された記号
    TK_NUM,
    TK_EOF
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    int val;
    char* str;
    Token* next; // 連結リストを作る
};

Token* token;

void error(char* msg){
    fprintf(stderr, "%s", msg);
    exit(1);
}

void print_list(Token* token){
    do {
        fprintf(stderr, "kind:%d, val:%d, str:%s\n", token->kind, token->val, token->str);
        token = token->next;
    } while (token != NULL);
    fprintf(stderr,"\n\n");
}

Token* new_token(TokenKind kind, Token* cur, char* p){
    // fprintf(stderr, "type %d registered\n", kind);
    cur->next = (Token*)calloc(1, sizeof(Token));
    cur = cur->next;

    cur->kind = kind;
    cur->str = (char*)calloc(1, sizeof(char));
    cur->str = p;
    return cur;
}

Token* tokenize(char* p){
    Token head;
    Token* cur = &head;

    int num;
    
    while(*p){
        // print_list(&head);
        if (isspace(*p)){
            p++;
            continue;
        } else if (*p == '+' || *p == '-'){
            cur = new_token(TK_RESERVED, cur, p);
            // fprintf(stderr, "p: %s\n", p);
            p++;
            continue;
        } else if (isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            // fprintf(stderr, "p: %s\n", p);
            num = strtol(p, &p, 10);
            cur->val = num;
            continue;
        }

        error("invalid input\n");
    }

    cur = new_token(TK_EOF, cur, p);
    return head.next;
}

int expect_number(){
    // fprintf(stderr, "type %d found\n", token->kind);
    if (token->kind != TK_NUM){
        error("expected number, but got another kind of value\n");
    }
    int n = token->val;
    token = token->next;
    return n;
}

int at_eof(){
    return token->kind == TK_EOF;
}

int consume(char c){
    if (token->kind != TK_RESERVED || token->str[0] != c){
        return false;
    }
    token = token->next;
    return true;
}

void expect(char c){
    if (token->kind != TK_RESERVED || token->str[0] != c){
        error("got unexpexted value\n");
    }
    token = token->next;
}

int main(int argc, char** argv){    
    if (argc != 2){
        fprintf(stderr, "usage: ./compiler code\n");
        return 1;
    }

    token = tokenize(argv[1]);
    // fprintf(stderr, "token::");
    // print_list(token);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    printf("\tmov rax, %d\n", expect_number()); // raxに返り値を入れる
    
    while(!at_eof()){
        if (consume('+')){
            printf("\tadd rax, %d\n", expect_number());
            continue;
        }
        expect('-');
        printf("\tsub rax, %d\n", expect_number());
    }

    printf("\tret\n");
    return 0;
}