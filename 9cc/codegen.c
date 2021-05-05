#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

int label_index = 0;

void gen(Node *node) {
  int current_label;
  if (node->kind == ND_IF) {
    printf("  # -- start if --\n");

    // ラベル番号を確保、次のラベル用にカウントアップ
    current_label = label_index;
    label_index++;

    // 条件部
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax,0\n");
    printf("  je .Lelse%d\n", current_label);

    // 実行部
    gen(node->body);
    printf("  jmp .Lend%d\n", current_label);

    // else
    printf(".Lelse%d:\n", current_label);
    if (node->elsebody) {
      gen(node->elsebody);
    }

    // 終了
    printf(".Lend%d:\n", current_label);
    printf("  # -- end if --\n");
    return;
  }

  if (node->kind == ND_WHILE) {
    printf("  # -- start while --\n");

    // ラベル番号を確保、次のラベル用にカウントアップ
    current_label = label_index;
    label_index++;

    // 開始
    printf(".Lbegin%d:\n", current_label);

    // 条件部
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax,0\n");
    printf("  je .Lend%d\n", current_label);

    // 実行部
    gen(node->body);
    printf("  jmp .Lbegin%d\n", current_label);

    // 終了
    printf(".Lend%d:\n", current_label);
    printf("  # -- end while --\n");
    return;
  }

  if (node->kind == ND_FOR) {
    printf("  # -- start for --\n");
    // ラベル番号を確保、次のラベル用にカウントアップ
    current_label = label_index;
    label_index++;

    // 開始
    if(node->init)
      gen(node->init);
    printf(".Lbegin%d:\n", current_label);

    // 条件部
    if(node->cond) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax,0\n");
      printf("  je .Lend%d\n", current_label);
    }

    // 実行部
    gen(node->body);

    // 後処理
    if(node->post)
      gen(node->post);
    printf("  jmp .Lbegin%d\n", current_label);

    // 終了
    printf(".Lend%d:\n", current_label);
    printf("  # -- end for --\n");
    return;
  }

  if (node->kind == ND_BLOCK) {
    printf("  # -- start block --\n");
    fprintf(stderr, "-- Node BLOCK, counst=%d\n", node->stmts_count);
    int i;
    for (i=0; i < node->stmts_count; i++) {
      fprintf(stderr, "block line(%d)\n", i);
      printf("  # -block line-\n");
      gen(node->stmts[i]);

      // 途中の結果はスタックから取り除く（最後だけ残す）
      if (i < node->stmts_count-1)
        printf("  pop rax\n");
    }
    printf("  # -- end block --\n");
    return;
  }

  if (node->kind == ND_RETURN) {
    printf("  # -- return --\n");
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  if (node->kind == ND_FUNC_CALL) {
    printf("  # -- func call --\n");
    if (node->arg) { // 1 arg
      /*-- args --
        RDI	第1引数	✔
        RSI	第2引数	✔
        RDX	第3引数	✔
        RCX	第4引数	✔
        R8	第5引数	✔
        R9	第6引数	✔
      ----*/
      printf("  # - arg for func -\n");
      gen(node->arg);
      printf("  pop rdi\n");
    }
    printf("  call %s\n", node->func_name);
    printf("  push rax\n");
    printf("  # -- end func call --\n");
    return;
  }


  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  // 変数読み出し
  if (node->kind == ND_LVAR) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  // 変数代入
  if (node->kind == ND_ASSIGN) {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");

    return;
  }

  // --- 演算子 ----
  if (node->lhs && node->rhs) {
    gen(node->lhs);
    gen(node->rhs);
  }
  else {
    error("NOT Operator Node found, Kind(%d) while Genereting code \n", node->kind);
  }

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;

  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_GT:
    printf("  cmp rax, rdi\n");
    printf("  setg al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_GE:
    printf("  cmp rax, rdi\n");
    printf("  setge al\n");
    printf("  movzb rax, al\n");
    break;

  default:
    error("UNKNOWN Node Kind(%d) while Genereting code \n", node->kind);
  }

  printf("  push rax\n");
}
