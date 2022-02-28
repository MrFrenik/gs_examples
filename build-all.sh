declare -A PLATFORM
# maps OS string to folder name
PLATFORM["msys"]="win"
PLATFORM["darwin"]="osx"
PLATFORM["linux-gnu"]="linux"
for sub_section in */ ; do
    echo $sub_section
    cd $sub_section
    for d in */ ; do
        echo $d
        cd $d
        bash proc/${PLATFORM["$OSTYPE"]}/*.sh
        cd ..
    done
    cd ..
done
