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
assert 30 "a=1;b=1;c=1;d=1;e=1;f=1;g=1;h=1;i=1;j=1;k=1;l=1;m=1;n=1;o=1;p=1;q=1;r=1;s=1;t=1;u=1;v=1;w=1;x=1;y=1;z=1;AA=1;BB=1;CC=1;DD=1;a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+AA+BB+CC+DD;"

# return
assert 2 "return 2;"
assert 3 "return 1+2;"
assert 3 "return (1+2);"
assert 5 "a=1;b=2; return a+b * b;"

echo OK
