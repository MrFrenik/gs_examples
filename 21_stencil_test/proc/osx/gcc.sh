#!/bin/bash

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
	-std=c99 -x objective-c -O0 -w 
)

# Include directories
inc=(
	-I ../../third_party/include/
    -I ../source/
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

# Build
gcc ${flags[*]} ${fworks[*]} ${inc[*]} ${src[*]} -o ${proj_name}

cd ..



