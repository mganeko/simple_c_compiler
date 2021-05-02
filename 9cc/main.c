#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

// // 現在着目しているトークン
// Token *token;

// // 入力プログラム
// char *user_input;


int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  // --- 引数を取得 ---
  char *p = argv[1];
  user_input = p;

  // トークナイズする
  token = tokenize(user_input);
  fprintf(stderr, "--- after tokenize ---\n");
  //Node *node = expr();
  program();
  fprintf(stderr, "--- after program ---\n");

  // --- start ---
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // プロローグ
  // (OLD: 変数26個分の領域を確保する)
  // DONE: 利用している変数の数だけ、領域を確保するように変更
  int stack_offset = 8*count_lvar();
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  //printf("  sub rsp, 208\n"); // 26*8 = 208
  printf("  sub rsp, %d\n", stack_offset); 

  // --- 抽象構文木を下りながらコード生成 ---
  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がRAXに残っているのでそれが返り値になる
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");

  return 0;
}