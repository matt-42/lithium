#! /bin/bash


docker run -it -v $HOME/lithium2:/lithium lithium_linux_test /bin/bash -c "
  git clone --recurse-submodules /lithium /tmp/lithium && cd /tmp/lithium &&
  /lithium/test/linux/setup_databases.sh && 
  ./vcpkg/bootstrap-vcpkg.sh &&
  ./vcpkg/vcpkg install &&
  cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_TOOLCHAIN_FILE=/tmp/lithium/vcpkg/scripts/buildsystems/vcpkg.cmake . && 
  make -j12 && ctest"
