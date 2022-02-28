#!/bin/bash

rm -rf lib
mkdir lib
cd lib

proj_name=gs
proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -Wl,--no-as-needed -ldl -lGL -lX11 -pthread -lXi -D_POSIX_C_SOURCE=199309L
)

# Include directories
inc=(
    -I ../../third_party/include/
)

# Source files
src=(
    ../source/impl.c
)

# Build Library
gcc -c -O3 ${flags[*]} ${inc[*]} ${src[*]} ${libs[*]}
ar -rvs lib${proj_name}.a *o 
ranlib lib${proj_name}.a
rm *.o

cd ..

# Build example

#!bin/sh

rm -rf bin
mkdir bin
cd bin

proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -Wl,--no-as-needed -ldl -lGL -lX11 -pthread -lXi -D_POSIX_C_SOURCE=199309L
)

# Include directories
inc=(
    -I ../../third_party/include/
)

# Source files
src=(
    ../source/main.c
)

lib_dirs=(
    -L ../lib/
)

libs=(
    -lgs
)

# Build
gcc -O3 ${lib_dirs[*]} ${libs[*]} ${inc[*]} ${src[*]} ${flags[*]} -lm -o App

cd ..
