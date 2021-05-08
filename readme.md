# Study Building C Compiler

## 参照
https://www.sigbus.info/compilerbook

## Dockerを使ったビルド環境

https://www.sigbus.info/compilerbook#docker

Docker Imageビルド

```
$ docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile
```

or 

```
$ docker build -t compilerbook .
```

Dockerfileで、明示的にamd64を指定



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
- [ ] function local variable
  - [ ] locals を引数にして、関数内では改めて生成する
- [ ] function args as local variable








