#/bin/bash

docker run -it -v $HOME/lithium:/lithium lithium_linux_test /bin/bash -c "
  /lithium/test/linux/setup_databases.sh && 
  mkdir -p /tmp/build && cd /tmp/build && 
  ls /lithium && 
  cmake -DCMAKE_CXX_COMPILER=clang++-9 /lithium && 
  make -j12 && ctest"
