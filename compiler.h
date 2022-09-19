#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

// トークンの定義
typedef enum {
    TK_RESERVED, // 予約された記号
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_NUM,
    TK_IDENT,
    TK_EOF
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    int val;
    char* str;
    int len;
    Token* next; // 連結リストを作る
};

// ノードの定義
typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NEQ,
    ND_LT,
    ND_LEQ,
    ND_ASSIGN,
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_LVAR,
    ND_NUM
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    int val;
    int offset; // 変数のベースポインタからのオフセット
    Node* lhs; // 木構造を作る
    Node* rhs;
    Node* next; // 連結リスト用
};

// ローカル変数の型
typedef struct LVar LVar;

struct LVar {
    char* name;
    int len;
    int offset;
    LVar* next; // 連結リストを作る
};

// 変数・関数宣言
extern Token* token;
extern char* user_input;

extern Node* code[100];

extern LVar* locals;

Token* tokenize(char* p);
void parse_program();
void gen(Node* node);

void print_list(Token* token);
void print_tree(Node* node, int depth);

void error(char* fmt, ...);