#include "compiler.h"

void gen_lval(Node* node){
    if (node->kind != ND_LVAL){
        error("not a lvalue\n");
    }
    printf("\tmov rax, rbp\n");
    printf("\tsub rax, %d\n", node->offset);
    printf("\tpush rax\n");
    return;
}

void gen(Node* node){
    if (node->kind == ND_NUM){
        printf("\tpush %d\n", node->val);
        return;
    } else if (node->kind == ND_ASSIGN){
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("\tpop rdi\n");
        printf("\tpop rax\n");
        printf("\tmov [rax], rdi\n");
        printf("\tpush rdi\n");
        return;
    } else if (node->kind == ND_LVAL){
        gen_lval(node);
        printf("\tpop rax\n");
        printf("\tmov rax, [rax]\n");
        printf("\tpush rax\n");
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
    } else if (node->kind == ND_EQ){
        printf("\tcmp rax, rdi\n");
        printf("\tsete al\n");
        printf("\tmovzb rax, al\n");
    } else if (node->kind == ND_NEQ){
        printf("\tcmp rax, rdi\n");
        printf("\tsetne al\n");
        printf("\tmovzb rax, al\n");
    } else if (node->kind == ND_LT){
        printf("\tcmp rax, rdi\n");
        printf("\tsetl al\n");
        printf("\tmovzb rax, al\n");
    } else if (node->kind == ND_LEQ){
        printf("\tcmp rax, rdi\n");
        printf("\tsetle al\n");
        printf("\tmovzb rax, al\n");
    }
    printf("\tpush rax\n");
}