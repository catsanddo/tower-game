#!/bin/sh

mkdir -p bin/include
mkdir -p bin/lib

cp src/cose.h src/cose.c
gcc -Wall -Wextra -c src/cose.c -O2 -o bin/cose.o
rm src/cose.c
ar -crs bin/lib/libcose.a bin/*.o
cp src/*.h bin/include
sed '/".*\.c"/d' src/cose.h > bin/include/cose.h
