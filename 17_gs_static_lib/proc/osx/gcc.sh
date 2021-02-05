#!/bin/bash

rm -rf lib
mkdir lib
cd lib

proj_name=gs
proj_root_dir=$(pwd)/../

flags=(
	-std=c99 -x objective-c -O0 -w 
)

# Include directories
inc=(
	-I ../../third_party/include/
)

# Source files
src=(
	../source/impl.c
)

fworks=(
	-framework OpenGL
	-framework CoreFoundation 
	-framework CoreVideo 
	-framework IOKit 
	-framework Cocoa 
	-framework Carbon
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
    -std=c99
)

# Include directories
inc=(
    -I ../../third_party/include/
)

# Source files
src=(
    ../source/main.c
)

fworks=(
    -framework OpenGL
    -framework CoreFoundation 
    -framework CoreVideo 
    -framework IOKit 
    -framework Cocoa 
    -framework Carbon
)

libs=(
    -lgs
)

lib_dirs=(
    -L ../lib/
)

# Build
gcc -O3 ${lib_dirs[*]} ${libs[*]} ${fworks[*]} ${inc[*]} ${src[*]} ${flags[*]} -o App

cd ..







