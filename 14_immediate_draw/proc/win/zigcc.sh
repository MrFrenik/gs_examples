#!bin/sh

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -w
)

# Include directories
inc=(
	-I ../../third_party/include/			# Gunslinger includes
)

# Source files
src=(
	../source/main.c
)

libs=(
	-lopengl32
	-lkernel32 
	-luser32 
	-lshell32 
	-lgdi32 
    -lWinmm
	-lAdvapi32
)

# Build
zig cc -Wall -D__PRFCHWINTRIN_H -O1 ${inc[*]} ${src[*]} ${flags[*]} ${libs[*]} -o ${proj_name}.exe