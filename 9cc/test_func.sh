#!/bin/bash
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







# ------------

# 整数
assert_outer 42 "main() {42;}"

# 加減算
assert_outer 21 "main() { 5+20-4;}"

# func
assert_outer 21 "main() { a=1; foo(); }"
assert_outer 27 "main() { b=2; barbar(); }"
assert_outer 48 "main() { c=3; return foo() + barbar(); }";
assert_outer 2 "main() { twice_outer(1); }"
assert_outer 4 "main() { a=2;twice_outer(a); }"

assert_outer 1 "main() {  i = 7; mod_t = i - (i/3)*3; mod_t; }"
assert_outer 2 "main() {  i = 8; mod_t = i - (i/3)*3; mod_t; }"
assert_outer 0 "main() {  i = 9; mod_t = i - (i/3)*3; mod_t; }"
assert_outer 4 "main() {  i = 9; mod_f = i - (i/5)*5; mod_f; }"
assert_outer 2 "main() {  i = 12; mod_f = i - (i/5)*5;mod_f; }"


assert_outer 31 "main() { \
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

assert_outer 3 "main() { add_outer(1, 2); }"
assert_outer 4 "main() { a=1; add_outer(a, a+a*2); }"

assert_outer 15 "main() { a=0; sum_six(a, a+1, a+2, a+3, a+4, a+1+2*2); }"

# compile error MUST occur
assert_outer 127 "main() { a=0; sum_six(a, a+1, a+2, a+3, a+4, a+1+2*2, 99); }"

# ------------

# define user func
assert_outer 2 "two() { 2; } main() {two();}"
assert_outer 2 "two() { return 2; } main() {two();}"

assert_outer 3 "three() { return 1+2; } main() {three();}"
assert_outer 3 "main() {three();} three() { return 3; }"
assert_outer 3 "main() {a=1; three(); } three() { return 3; }"


# use local var in func
#assert_outer 3 "a=3; three(); three() { return a; }" must be NG
assert_outer 4 "main() { a=1; aa=2; three(); } three() { b = 3; return 4; }"
assert_outer 4 "main() { a=1; three();} three() { b = 3; return 4; } "
assert_outer 3 "main() { a=1; three();} three() { b = 3; return b; } "
assert_outer 3 "main() { a=1; three();} three() { b = 3; return b; }"
assert_outer 3 "main() { a=1; three();} three() { b = 1; c =2; return b+c; }"

assert_outer 3 "main() { a=1; two();} two() { return three(); } three() { b = 1; c =2; return b+c; }"


# func with args
assert_outer 2 "twice(x) { return x*2; } main() {twice(1);}"
assert_outer 3 "add(x, y) { return x+y; } main() {a=1; add(a, 1+1);}"
assert_outer 6 "add(x, y) { return x+y; } main() {x=1; add(5, x);}"


# ---- END ----
echo OK
exit 0

