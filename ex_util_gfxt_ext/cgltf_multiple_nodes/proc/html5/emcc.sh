#!bin/sh

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
    -O3 -w -s WASM=1 -s USE_WEBGL2=1 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 --preload-file ../assets
)

# Include directories
inc=(
    -I ../../../third_party/include/   # Gunslinger includes
    -I ../external/                 # External includes
)

# Source files
src=(
    ../source/main.c
)

libs=(
)

# Build
emcc ${inc[*]} ${src[*]} ${flags[*]} -o $proj_name.html

cd ..



