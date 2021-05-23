#include <stdio.h>


// cc helper/outer_func.c helper/test_alloc.c -o test_alloc  && ./test_alloc

int alloc_four(int **p, int a, int b, int c, int d);
void try_sizeof();

void main() {
  int *p;
  alloc_four(&p, 1, 2, 3, 4);
  printf("%d %d %d %d\n", p[0], p[1], p[2], p[3]); // 1 2 3 4
  
  int *q = p+2;
  printf("%d\n", *q); // 3

  printf("----- sizeof ------\n");
  try_sizeof();
}

void try_sizeof() {
  int a;
  int *p;
  printf("sizeof(int):%ld, sizeof(1):%ld, sizeof(int a):%ld sizeof(a+1):%ld \n", sizeof(int), sizeof(1), sizeof(a), sizeof(a+1) );
  printf("sizeof(&a):%ld, sizeof(p):%ld, sizeof(*p):%ld, sizeof(p+1):%ld, sizeof(1+p):%ld \n", sizeof(&a), sizeof(p), sizeof(*p), sizeof(p+1), sizeof(1+p) );

}