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
assert 0 "0;"
assert 42 "42;"

# 加減算
assert 21 "5+20-4;"
assert 41 " 12 +   34 - 5 ;"

# 四則演算
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

assert 15 '-3 * (-5);'
assert 4 '+4;'
assert 2 '-(3+5)+10;'
assert 15 '-3*+5*-1;'
assert 7 '+3-(-4);'

# --- 比較 ---
assert 0 '1==2;'
assert 1 '3 == 3;'
assert 1 '-4==-4;'
assert 1 '5!=50;'
assert 0 '-6!= -6;'
assert 1 '4*3==2*6;'
assert 1 '4*3==(2 + 10);'

assert 0 '1<1;'
assert 1 '2<3;'
assert 1 '-4<4;'
assert 0 '5<-5;'
assert 0 '-5<-5;'
assert 1 '1<=1;'
assert 0 '3<=2;'
assert 1 '-4<=4;'
assert 0 '5<=-5;'
assert 1 '-5<=-5;'

assert 0 '1>1;'
assert 0 '2>3;'
assert 1 '5>4;'
assert 0 '-4>4;'
assert 1 '5>-5;'
assert 0 '-5>-5;'
assert 1 '1>=1;'
assert 0 '2>=3;'
assert 1 '5>=4;'
assert 0 '-4>=4;'
assert 1 '5>=-5;'
assert 1 '-5>=-5;'


# 変数
#assert 0 "a;" NG on Ubuntu 18.04
assert 0 "a=0;"
assert 1 "a=1;"
assert 2 "z=2;";
assert 2 "b=(1+2+3-6*2+10)/2;"
assert 3 "a=1;b=2;a+b;"
assert 6 "foo=1;bar=2+3;foo+bar;"
assert 30 "a=1;b=1;c=1;d=1;e=1;f=1;g=1;h=1;i=1;j=1;k=1;l=1;m=1;n=1;o=1;p=1;q=1;r=1;s=1;t=1;u=1;v=1;w=1;x=1;y=1;z=1;AA=1;BB=1;CC=1;DD=1;a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+AA+BB+CC+DD;"
assert 4 "a=1;a+(a+a*2);"

# return
assert 2 "return 2;"
assert 3 "return 1+2;"
assert 3 "return (1+2);"
assert 5 "a=1;b=2; return a+b * b;"

# if/else
assert 2 "a=1; if (a==1) return 2; return 3;"
assert 3 "a=1; if (a!=1) return 2; return 3;"
assert 2 "a=1; if (a==1) a = a +1; return a;"
assert 2 "a=1; if (a==1) return 2; else a = 3; return a;"
assert 2 "a=3; if (a!=1) return 2; else a = 3; return a;"
assert 2 "a=1; if (a==1) a=2; else a = 3; return a;"

assert 2 "a=1; if (a<5) if (a==1) return 2; else a = 3; else a=6; return a;"
assert 3 "a=2; if (a<5) if (a==1) return 2; else a = 3; else a=6; return a;"
assert 6 "a=8; if (a<5) if (a==1) return 2; else a = 3; else a=6; return a;"

assert 2 "a=1; if (a<5) if (a==1) return 2; else a = 3; else if (a>10) a=11; else a=9; return a;"
assert 3 "a=2; if (a<5) if (a==1) return 2; else a = 3; else if (a>10) a=11; else a=9; return a;"
assert 11 "a=12; if (a<5) if (a==1) return 2; else a = 3; else if (a>10) a=11; else a=9; return a;"
assert 9 "a=6; if (a<5) if (a==1) return 2; else a = 3; else if (a>10) a=11; else a=9; return a;"

# while
assert 2 "a=10; while(a) a = a - 1;a+2;"
assert 7 "a=10; while(a > 5) a = a - 1;a+2;"
#assert 2 "a=1; a=a+1;"
assert 55 "sum=0; i=0; while((i=i+1) <=10) sum = sum + i; return sum;"

# for
assert 55 "sum=0; for(i=1; i<=10; i=i+1) sum=sum+i;sum;"
assert 55 "sum=0; i=1; for(; i<=10; i=i+1) sum=sum+i;sum;"
assert 55 "sum=0; i=0; for(; i<10; ) sum=sum+(i=i+1);sum;"
#assert 55 "sum=0; i=0; for(; ;) sum=sum+(i=i+1);sum;"  # 無限ループ

# block
assert 2 "a = 1; if (a>0) {a=2;} return a;"
assert 55 "sum=0; for(i=1; i<=10; i=i+1) { sum = sum + i;  sum = sum + 0; } return sum;"
assert 55 "sum=0; for(i=1; i<=20; i=i+1) { sum = sum + i;  if (i>=10)  return sum; } return 95;"
assert 55 "sum=0; for(i=1; i<=10;) { sum = sum + i; i= i+1; } return sum;"
assert 55 "sum=0; for(i=1; ; i=i+1) { sum = sum + i; if (i>=10) return sum; } return 96;"
assert 55 "sum=0; i=1; while(i<=10) { sum=sum+i; i=i+1; } sum;"
assert 55 "sum=0; for(i=1; i<=12;) { sum = sum + i; i=i+1; if (i>10)  return sum; } return 98;"
assert 55 "sum=0; i=1; for(; ;) { sum = sum + i; i= i+1; if (i> 10)  return sum;} return 99;"


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
