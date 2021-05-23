#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# --- link with helper/outer_func.c --- 
assert_outer() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -c helper/outer_func.c 
  cc -o tmp tmp.s outer_func.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}


# 整数
assert 0 "int main() { 0; }"
assert 42 "int main() {42; }"

# 加減算
assert 21 "int main() {5+20-4;}"
assert 41 "int main() { 12 +   34 - 5 ;}"

# 四則演算
assert 47 'int main() {5+6*7;}'
assert 15 'int main() {5*(9-6);}'
assert 4 'int main() {(3+5)/2;}'

assert 15 'int main() {-3 * (-5);}'
assert 4 'int main() {+4;}'
assert 2 'int main() {-(3+5)+10;}'
assert 15 'int main() {-3*+5*-1;}'
assert 7 'int main() {+3-(-4);}'

# --- 比較 ---
assert 0 'int main() {1==2;}'
assert 1 'int main() {3 == 3;}'
assert 1 'int main() {-4==-4;}'
assert 1 'int main() {5!=50;}'
assert 0 'int main() {-6!= -6;}'
assert 1 'int main() {4*3==2*6;}'
assert 1 'int main() {4*3==(2 + 10);}'

assert 0 'int main() {1<1;}'
assert 1 'int main() {2<3;}'
assert 1 'int main() {-4<4;}'
assert 0 'int main() {5<-5;}'
assert 0 'int main() {-5<-5;}'
assert 1 'int main() {1<=1;}'
assert 0 'int main() {3<=2;}'
assert 1 'int main() {-4<=4;}'
assert 0 'int main() {5<=-5;}'
assert 1 'int main() {-5<=-5;}'

assert 0 'int main() {1>1;}'
assert 0 'int main() {2>3;}'
assert 1 'int main() {5>4;}'
assert 0 'int main() {-4>4;}'
assert 1 'int main() {5>-5;}'
assert 0 'int main() {-5>-5;}'
assert 1 'int main() {1>=1;}'
assert 0 'int main() {2>=3;}'
assert 1 'int main() {5>=4;}'
assert 0 'int main() {-4>=4;}'
assert 1 'int main() {5>=-5;}'
assert 1 'int main() {-5>=-5;}'


# 変数
#assert 0 "a;" NG on Ubuntu 18.04
assert 0 "int main() {int a; a=0;}"
assert 1 "int main() {int a;a=1;}"
assert 2 "int main() {int z; z=2;}";
assert 2 "int main() {int b; b=(1+2+3-6*2+10)/2;}"
assert 2 "int main() {int a; a=1; a=a+1;}"
assert 3 "int main() {int a; int b; a=1;b=2;a+b;}"
assert 6 "int main() {int foo; int bar; foo=1;bar=2+3;foo+bar;}"
assert 30 "int main() {int a; int b; int c; int d; int e; int f; int g; int h; int i; int j; int k; \
 int l; int m; int n; int o; int p; int q; int r; int s; int t; int u; int v; int w; int x; int y; int z; \
 int AA; int BB; int CC; int DD;   
 a=1;b=1;c=1;d=1;e=1;f=1;g=1;h=1;i=1;j=1;k=1;l=1;m=1;n=1;o=1;p=1;q=1;r=1;s=1;t=1;u=1;v=1;w=1;x=1;y=1;z=1; \
 AA=1;BB=1;CC=1;DD=1; \
 a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+AA+BB+CC+DD;}"
assert 4 "int main() {int a; a=1;a+(a+a*2);}"

# return
assert 2 "int main() {return 2;}"
assert 3 "int main() {return 1+2;}"
assert 3 "int main() {return (1+2);}"
assert 5 "int main() {int a; int b; a=1;b=2; return a+b * b;}"

