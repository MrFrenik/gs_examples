#!bin/sh

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
    -std=c++11 -x -O0 -w 
)

# Include directories
inc=(
    -I ../../../third_party/include/           # Gunslinger includes
    -I ../external/
)

# Source files
src=(
    ../src/*.cpp
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
g++ -O0 ${inc[*]} ${src[*]} ${flags[*]} ${libs[*]} -lm -o ${proj_name}

cd ..



