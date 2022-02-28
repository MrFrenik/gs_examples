#!/bin/bash

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
	-std=c++11 -Wl,--no-as-needed -ldl -lGL -lX11 -pthread -lXi
)

# Include directories
inc=(
	-I ../../../third_party/include/ -I ../external/
)

# Source files
src=(
	../src/*.cpp
)

# Build
g++ -O3 ${inc[*]} ${src[*]} ${flags[*]} -lm -o ${proj_name}

cd ..
