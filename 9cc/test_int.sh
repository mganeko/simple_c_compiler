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
assert 42 "int main() {42; }"


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

# ---- END ----
echo OK
exit 0

