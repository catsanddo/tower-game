#!/bin/sh

gcc util/entity-gen.c -o util/entity-gen
./util/entity-gen
mv entity.dat res/entity.dat
