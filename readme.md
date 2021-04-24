# Study Building C Compiler

## 参照
https://www.sigbus.info/compilerbook

## Dockerを使ったビルド環境

https://www.sigbus.info/compilerbook#docker

Docker Imageビルド

```
$ docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile
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
$ docker run --rm -it -v `pwd`/1cc:/1cc compilerbook
```

alias cbookit



