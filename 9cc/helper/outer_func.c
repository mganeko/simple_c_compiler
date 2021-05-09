// cc -c helper/outer_func.c 

# include <stdio.h>

int foo() { printf("foo OK\n"); return 21;}
int barbar() { printf("barbar OK\n"); return 27;}
int twice_outer(int n) { printf("twice n=%d\n", n); return n*2; }

int fizz() { printf("fizz\n"); return 3; }
int buzz() { printf("buzz\n"); return 5; }
int fizzbuzz() { printf("fizzbuzz\n"); return 15; }
int num(int n) { printf("%d\n", n); return n; }

int add_outer(int a, int b) { printf("add %d, %d --> %d\n", a, b, a+b); return a + b; }
int sum_six(int a, int b, int c, int d, int e, int f) { printf("sum %d, %d, %d, %d, %d, %d\n", a, b, c, d, e, f); return a + b + c + d + e + f; }