# if/else
assert 2 "int main() {int a; a=1; if (a==1) return 2; return 3;}"
assert 3 "int main() {int a; a=1; if (a!=1) return 2; return 3;}"
assert 2 "int main() {int a; a=1; if (a==1) a = a +1; return a;}"
assert 2 "int main() {int a; a=1; if (a==1) return 2; else a = 3; return a;}"
assert 2 "int main() {int a; a=3; if (a!=1) return 2; else a = 3; return a;}"
assert 2 "int main() {int a; a=1; if (a==1) a=2; else a = 3; return a;}"

assert 2 "int main() {int a; a=1; if (a<5) if (a==1) return 2; else a = 3; else a=6; return a;}"
assert 3 "int main() {int a; a=2; if (a<5) if (a==1) return 2; else a = 3; else a=6; return a;}"
assert 6 "int main() {int a; a=8; if (a<5) if (a==1) return 2; else a = 3; else a=6; return a;}"

assert 2 "int main() {int a; a=1; if (a<5) if (a==1) return 2; else a = 3; else if (a>10) a=11; else a=9; return a;}"
assert 3 "int main() {int a; a=2; if (a<5) if (a==1) return 2; else a = 3; else if (a>10) a=11; else a=9; return a;}"
assert 11 "int main() {int a; a=12; if (a<5) if (a==1) return 2; else a = 3; else if (a>10) a=11; else a=9; return a;}"
assert 9 "int main() {int a; a=6; if (a<5) if (a==1) return 2; else a = 3; else if (a>10) a=11; else a=9; return a;}"

# while
assert 2 "int main() {int a; a=10; while(a) a = a - 1;a+2;}"
assert 7 "int main() {int a; a=10; while(a > 5) a = a - 1;a+2;}"
assert 55 "int main() {int sum; int i; sum=0; i=0; while((i=i+1) <=10) sum = sum + i; return sum;}"

# for
assert 55 "int main(){int sum; int i; sum=0; for(i=1; i<=10; i=i+1) sum=sum+i;sum;}"
assert 55 "int main(){int sum; int i; sum=0; i=1; for(; i<=10; i=i+1) sum=sum+i;sum;}"
assert 55 "int main() {int sum; int i; sum=0; i=0; for(; i<10; ) sum=sum+(i=i+1);sum;}"
#assert 55 "sum=0; i=0; for(; ;) sum=sum+(i=i+1);sum;"  # 無限ループ

# block
assert 2 "int main() {int a; a = 1; if (a>0) {a=2;} return a;}"
assert 55 "int main() {int sum; int i; sum=0; for(i=1; i<=10; i=i+1) { sum = sum + i;  sum = sum + 0; } return sum;}"
assert 55 "int main() {int sum; int i; sum=0; for(i=1; i<=20; i=i+1) { sum = sum + i;  if (i>=10)  return sum; } return 95;}"
assert 55 "int main() {int sum; int i; sum=0; for(i=1; i<=10;) { sum = sum + i; i= i+1; } return sum;}"
assert 55 "int main() {int sum; int i; sum=0; for(i=1; ; i=i+1) { sum = sum + i; if (i>=10) return sum; } return 96;}"
assert 55 "int main() {int sum; int i; sum=0; i=1; while(i<=10) { sum=sum+i; i=i+1; } sum;}"
assert 55 "int main() {int sum; int i; sum=0; for(i=1; i<=12;) { sum = sum + i; i=i+1; if (i>10)  return sum; } return 98;}"
assert 55 "int main() {int i; int sum; sum=0; i=1; for(; ;) { sum = sum + i; i= i+1; if (i> 10)  return sum;} return 99;}"


# func
assert_outer 21 "int main() { int a; a=1; foo(); }"
assert_outer 27 "int main() { int b; b=2; barbar(); }"
assert_outer 48 "int main() { int c; c=3; return foo() + barbar(); }";
assert_outer 2 "int main() { twice_outer(1); }"
assert_outer 4 "int main() { int a; a=2;twice_outer(a); }"

