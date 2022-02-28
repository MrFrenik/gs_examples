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
	-I ../../../third_party/include/			# Gunslinger includes
    -I ../external/
)

# Source files
src=(
	../source/main.c ../external/flecs/flecs.c
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
gcc -O3 ${inc[*]} ${src[*]} ${flags[*]} ${libs[*]} -lm -o ${proj_name}

cd ..



