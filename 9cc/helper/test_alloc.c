#include <stdio.h>


// cc helper/outer_func.c helper/test_alloc.c -o test_alloc

int alloc_four(int **p, int a, int b, int c, int d);


void main() {
  int *p;
  alloc_four(&p, 1, 2, 3, 4);
  printf("%d %d %d %d\n", p[0], p[1], p[2], p[3]); // 1 2 3 4
  
  int *q = p+2;
  printf("%d\n", *q); // 3
}