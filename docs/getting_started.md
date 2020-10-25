# Getting Started

There is two ways to get started with lithium:
  - Using the command line interface, which build and run programs inside docker containers.
  - By instaling lithium locally

## Using the command line interface

*Requirements*: Python 3 and Docker

The lithium cli allows you to build and run code without installing any dependencies and without writing any Makefile or CMakeFile.

Install the cli somewhere in your $PATH:
```text
$ wget https://raw.githubusercontent.com/matt-42/lithium/master/cli/li 
```

Once installed, you can compile and run servers in one command:
```bash
$ li run [source files...] [program arguments]
```

When run the first time, `li run` will take 1-2 minutes to build the lithium docker image.

For more info about the cli:
```bash
$ li -h
$ li run -h
```

If this cli does not suit your workflow, please post a github issue.

## Install Lithium locally

To install the Lithium headers and the `li_symbol_generator` tool:

```bash
curl https://raw.githubusercontent.com/matt-42/lithium/master/install.sh | bash -s - INSTALL_PREFIX
```

## Automatic symbols generation

Note: Automatic symbols generation is included in the command line interface.

The Lithium paradigm relies on compile time symbols (in the `s::` namespace) to bring introspection
into C++. If you use symbols, you can either declare them manually...
```c++
#ifndef LI_SYMBOL_my_symbol
#define LI_SYMBOL_my_symbol
  LI_SYMBOL(my_symbol)
#endif
```
... or use `li_symbol_generator` to generate these definitions automatically.

You should be able to run `li_symbol_generator` without arguments and get its help message:
```text
$ li_symbol_generator
=================== Lithium symbol generator ===================

Usage: ./build/libraries/symbol/li_symbol_generator input_cpp_file1, ..., input_cpp_fileN
   Output on stdout the definitions of all the symbols used in the input files.

Usage: ./build/libraries/symbol/li_symbol_generator dir1, dir2, ...
   For all dirN folder and dirN subfolders write a symbols.hh file containing the
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

## CMake

The recommended way (without using the cli) is to use cmake to compile project using lithium.
Check the cmake+lithium project starter here:
https://github.com/matt-42/lithium/tree/master/cmake_project_template

## Note for projects linking multiple C++ files

The `sql` libraries use a global variable to store the pool of connections. If you encounter
multiple definitions at link time, you need to `#define LI_EXTERN_GLOBALS` before including the lithium
header in all your files exept one.

In one file:
```c++
#include <lithium.hh> // or <lithium_XXXX.hh> 
```

And in all the others:
```c++
#define LI_EXTERN_GLOBALS
#include <lithium.hh> // or <lithium_XXXX.hh> 
```
