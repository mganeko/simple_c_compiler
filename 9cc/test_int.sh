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
assert 2 "int add(int x, int y) { return x+y; } int main() { int a; a=1; int b; b=add(6, a); return b;}"

# ---- END ----
echo OK
exit 0

