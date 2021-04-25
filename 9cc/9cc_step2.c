#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  // -----
  char *p = argv[1];

  // --- start ---
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 1st value
  long int value = strtol(p, &p, 10);
  printf("  mov rax, %ld\n", value);

  // --- loop for other node ---
  while(*p) {
    if (*p == '+') {
      p++;
      value = strtol(p, &p, 10);
      printf("  add rax, %ld\n", value);
      continue;
    }
    
    if (*p == '-') {
      p++;
      value = strtol(p, &p, 10);
      printf("  sub rax, %ld\n", value);
      continue;
    }

    // --- other ---
    fprintf(stderr, "予期しない文字です: '%c'\n", *p);
    return 1;
  }

  // -- end ---
  printf("  ret\n");
  return 0;
}