#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

// トークンの定義
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
    ND_NUM
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    int val;
    Node* lhs; // 木構造を作る
    Node* rhs;
};

// 変数・関数宣言
extern Token* token;
extern char* user_input;

Token* tokenize(char* p);
Node* parse_expr();
void gen(Node* node);

void print_list(Token* token);
void print_tree(Node* node, int depth);