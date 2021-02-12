for d in */ ; do
    echo $d
    cd $d
    bash proc/win/mingw.sh
    cd ..
done
