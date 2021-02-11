#!bin/sh

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
	-w -s USE_WEBGL2=1 -s USE_GLFW=3 -s WASM=1
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
)

# Build
# emcc -O3 -s SINGLE_FILE -s WASM=0 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' ../source/main.c -o main.html
emcc ${inc[*]} ${src[*]} ${flags[*]} -o main.html
# emcc -O0 ${inc[*]} ${src[*]} ${flags[*]} ${libs[*]} -lm -o ${proj_name}

cd ..



