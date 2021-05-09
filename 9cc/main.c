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

// -- ユーザー定義関数のいったん保留場所 ---
Node *user_func[CODE_LINE_MAX];
int user_func_count = 0;

// --- top level local variable ---
// トップレベルのローカル変数
LVar *top_locals = NULL;


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
  //fprintf(stderr, "--- after tokenize ---\n");
  program(&top_locals);
  //fprintf(stderr, "--- after program ---\n");

  // --- start ---
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");


  /*--- mainも関数として生成される ---
  // プロローグ
  // (OLD: 変数26個分の領域を確保する)
  // DONE: 利用している変数の数だけ、領域を確保するように変更
  // DONE: 16バイト境界にそろえる
  //int stack_offset = 8* count_lvar();
  //int stack_offset = 16 * ((int)(count_lvar(top_locals)/2) + 1);
  int stack_offset = 16 * ((int)(count_lvar(top_locals)+1) /2);
  printf("main:\n");
  printf("  # -- main prologue --\n");
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  //printf("  sub rsp, 208\n"); // 26*8 = 208
  printf("  sub rsp, %d\n", stack_offset); 
  printf("  # -- main body --\n");
  --- mainも関数として生成される ---*/

  // --- 抽象構文木を下りながらコード生成 ---
  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    Node *node = code[i];

    if (node->kind == ND_FUNC_DEF) {
      // --- 関数定義は後で生成するので、保留にしておく ---
      user_func[user_func_count] = node;
      user_func_count++;
      user_func[user_func_count] = NULL;
    }
    else {
      gen(code[i]);
    }

    if (node->kind != ND_FUNC_DEF) {
      // 関数定義以外の場合、
      // 式の評価結果としてスタックに一つの値が残っている
      // はずなので、スタックが溢れないようにポップしておく
      printf("  pop rax # code in main\n");
    }
  }

  /*--- mainも関数として生成される ---
  // エピローグ
  // 最後の式の結果がRAXに残っているのでそれが返り値になる
  printf("  # -- main epilogue --\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  --- mainも関数として生成される ---*/

  // --- ユーザー定義関数を生成する ---
  for (int i = 0; user_func[i]; i++) {
    Node *node = user_func[i];
    if (node->kind == ND_FUNC_DEF) {
      //fprintf(stderr, "-- generating function name=%s\n", node->func_name);
      gen(node);
    }
  }

  return 0;
}