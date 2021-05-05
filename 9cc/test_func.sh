#!/bin/bash
assert() {
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
assert 42 "42;"

# 加減算
assert 21 "5+20-4;"

# func
assert 21 "a=1; foo();"
assert 27 "b=2; barbar();"
assert 48 "c=3; return foo() + barbar();";
assert 2 "twice(1);"
assert 4 "a=2;twice(a);"

assert 1 " i = 7; mod_t = i - (i/3)*3; mod_t;"
assert 2 " i = 8; mod_t = i - (i/3)*3; mod_t;"
assert 0 " i = 9; mod_t = i - (i/3)*3; mod_t;"
assert 4 " i = 9; mod_f = i - (i/5)*5; mod_f;"
assert 2 " i = 12; mod_f = i - (i/5)*5;mod_f;"


assert 31 "for(i=1; i<=30; i=i+1) { \
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


# ---- END ----
echo OK
