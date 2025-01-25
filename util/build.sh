#!/bin/bash

INC_PATH="-I../../include"
LIBS="-lSDL2main -lSDL2 -lSDL2_image -lm"
LIB_PATH="-L../../lib"

mkdir -p util/bin

pushd util/bin
cl //I'..\..\include' '..\map-switch.c'
cl //I'..\..\include' '..\png2tga.c'
cl //I'..\..\include' '..\dialogue_tree.c'
# cl //Zi //I'..\..\include' '..\tile-editor.c' //Fe:'..\..\bin\tiles.exe' //link //SUBSYSTEM:CONSOLE //LIBPATH:'..\..\lib' SDL2main.lib SDL2.lib SDL2_image.lib Shell32.lib
# cl //Zi //I'..\..\include' '..\entity-editor.c' //Fe:'..\..\bin\entities.exe' //link //SUBSYSTEM:CONSOLE //LIBPATH:'..\..\lib' SDL2main.lib SDL2.lib SDL2_image.lib Shell32.lib
popd
