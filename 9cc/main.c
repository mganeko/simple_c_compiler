#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;


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
  Node *node = expr();

  // --- start ---
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // --- 抽象構文木を下りながらコード生成 ---
  gen(node);

  // --- end ---
  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");

  return 0;
}