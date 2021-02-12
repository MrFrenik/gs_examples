if [[ "$OSTYPE" == "msys" ]]; then
  for d in */ ; do
      echo $d
      cd $d
      bash proc/win/*.sh
      cd ..
  done
elif [[ "$OSTYPE" == "darwin" ]]; then
  for d in */ ; do
      echo $d
      cd $d
      bash proc/osx/*.sh
      cd ..
  done
elif [[ "$OSTYPE" == "linux-gnu" ]]; then
  for d in */ ; do
      echo $d
      cd $d
      bash proc/linux/*.sh
      cd ..
  done
fi
