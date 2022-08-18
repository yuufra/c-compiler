#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>

// EBNFによる文法
// expr    = mul ("+" mul | "-" mul)*
// mul     = unary ("*" unary | "/" unary)*
// unary   = ("+" | "-")? primary
// primary = num | "(" expr ")"
// (優先順位が高い演算子ほど先に計算したいので下に来る)

// トークンによる中間表現をノード(木構造)による中間表現に変換

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
    Token* next; // 連結リストを作る
};

// ノードの定義
typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    int val;
    Node* lhs; // 木構造を作る
    Node* rhs;
};

// 関数の宣言
Node* parse_expr();
Node* parse_mul();
Node* parse_unary();
Node* parse_primary();

int expect_number();
int consume(char);
void expect(char);


Token* token;
char* user_input;

void error_at(char* loc, char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s^\n", pos, " "); // *と第3引数で最小フィールド幅の指定
    vfprintf(stderr, fmt, ap);
    exit(1);
}

// トークンと連結リストの関数
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
        } else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'){
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

        error_at(p, "invalid input\n");
    }

    cur = new_token(TK_EOF, cur, p);
    return head.next;
}

// 構文木とノードの関数
void print_tree(Node* node, int depth){
    fprintf(stderr, "- type:%d", node->kind);
    if (node->kind == ND_NUM){
        fprintf(stderr, ",val:%d\n", node->val);
    } else {
        fprintf(stderr, "\n");
        fprintf(stderr, "%*s", 2*depth, " ");
        print_tree(node->lhs, depth+1);
        fprintf(stderr, "%*s", 2*depth, " ");
        print_tree(node->rhs, depth+1);
    }
}

Node* new_node(NodeKind kind, Node* lhs, Node* rhs){
    // fprintf(stderr, "type %d registered\n", kind);
    Node* node = (Node*)calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* new_node_num(int val){
    // fprintf(stderr, "number registered(value:%d)\n", val);
    Node* node = (Node*)calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// 例えば1*2+3*4をパースするところから考える
// 下から積み上げていく
Node* parse_expr(){
    // fprintf(stderr, "parse_expr called\n");
    Node* node = parse_mul();

    for(;;){
        if(consume('+')){
            node = new_node(ND_ADD, node, parse_mul());
            // print_tree(node, 0);
            continue;
        } else if(consume('-')){
            node = new_node(ND_SUB, node, parse_mul());
            // print_tree(node, 0);
            continue;
        }
        return node;
    }
}

Node* parse_mul(){
    // fprintf(stderr, "parse_mul called\n");
    Node* node = parse_unary();

    for(;;){
        if(consume('*')){
            node = new_node(ND_MUL, node, parse_unary());
            // print_tree(node, 0);
            continue;
        } else if(consume('/')){
            node = new_node(ND_DIV, node, parse_unary());
            // print_tree(node, 0);
            continue;
        }
        return node;
    }
}

Node* parse_unary(){
    Node* node;
    if(consume('+')){
        node = parse_primary();
        // print_tree(node, 0);
    } else if(consume('-')){
        node = new_node_num(0);
        node = new_node(ND_SUB, node, parse_primary());
        // print_tree(node, 0);
    } else {
        node = parse_primary();
    }
    return node;
}

Node* parse_primary(){
    // fprintf(stderr, "parse_primary called\n");
    Node* node;
    if(consume('(')){
        node = parse_expr();
        expect(')');
    } else {
        int num = expect_number();
        node = new_node_num(num);
        // print_tree(node, 0);
    }
    return node;
}

int expect_number(){
    // fprintf(stderr, "type %d found\n", token->kind);
    if (token->kind != TK_NUM){
        error_at(token->str, "expected number, but got unexpexted value\n");
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
        error_at(token->str, "expected '%c', but got unexpexted value\n", c);
    }
    token = token->next;
}

void gen(Node* node){
    if (node->kind == ND_NUM){
        printf("\tpush %d\n", node->val);
        return;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("\tpop rdi\n"); // 2-1を考えるとこの順番になる
    printf("\tpop rax\n");
    if (node->kind == ND_ADD){
        printf("\tadd rax, rdi\n");
    } else if (node->kind == ND_SUB){
        printf("\tsub rax, rdi\n");
    } else if (node->kind == ND_MUL){
        printf("\timul rax, rdi\n");
    } else if (node->kind == ND_DIV){
        printf("\tcqo\n"); // 64bitのraxを[rdx:rax]の128bitに伸ばす
        printf("\tidiv rdi\n"); // [rdx:rax] / rdi = rax あまり rdx
    }
    printf("\tpush rax\n");
}

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