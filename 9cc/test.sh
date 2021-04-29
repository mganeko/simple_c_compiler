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
assert 0 "a;"
assert 1 "a=1;"
assert 2 "z=2;";
assert 2 "b=(1+2+3-6*2+10)/2;"
assert 3 "a=1;b=2;a+b;"
assert 6 "foo=1;bar=2+3;foo+bar;"

echo OK
