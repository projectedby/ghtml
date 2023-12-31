GHTML / GNU LIKE HTML COMPILER
==============================

__HTML is a programming language.__

이 프로젝트는 HTML 형식의 파일을 이용하여 여러 운영체제에서 실행할 수 있는 바이너리 파일을 컴파일할 수 있도록 하는 프로젝트입니다.

## 미리 설치되어야 하는 패키지

```sh
sudo apt install libxml2-dev
sudo apt install clang
sudo apt install llvm-dev
```

## 빌드방법

```sh
./configure
make
```

## HTML 파일에서 실행 파일을 만드는 스크립트

```sh
./src/ghtml simple.html
clang hello.bc -o hello
./hello
echo $?
```