assert_outer 1 "int main() {  int i; int mod_t; i = 7; mod_t = i - (i/3)*3; mod_t; }"
#assert_outer 2 "int main() {  i = 8; mod_t = i - (i/3)*3; mod_t; }"
#assert_outer 0 "int main() {  i = 9; mod_t = i - (i/3)*3; mod_t; }"
#assert_outer 4 "int main() {  i = 9; mod_f = i - (i/5)*5; mod_f; }"
#assert_outer 2 "int main() {  i = 12; mod_f = i - (i/5)*5;mod_f; }"


assert_outer 31 "int main() { \
 int i; int mod_t; int mod_f;  \
 for(i=1; i<=30; i=i+1) { \
  mod_t = i - (i/3)*3; mod_f = i - (i/5)*5; \
  if (mod_t == 0) { \
    if (mod_f == 0) fizzbuzz(); \
    else fizz(); \
  } \
  else {
    if (mod_f == 0) buzz(); \
    else num(i); \
  } \
 } i; }"

assert_outer 3 "int main() { add_outer(1, 2); }"
assert_outer 4 "int main() { int a; a=1; add_outer(a, a+a*2); }"

assert_outer 15 "int main() { int a; a=0; sum_six(a, a+1, a+2, a+3, a+4, a+1+2*2); }"

# compile error MUST occur
assert_outer 127 "int main() { int a; a=0; sum_six(a, a+1, a+2, a+3, a+4, a+1+2*2, 99); }"

# ------------

# define user func
assert_outer 2 "int two() { 2; } int main() {two();}"
assert_outer 2 "int two() { return 2; } int main() {two();}"

assert_outer 3 "int three() { return 1+2; } int main() {three();}"
assert_outer 3 "int main() {three();} int three() { return 3; }"
assert_outer 3 "int main() {int a; a=1; three(); } int three() { return 3; }"


# use local var in func
#assert_outer 3 "a=3; three(); three() { return a; }" must be NG
assert_outer 4 "int main() { int a; int aa; a=1; aa=2; three(); } int three() { int b; b = 3; return 4; }"
assert_outer 4 "int main() { int a; a=1; three();} int three() { int b;  b = 3; return 4; } "
assert_outer 3 "int main() { int a; a=1; three();} int three() { int b; b = 3; return b; } "
assert_outer 3 "int main() { int a; a=1; three();} int three() { int b;b = 3; return b; }"
assert_outer 3 "int main() { int a; a=1; three();} int three() { int b;int c;b = 1; c =2; return b+c; }"

assert_outer 3 "int main() { int a; a=1; two();} int two() { return three(); } int three() { int b; int c;  b = 1; c =2; return b+c; }"


# func with args
assert_outer 2 "int twice(int x) { return x*2; } int main() {twice(1);}"
assert_outer 3 "int add(int x, int y) { return x+y; } int main() {int a; a=1; add(a, 1+1);}"
assert_outer 6 "int add(int x, int y) { return x+y; } int main() {int x; x=1; add(5, x);}"

assert_outer 31 "int mod(int x, int base) { return x - (x/base)*base; }   \
int main() { \
 int i; int mod_t; int mod_f; 
 for(i=1; i<=30; i=i+1) { \
  mod_t = i - (i/3)*3; mod_f = i - (i/5)*5; \
  if (mod_t == 0) { \
    if (mod_f == 0) fizzbuzz(); \
    else fizz(); \
  } \
  else {
    if (mod_f == 0) buzz(); \
    else num(i); \
  } \
 } i; }"


assert_outer 34 "int fib(int x) { \
  if (x <= 1) { return x; } \
  else { return fib(x - 1) + fib(x - 2); } \
}\
int main() { \
  int f; int i;
  f = 0;
  for (i=0; i < 10; i=i+1) { f=fib(i); num(f); } \
  return f; \
}"

# ---- addr ---

#NOT OK 型の不一致; assert 2 "int main() { int a; int b; a=1; b=&a; a=2; return *b;}"
#NOT OK 型の不一致; assert 3 "int main() { int x; int y; int z; x = 3; y = 5; z = &y + 8; return *z;}"
#NOT OK 型の不一致; assert 6 "int main() { int x; int y; int z; x = 3; y = &x; z = y; return *z * 2;}"

