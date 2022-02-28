#!/bin/bash

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -Wl,--no-as-needed -ldl -lGL -lX11 -pthread -lXi
)

# Include directories
inc=(
	-I ../../../third_party/include/
    -I ../source/
)

# Source files
src=(
	../source/main.c
)

# Build
gcc -O3 ${inc[*]} ${src[*]} ${flags[*]} -lm -o ${proj_name}

cd ..
