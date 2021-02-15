#! /bin/bash

if [ $# -eq 0 ]; then
  echo "Usage: install.sh INSTALL_PREFIX"
fi

realpath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

PREFIX=$(realpath $1)

DIR=`mktemp -d`
git clone https://github.com/matt-42/lithium.git $DIR
cd $DIR;



# Install dirs
mkdir -p $PREFIX/bin 
mkdir -p $PREFIX/include

# Compile li_symbol_generator
echo "Install $PREFIX/bin/li_symbol_generator"
g++ -O3 -DNDEBUG -std=c++17 $DIR/libraries/symbol/symbol/symbol_generator.cc -o $PREFIX/bin/li_symbol_generator

cd $DIR/single_headers
for header in `ls *.hh`; do
  echo "Install $PREFIX/include/$header"
  cp $DIR/single_headers/$header $PREFIX/include/$header 
done

echo "Cleaning up."
rm -fr $DIR
