#include "compiler.h"

// EBNFによる文法
// program    = stmt*
// stmt       = expr ";"
// expr       = assign
// assign     = equality ( "=" assign )?
// equality   = relational ("==" relational || "!=" relational)*
// relational = add ("<" add || "<=" add || ">" add || ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num | ident | "(" expr ")"
// (優先順位が高い演算子ほど先に計算したいので下に来る)
// ("="は右結合であることに注意)

// トークンによる中間表現をノード(木構造)による中間表現に変換

// 関数の宣言
void parse_program();
Node* parse_stmt();
Node* parse_expr();
Node* parse_assign();
Node* parse_equality();
Node* parse_relational();
Node* parse_add();
Node* parse_mul();
Node* parse_unary();
Node* parse_primary();

Token* consume_ident();
int expect_number();
int at_eof();
int consume(char*);
void expect(char*);

// エラー関数
void error_at(char* loc, char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s^\n", pos, " "); // *と第3引数で最小フィールド幅の指定
    vfprintf(stderr, fmt, ap);
    exit(1);
}

void error(char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    fprintf(stderr, "%s\n", user_input);
    vfprintf(stderr, fmt, ap);
    exit(1);
}


// トークンと連結リストの関数
void print_list(Token* token){
    while (token != NULL) {
        fprintf(stderr, "- type:%d", token->kind);
        if (token->kind == TK_NUM){
            fprintf(stderr, ", val:%d\n", token->val);
        } else if (token->kind == TK_RESERVED || token->kind == TK_IDENT){
            fprintf(stderr, ", str:%s\n", token->str);
        }
        token = token->next;
    }
    fprintf(stderr,"\n\n");
}

int get_ident(char* p){
    int len = 0;
    while('a' <= *p && *p <= 'z'){
        len++;
        p++;
    }
    // fprintf(stderr,"len:%d\n",len);
    return len;
}

Token* new_token(TokenKind kind, Token* cur, char* p, int len){
    // fprintf(stderr, "type %d registered\n", kind);
    cur->next = (Token*)calloc(1, sizeof(Token));
    cur = cur->next;

    cur->kind = kind;
    cur->str = (char*)calloc(len, sizeof(char));
    strncpy(cur->str, p, len);
    cur->len = len;
    return cur;
}

Token* tokenize(char* p){
    Token* head = (Token*)calloc(1, sizeof(Token));
    Token* cur = head;

    int num;
    
    while(*p){
        // fprintf(stderr, "*p:%c\n", *p);
        // print_list(head->next);
        if (isspace(*p)){
            p++;
            continue;
        } else if (strncmp(p, "==", 2)==0 || strncmp(p, "!=", 2)==0 ||strncmp(p, "<=", 2)==0 || strncmp(p, ">=", 2)==0){
            cur = new_token(TK_RESERVED, cur, p, 2);
            // fprintf(stderr, "p: %s\n", p);
            p += 2;
            continue;
        } else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<'  || *p == '>'  || *p == '=' || *p == ';'){
            cur = new_token(TK_RESERVED, cur, p, 1);
            // fprintf(stderr, "p: %s\n", p);
            p++;
            continue;
        } else if (isdigit(*p)){
            cur = new_token(TK_NUM, cur, p, 1);
            // fprintf(stderr, "p: %s\n", p);
            num = strtol(p, &p, 10);
            cur->val = num;
            continue;
        } else if ('a' <= *p && *p <= 'z'){
            int len = get_ident(p);
            cur = new_token(TK_IDENT, cur, p, len);
            p += len;
            continue;
        }

        error_at(p, "invalid input\n");
    }

    cur = new_token(TK_EOF, cur, p, 0);
    return head->next;
}


// 構文木とノードの関数
void print_tree(Node* node, int depth){
    fprintf(stderr, "- type:%d", node->kind);
    if (node->kind == ND_NUM){
        fprintf(stderr, ",val:%d\n", node->val);
    } else if (node->kind == ND_LVAR){
        fprintf(stderr, ",offset:%d\n", node->offset);
    } else {
        fprintf(stderr, "\n");
        fprintf(stderr, "%*s", 2*depth, " ");
        print_tree(node->lhs, depth+1);
        fprintf(stderr, "%*s", 2*depth, " ");
        print_tree(node->rhs, depth+1);
    }
}

LVar* locals;

