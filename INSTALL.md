# Installation

Lithium is a set of headers only libraries.

# Single headers

Copy the compiled single header of the library you need and include it in your C++ code: https://github.com/matt-42/lithium/tree/master/single_headers:
  - lithium.hh: all libraries
  - lithium_XXXX.hh: library XXXX only.

# Note for projects linking multiple C++ files

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

# Automatic symbol generation

The `li_symbol_generator` tool need to be compiled: 

```c++
git clone https://github.com/matt-42/lithium.git
cd lithium;
mkdir build; cd build;
cmake .. -DCMAKE_INSTALL_PREFIX=_your_prefix_; make -j4 install;
```

The `li_symbol_generator` will be install in `_your_prefix_/bin`

Check https://github.com/matt-42/lithium/tree/master/libraries/symbol for more information on how to use automatic symbol generation.


# Getting started with CMake

Check the cmake+lithium project starter here:
https://github.com/matt-42/lithium/tree/master/cmake_project_template
