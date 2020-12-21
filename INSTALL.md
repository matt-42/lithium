# Getting Started

There is two ways to get started with lithium:
  - Using the command line interface, which build and run programs inside docker containers.
  - By instaling lithium locally

## Using the command line interface

*Requirements*: Python 3 and Docker

The lithium cli allows you to build and run code without installing any dependencies and without writing any Makefile or CMakeFile.

Install the cli somewhere in your $PATH:
```
wget https://raw.githubusercontent.com/matt-42/lithium/master/cli/li 
```

Once installed, you can compile and run servers in one command:
```
$ li run [source files...] [program arguments]
```

When run the first time, `li run` will take 1-2 minutes to build the lithium docker image.

For more info about the cli:
```
$ li -h
$ li run -h
```

If this cli does not suit your workflow, please post a github issue.

## Install Lithium locally

```sh
git clone https://github.com/matt-42/lithium.git
cd lithium;

# Global install:
mkdir build && cd build && && cmake .. && make -j4 install;

# Local install (replace $HOME/local with your prefered location)
mkdir build && cd build && && cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/local && make -j4 install;
```

The Lithium paradigm relies on compile time symbols (in the `s::` namespace) to bring introspection
into C++. If you use symbols, you can either declare them manually, or use `li_symbol_generator` to generate
these definitions automatically.

To get `li_symbol_generator`, you need to compile and install lithium locally:


You should be able to run `li_symbol_generator` without arguments and get its help message:
```sh
$ li_symbol_generator
=================== Lithium symbol generator ===================

Usage: li_symbol_generator input_cpp_file1, ..., input_cpp_fileN
   Output on stdout the definitions of all the symbols used in the input files.

Usage: li_symbol_generator project_root
   For each folder under project root write a symbols.hh file containing the
   declarations of all symbols used in C++ source and header of this same directory.
```

If you use cmake, here is a custom target that will run the generator before each compilation.
```cmake
# the symbol generation target
add_custom_target(
    symbols_generation
    COMMAND li_symbol_generator ${CMAKE_CURRENT_SOURCE_DIR})

# Your executables
add_executable(your_executable your_main.cc)
# Link the symbol generation to your cmake target:
add_dependencies(your_executable symbols_generation)
```

Now that you have setup symbol generation (or choose manual declaration), you are done because 
Lithium is header only. You can  `#include` one of the lithium headers and start coding. Check
https://github.com/matt-42/lithium/tree/master/single_headers for the full list of available headers.

### CMake

The recommended way (without using the cli) is to use cmake to compile project using lithium.
Check the cmake+lithium project starter here:
https://github.com/matt-42/lithium/tree/master/cmake_project_template

## Note for projects linking multiple C++ files

The `sql` libraries use a global variable to store the connections. If you encounter
multiple definitions at link time, you need to `#define LI_EXTERN_GLOBALS` before including the lithium
header in all your files exept one.

In one file:
```
#include <lithium.hh> // or <lithium_XXXX.hh> 
```

And in all the others:
```
#define LI_EXTERN_GLOBALS
#include <lithium.hh> // or <lithium_XXXX.hh> 
```
