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



# --- Array ---

assert 3 "int main() { int a[10]; int b; b=1; b+2;}"
assert 4 "int main() { int a[1];  a[0] = 4;}"
assert 5 "int main() { int a[1]; a[0] = 5; return a[0];}"

assert 3 "int main() { int a[10]; a[0] = 1; a[1] = 2; return a[0] + a[1];}"


# ---- END ----
echo OK
exit 0