# --- type int ---
assert 1 "int main() { int a; a=1; return a;}"
assert 2 "int main() { int a; int b; a=1; b=a+1; return b;}"

assert 2 "int double(int x) { return 2*x; } int main() { int a; a=1; int b; b=double(a); return b;}"
assert 7 "int add(int x, int y) { return x+y; } int main() { int a; a=1; int b; b=add(6, a); return b;}"

# --- pointer --
assert 5 "int getValuePP(int **addr) { return **addr;} int main() { int a; int *b; int **c; a=5; b=&a; c=&b; return getValuePP(c); }"
assert 4 "int main() { int a; int *b; b=&a; *b = 4; }"
assert 3 "int main() { int a; a=1; int *b; b=&a; *b = 4; return a - 1; }"
assert 10 "int setValuePP(int **addr, int x) { **addr = x;} int main() { int a; int *b; int **c; a=5; b=&a; c=&b; setValuePP(c, a+5); return a; }"

assert_outer 4 "int main() { int *p; int *q; int x; \
  alloc_four(&p, 1, 2, 4, 8); \
  q = p + 2;  \
  *q; }"

assert_outer 8 "int main() { int *p; int *q; int x; \
  alloc_four(&p, 1, 2, 4, 8); \
  q = p + 3; \
  return *q; }"

assert_outer 10 "int main() { int *p; int *q; int x; \
  alloc_four(&p, 1, 2, 4, 8); \
  q = p + 3; \
  x = *q;
  q = q - 2;
  return *q + x; }"

assert_outer 8 "int main() { int *p; int *q; int x; \
  alloc_four(&p, 1, 2, 4, 8); \
  q = 3 + p; \
  return *q; }"

assert_outer 8 "int main() { int *p; int *q; int x; \
  alloc_four(&p, 1, 2, 4, 8); \
  q = 1 + p + 2; \
  return *q; }"

# --- sizeof ---

assert 4 "int main() { sizeof(1); }"
assert 4 "int main() { int a; sizeof(a); }"
assert 8 "int main() { int a; sizeof(&a); }"
assert 8 "int main() { int *a; sizeof(a); }"
assert 8 "int main() { int **a; sizeof(a); }"
assert 8 "int main() { int ***a; sizeof(a); }"

assert 4 "int main() { int *a; sizeof(*a); }"
assert 8 "int main() { int **a; sizeof(*a); }"
assert 8 "int main() { int ***a; sizeof(*a); }"

assert 4 "int main() { int **a; sizeof(**a); }"
assert 8 "int main() { int ***a; sizeof(**a); }"
assert 4 "int main() { int ***a; sizeof(***a); }"

assert 4 "int add(int x, int y) { return x+y; } int main() { sizeof(add(1, 2)); }"

assert 4 "int main() { int a; sizeof(1 + 2); }"
assert 4 "int main() { int a; sizeof(1 + 2*a); }"
assert 4 "int main() { int a; sizeof(a == 1); }"
assert 4 "int main() { int a; sizeof(a > 1); }"
assert 4 "int add(int x, int y) { return x+y; } int main() { int a; sizeof(1+add(1, 2)+a); }"


assert 8 "int main() { int *a; sizeof(a + 1); }"
assert 8 "int main() { int *a; sizeof(1 + a); }"
assert 8 "int main() { int *a; sizeof(a + 1 + 2 - 1); }"
assert 8 "int main() { int *a; sizeof(1 + 2 + a - 1 - 1); }"

assert 8 "int main() { int **a; sizeof(a + 1); }"

assert 127 "int main() { int *a; int *b; sizeof(a + b); }" # ERROR:127

assert 4 "int main() { int *a; sizeof(*(a + 1 + 2 - 1)); }"
assert 8 "int main() { int **a; sizeof(*(a + 1)); }"

# ---- END ----
echo OK
exit 0

