#! /bin/bash

docker run -it -v $HOME/lithium:/lithium lithium_linux_test /bin/bash -c "
  /lithium/test/linux/setup_databases.sh && 
  git clone --recurse-submodules /lithium /tmp/lithium && cd /tmp/lithium &&
  ./vcpkg/bootstrap-vcpkg.sh &&
  ./vcpkg/vcpkg install &&
  cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_TOOLCHAIN_FILE=/tmp/lithium/vcpkg/scripts/buildsystems/vcpkg.cmake . && 
  make -j12 && ctest --output-on-failure"
