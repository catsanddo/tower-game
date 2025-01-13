#!/bin/bash

INC_PATH="-I../include"
LIBS="-lSDL2 -lSDL2_image -lSDL2_ttf -lm"
LIB_PATH="-L../lib"

mkdir -p bin

cd bin
# cl //Zi //I'..\include' '..\src\tower.c' //Fe:tower.exe //link //LIBPATH:'..\lib' //SUBSYSTEM:CONSOLE SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib Shell32.lib
gcc -g -I../include ../src/tower.c -o tower $LIBS
# zig cc -g $INC_PATH $LIB_PATH ../src/tower.c $LIBS -o tower.exe
cp -r ../res .
# cp ../lib/*.dll .
cd ..
