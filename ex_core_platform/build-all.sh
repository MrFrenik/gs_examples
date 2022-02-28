declare -A PLATFORM
# maps OS string to folder name
PLATFORM["msys"]="win"
PLATFORM["darwin"]="osx"
PLATFORM["linux-gnu"]="linux"
for d in */ ; do
    echo $d
    cd $d
    bash build-all.sh
    cd ..
done