LVar* find_lvar(Token* tok){
    for (LVar* var = locals; var; var = var->next){
        if (var->len == tok->len && strncmp(var->name, tok->str, var->len) == 0)
            return var;
    }
    return NULL;
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

Node* new_node_ident(Token* tok){
    // fprintf(stderr, "number registered(value:%d)\n", val);
    Node* node = (Node*)calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar* lvar = find_lvar(tok);
    if (lvar){
        node->offset = lvar->offset;
    } else {
        lvar = (LVar*)calloc(1, sizeof(LVar));
        lvar->name = (char*)calloc(tok->len, sizeof(char));
        strncpy(lvar->name, tok->str, tok->len);
        lvar->len = tok->len;
        lvar->offset = locals->offset + 8;
        node->offset = lvar->offset;
        
        lvar->next = locals; // 逆向きに追加
        locals = lvar;
    }

    return node;
}


// パース関数
Node* code[100];

void parse_program(){
    int i=0;
    locals = (LVar*)calloc(1, sizeof(LVar));

    while(!at_eof()){
        code[i] = parse_stmt();
        // print_tree(code[i], 0);
        i++;
    }
    code[i] = NULL;
}

Node* parse_stmt(){
    Node* node = parse_expr();
    expect(";");
    return node;
}

Node* parse_expr(){
    return parse_assign();    
}

Node* parse_assign(){
    Node* node = parse_equality();

    if (consume("=")){
        node = new_node(ND_ASSIGN, node, parse_assign());
    }
    return node;
}

Node* parse_equality(){
    // fprintf(stderr, "parse_equality called\n");
    Node* node = parse_relational();

    for(;;){
        if(consume("==")){
            node = new_node(ND_EQ, node, parse_relational());
            // print_tree(node, 0);
            continue;
        } else if(consume("!=")){
            node = new_node(ND_NEQ, node, parse_relational());
            // print_tree(node, 0);
            continue;
        }
        return node;
    }
}

Node* parse_relational(){
    // fprintf(stderr, "parse_equality called\n");
    Node* node = parse_add();

    for(;;){
        if(consume("<")){
            node = new_node(ND_LT, node, parse_add());
            // print_tree(node, 0);
            continue;
        } else if(consume("<=")){
            node = new_node(ND_LEQ, node, parse_add());
            // print_tree(node, 0);
            continue;
        } else if(consume(">")){
            node = new_node(ND_LT, parse_add(), node);
            // print_tree(node, 0);
            continue;
        } else if(consume(">=")){
            node = new_node(ND_LEQ, parse_add(), node);
            // print_tree(node, 0);
            continue;
        }
        return node;
    }
}


Node* parse_add(){
    // fprintf(stderr, "parse_add called\n");
    Node* node = parse_mul();

    for(;;){
        if(consume("+")){
            node = new_node(ND_ADD, node, parse_mul());
            // print_tree(node, 0);
            continue;
        } else if(consume("-")){
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
        if(consume("*")){
            node = new_node(ND_MUL, node, parse_unary());
            // print_tree(node, 0);
            continue;
        } else if(consume("/")){
            node = new_node(ND_DIV, node, parse_unary());
            // print_tree(node, 0);
            continue;
        }
        return node;
    }
}

Node* parse_unary(){
    // fprintf(stderr, "parse_unary called\n");
    Node* node;
    if(consume("+")){
        node = parse_primary();
        // print_tree(node, 0);
    } else if(consume("-")){
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
    Token* tok = consume_ident();

    if(consume("(")){
        node = parse_expr();
        expect(")");
    } else if(tok){
        node = new_node_ident(tok);
    } else {
        int num = expect_number();
        node = new_node_num(num);
        // print_tree(node, 0);
    }
    return node;
}


// 読み込む関数
Token* consume_ident(){
    if (token->kind != TK_IDENT){
        return NULL;
    }
    Token* ret = token;
    token = token->next;
    return ret;
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

int consume(char* c){
    if (token->kind != TK_RESERVED || strlen(c) != token->len || strncmp(token->str, c, token->len)!=0){
        return false;
    }
    token = token->next;
    return true;
}

void expect(char* c){
    if (token->kind != TK_RESERVED || strlen(c) != token->len || strncmp(token->str, c, token->len)!=0){
        error_at(token->str, "expected '%c', but got unexpexted value\n", c);
    }
    token = token->next;
}