#include "compiler.h"

int label_num = 0;

void gen_lval(Node* node){
    if (node->kind != ND_LVAR){
        error("not a lvalue\n");
    }
    printf("\tmov rax, rbp\n");
    printf("\tsub rax, %d\n", node->offset);
    printf("\tpush rax\n");
    return;
}

void gen(Node* node){
    // fprintf(stderr, "gen called(kind:%d)\n", node->kind);
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
    } else if (node->kind == ND_LVAR){
        gen_lval(node);
        printf("\tpop rax\n");
        printf("\tmov rax, [rax]\n");
        printf("\tpush rax\n");
        return;
    } else if (node->kind == ND_RETURN){
        gen(node->lhs);
        printf("\tpop rax\n");
        printf("\tmov rsp, rbp\n");
        printf("\tpop rbp\n");
        printf("\tret\n");
        return;
    } else if (node->kind == ND_IF){
        int label = label_num;
        label_num++;

        gen(node->lhs->lhs);

        if (!node->rhs){ // elseがない場合
            printf("\tpop rax\n");
            printf("\tcmp rax, 0\n");
            printf("\tje .Lend%d\n", label);
            gen(node->lhs->rhs);
            printf(".Lend%d:\n", label);
        } else {
            printf("\tpop rax\n");
            printf("\tcmp rax, 0\n");
            printf("\tje .Lelse%d\n", label);
            gen(node->lhs->rhs);
            printf("\tjmp .Lend%d\n", label);
            printf(".Lelse%d:\n", label);
            gen(node->rhs);
            printf(".Lend%d:\n", label);
        }
        return;
    } else if (node->kind == ND_WHILE){
        int label = label_num;
        label_num++;

        printf(".Lbegin%d:\n", label);
        gen(node->lhs);
        printf("\tpop rax\n");
        printf("\tcmp rax, 0\n");
        printf("\tje .Lend%d\n", label);
        gen(node->rhs);
        printf("\tjmp .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
        return;
    } else if (node->kind == ND_FOR){
        int label = label_num;
        label_num++;

        gen(node->lhs->lhs->lhs);
        printf(".Lbegin%d:\n", label);
        gen(node->lhs->lhs->rhs);
        printf("\tpop rax\n");
        printf("\tcmp rax, 0\n");
        printf("\tje .Lend%d\n", label);
        gen(node->rhs);
        gen(node->lhs->rhs);
        printf("\tjmp .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
        return;
    } else if (node->kind == ND_BLOCK){
        while (node && node->lhs){
            gen(node->lhs);
            if (node->next) // 複文の最後の行では要らない
                printf("\tpop rax\n");
            node = node->next;
        }
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