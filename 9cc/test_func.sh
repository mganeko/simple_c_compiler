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


# 整数
assert_outer 42 "42;"

# 加減算
assert_outer 21 "5+20-4;"

# func
assert_outer 21 "a=1; foo();"
assert_outer 27 "b=2; barbar();"
assert_outer 48 "c=3; return foo() + barbar();";
assert_outer 2 "twice(1);"
assert_outer 4 "a=2;twice(a);"

assert_outer 1 " i = 7; mod_t = i - (i/3)*3; mod_t;"
assert_outer 2 " i = 8; mod_t = i - (i/3)*3; mod_t;"
assert_outer 0 " i = 9; mod_t = i - (i/3)*3; mod_t;"
assert_outer 4 " i = 9; mod_f = i - (i/5)*5; mod_f;"
assert_outer 2 " i = 12; mod_f = i - (i/5)*5;mod_f;"


assert_outer 31 "for(i=1; i<=30; i=i+1) { \
  mod_t = i - (i/3)*3; mod_f = i - (i/5)*5; \
  if (mod_t == 0) { \
    if (mod_f == 0) fizzbuzz(); \
    else fizz(); \
  } \
  else {
    if (mod_f == 0) buzz(); \
    else num(i); \
  } \
} i;"

assert_outer 3 "add(1, 2);"
assert_outer 4 "a=1; add(a, a+a*2);"

assert_outer 15 "a=0; sum_six(a, a+1, a+2, a+3, a+4, a+1+2*2);"

# compile error MUST occur
assert_outer 127 "a=0; sum_six(a, a+1, a+2, a+3, a+4, a+1+2*2, 99);"

# ---- END ----
echo OK
