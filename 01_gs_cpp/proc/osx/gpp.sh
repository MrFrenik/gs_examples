#!/bin/bash

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
	-std=c++11 -x objective-c++ -O0 -w 
)

# Include directories
inc=(
	-I ../../third_party/include/
)

# Source files
src=(
	../src/main.cpp
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
g++ ${flags[*]} ${fworks[*]} ${inc[*]} ${src[*]} -o ${proj_name}

cd ..



