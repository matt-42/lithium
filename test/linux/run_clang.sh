#/bin/bash

docker run -it -v $HOME/lithium:/lithium lithium_linux_test /bin/bash -c "
  /lithium/test/linux/setup_databases.sh && 
  git clone /lithium /tmp/lithium && cd /tmp/lithium &&
  ./vcpkg/bootstrap-vcpkg.sh &&
  ./vcpkg/vcpkg install &&
  cmake -DCMAKE_CXX_COMPILER=clang++-9 . && 
  make -j12 && ctest"
