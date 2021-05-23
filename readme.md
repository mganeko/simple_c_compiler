# Study Building C Compiler

## 参照
https://www.sigbus.info/compilerbook

## Dockerを使ったビルド環境

https://www.sigbus.info/compilerbook#docker

Docker Imageビルド

```
$ docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile
```

or Dockerfileで、明示的にamd64を指定

```
$ docker build -t compilerbook .
```


or オプションでプラットフォームを指定

```
$ docker build --platform linux/amd64 -t compilerbook0 -f Dockerfile_0 .
```


Dockerで実行

```
$ docker run --rm compilerbook ls /
```

alias: cbookrun

Make test

```
$ docker run --rm -v `pwd`/1cc:/1cc -w /1cc compilerbook make test
```
alias: cbookmake

マウントしながらインタラクティブ実行

```
$ docker run --rm -it -v `pwd`/9cc:/9cc compilerbook
```

alias cbookit


Windowsの場合

```
$ docker run --rm -it -v C:/path/to/repo/9cc:/9cc compilerbook
```



Online Compiler
https://godbolt.org


## 再帰下降構文解析

```
expr    = mul ("+" mul | "-" mul)*
mul     = primary ("*" primary | "/" primary)*
primary = num | "(" expr ")"
```

## Step 7

new_token() で、長さを知るための引数が必要ではないか？
- 長さを与える
- or 終了位置を与える（リファレンス実装はこちら）

## Step 8

void gen(Node *node)  のサンプルで case '+': となっているのは case ND_ADD 等ではないか？

## Step 10

lvar->offset = locals->offset + 8; // locals == NULL の時がある

## Step 11

tokenize() tokens[i].ty = TK_RETURN;

※tokensはまだない

Node *stmt() {
  if (consume(TK_RETURN)) { // <-- 今ある関数とは違う

if (!consume(';'))  // <-- 今ある関数とは違う

# Step 12
if, while for

# Step 13
block {}

# Step 14
call function

## assemble test

```
# include <stdio.h>
int foo() { printf("foo OK\n"); return 21;}
```

$ cc -c helper/outer_func.c 
  --> ./outer_func.o

-- bin

$ cc -o call call.s

$ ./call ; echo $?
--> 1

```
.intel_syntax noprefix
.globl main
main:
  push 1
  pop rax
  call foo
  ret
```

戻り値は rbx? --> rax





$ cc -o call call.s outer_func.o 
$ ./call ; echo $?
-->
foo OK
21

## func with args

- RDI	第1引数
- RSI	第2引数
- RDX	第3引数
- RCX	第4引数
- R8	第5引数
- R9	第6引数

TODO
- [x] RSPが16の倍数

# Step 15

Func def

- [x] return from user func
- [x] function local variable
  - [x] locals を引数にして、関数内では改めて生成する
- [x] function args as local variable


# Step 18

- [x] 変数でポインタ型を宣言できるように
  - [x] ポインタのポインタのポインタ型も
- [x] ポインタの指す値が参照出来る様に
  - [x] ポインタ中の中の中、が参照できるように
- [x] ポインタの指す先に、値が代入できるように
  - [x] ポインタ先の先に、値が代入できるように

# Step 19

- [x] 今の演算の型を判定する
  - [x] intの場合
  - [x] intへのポインターの場合 4バイト単位
  - [x] ポインターへのポインターの場合 8バイト単位
- [x] int *p; p+1
- [ ] int *p; 1+p
- [ ] int *p; p+1+2
- [ ] int *p; int **q; q+1
 

# Step 20 - sizeof

- [x] tokenize sizeof
- [ ] parse sizeof
  - [ ] sizeofの対象をパース
    - [x] 簡易
    - [ ] ドキュメントの通り
  - [ ] 対象の型を判定する int or pointer --> 一度でもポインターが出てきたらポインター
    - [x]] 整数:sizeof(1) --> 4
    - [x] int型 変数: int a; sizeof(a) --> 4
    - [ ] sizeof(int) --> 4
    - [ ] sizeof(1+2) --> 4
    - [ ] sizeof(a+1) --> 4
    - [ ] 関数の戻り値を含む 式
    - [x] アドレスの中身 int *a; sizeoof(*a) --> 4
    - [x] アドレスの中身 int **a; sizeoof(*a) --> 8
    - [ ] アドレスの中身 int **a; sizeoof(**a) --> 4
    - [ ] アドレスの中身 int ***a; sizeoof(**a) --> 8
    - [x] アドレス & --> 8
    - [x] ポンター型変数 int*, int**,  int***
    - [ ] ポインター演算 
    - [ ] ポインター演算の中身 (4 or 8)
  - [x] Node ND_NUM 4/8に置き換える
  - [ ] 式（四則演算）に型を持たせる
  - [ ] 関数に型を持たせる





