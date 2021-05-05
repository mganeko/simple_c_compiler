// cc -c helper/outer_func.c 

# include <stdio.h>

int foo() { printf("foo OK\n"); return 21;}
int barbar() { printf("barbar OK\n"); return 27;}
int twice(int n) { printf("twice n=%d\n", n); return n*2; }

int fizz() { printf("fizz\n"); return 3; }
int buzz() { printf("buzz\n"); return 5; }
int fizzbuzz() { printf("fizzbuzz\n"); return 15; }
int num(int n) { printf("%d\n", n); return n; }