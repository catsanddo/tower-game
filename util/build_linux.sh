#!/bin/bash

INC_PATH="-I../../include"
LIBS="-lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lm"
LIB_PATH="-L../../lib"

mkdir -p util/bin

pushd util/bin
# cl //I'..\..\include' '..\map-switch.c'
# cl //I'..\..\include' '..\png2tga.c'
# cl //I'..\..\include' '..\dialogue_tree.c'
# cl //Zi //I'..\..\include' '..\tile-editor.c' //Fe:'..\..\bin\tiles.exe' //link //SUBSYSTEM:CONSOLE //LIBPATH:'..\..\lib' SDL2main.lib SDL2.lib SDL2_image.lib SDL2_ttf.lib Shell32.lib
# cl //Zi //I'..\..\include' '..\entity-editor.c' //Fe:'..\..\bin\entities.exe' //link //SUBSYSTEM:CONSOLE //LIBPATH:'..\..\lib' SDL2main.lib SDL2.lib SDL2_image.lib SDL2_ttf.lib Shell32.lib
gcc -I'../../include' '../map-switch.c' -o 'map-switch.c'
gcc -I'../../include' '../png2tga.c' -lm -o 'png2tga.c'
gcc -I'../../include' '../dialogue_tree.c' -o 'dialogue_tree.c'
# gcc -g '../tile-editor.c' -o '../../bin/tiles.exe' $INC_PATH $LIBS $LIB_PATH
# gcc -g '../entity-editor.c' -o '../../bin/entities.exe' $INC_PATH $LIBS $LIB_PATH
popd
