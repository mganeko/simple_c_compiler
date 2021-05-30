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
assert 6 "int main() { int a[10]; a[0] = 1; a[1] = 2; a[1+1*1] = 3; return a[0] + a[1] + a[2];}"
assert 6 "int main() { int a[10]; int b; a[0] = 1; a[1] = 2; a[1+1*1] = 3; b = a[0] + a[1] + a[2];}"

assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; return a[0] + a[1];}"


#assert 10 "int main() { int i; for (i=0; i<10; i=i+1) { i; } i; }"
#assert 10 "int main() { int a[10]; int i; for (i=0; i<10; i=i+1) { i; } i; }"

assert 2 "int main() { int a[10]; int i; for (i=0; i<2; i=i+1) { a[i] = i+1; } a[1]; }"
#assert 3 "int main() { int a[10]; int i; a[0]=1; a[1]=2; a[2]=3;  a[2]; }"
#assert 4 "int main() { int a[10]; int i; a[0]=1; a[1]=2; a[2]=3; a[3]=4;  a[3]; }"

assert 5 "int main() { int a[10]; int i; a[0]=1; a[1]=2; a[2]=3; a[3]=4; a[4]=5;  a[4]; }"

#assert 3 "int main() { int a[10]; int i; for (i=0; i<3; i=i+1) { a[i] = i+1; } a[2]; }"
#assert 4 "int main() { int a[10]; int i; for (i=0; i<4; i=i+1) { a[i] = i+1; } a[3]; }"

#assert 5 "int main() { int a[10]; int i; for (i=0; i<5; i=i+1) { a[i] = i+1; } a[4]; }"
assert 9 "int main() { int a[10]; int i; for (i=0; i<9; i=i+1) { a[i] = i+1; } a[8]; }"

assert 15 "int main() { int a[10]; int i; for (i=0; i<9; i=i+1) { a[i] = i+1; } a[0] + a[1] + a[2] + a[3] + a[4]; }"
#assert 15 "int main() { int a[5]; int i; for (i=0; i<5; i=i+1) { a[i] = i+1; } a[0] + a[1] + a[2] + a[3] + a[4]; }"
#assert 10 "int main() { int a[10]; int b; int i; for (i=0; i<10; i=i+1) { b = i+1; } b; }"

#assert 2 "int main() { int a[20]; a[8] = 2; a[8]; }"
#assert 2 "int main() { int a[20]; a[9] = 2; a[9]; }"


assert 2 "int main() { int a[20]; a[10] = 2; a[10]; }"

assert 6 "int main() { int a[20]; a[9] = 1; a[10] = 2; a[11] = 3; a[9] + a[10] + a[11]; }"

#assert 10 "int main() { int a[20]; int b; int i; for (i=0; i<10; i=i+1) { a[i] = 10; } a[5]; }"
assert 10 "int main() { int a[10]; int i; for (i=0; i<10; i=i+1) { a[i] = i+1; } a[9]; }"


assert 55 "int add(int x, int y) { return x+y; } int main() { int a[10]; int i; for (i=0; i<10; i=i+1) { a[i] = i+1; } \
  int sum; sum = 0; for(i = 0; i < 10; i=i+1) sum = add(sum, a[i]); return sum;}"


assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; return a[0] + a[1]; }"
assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"
assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return p[0] + p[1]; }"

assert_outer 15 "int main() { int *p; int *q; int x; \
  alloc_four(&p, 1, 2, 4, 8); \
  x = p[0] + p[1] + p[2] + p[3]; \
  return x; }"

# ---- END ----
echo OK
exit 0

