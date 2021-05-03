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
assert 42 "42;"

# 加減算
assert 21 "5+20-4;"

# block
assert 2 "a = 1; if (a>0) {a=2;} return a;"
assert 55 "sum=0; for(i=1; i<=10; i=i+1) { sum = sum + i;  sum = sum + 0; } return sum;"
assert 55 "sum=0; for(i=1; i<=20; i=i+1) { sum = sum + i;  if (i>=10)  return sum; } return 95;"
assert 55 "sum=0; for(i=1; i<=10;) { sum = sum + i; i= i+1; } return sum;"
assert 55 "sum=0; for(i=1; ; i=i+1) { sum = sum + i; if (i>=10) return sum; } return 96;"
assert 55 "sum=0; i=1; while(i<=10) { sum=sum+i; i=i+1; } sum;"

assert 77 "sum=0; for(i=1; i<=15;) { sum = sum + i; i=i+1; 2; if (i>10)  return 77; } return sum;"
assert 55 "sum=0; for(i=1; i<=12;) { sum = sum + i; i=i+1; if (i>10)  return sum; } return sum;"
assert 55 "sum=0; for(i=1; i<=12;) { sum = sum + i; i=i+1; if (i>10)  return sum; } return 98;"
assert 55 "sum=0; i=1; for(; ;) { sum = sum + i; i= i+1; if (i> 10)  return sum;} return 99;"


# ---- END ----
echo OK
