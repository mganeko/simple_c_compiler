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
$ docker run --rm -it -v `pwd`/1cc:/1cc compilerbook
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
