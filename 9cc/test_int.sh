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
assert 42 "main() {42; }"


# --- type int ---

assert 1 "main() { int a; a=1; return a;}"
assert 2 "main() { int a; int b; a=1; b=a+1; return b;}"

# ---- END ----
echo OK
exit 0

