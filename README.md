The Lithium C++ Libraries ![Travis](https://travis-ci.com/matt-42/lithium.svg?branch=master) [![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/matthieugarrigues)
========================

Lithium's goal is to ease the development of C++ high performance HTTP
APIs. As much effort is put into simplifying it's use as in optimizing its performances.
According to the [TechEmpower
benchmark](https://tfb-status.techempower.com), it is one of the
fastest web framework available.

It is a set of independent header only C++ libraries, each one of them can be used
independently. Each library solves one problem and can be used
separately.

### [li::metamap](https://github.com/matt-42/lithium/tree/master/libraries/metamap)

A zero-cost compile time key/value map.

### [li::json](https://github.com/matt-42/lithium/tree/master/libraries/json)

A JSON serializer/deserializer designed for
ease of use and performances.

### [li::sql](https://github.com/matt-42/lithium/tree/master/libraries/sql)

A set of asynchronous and synchronous SQL drivers.

### [li::http_client](https://github.com/matt-42/lithium/tree/master/libraries/http_client)

A http_client built around the libcurl library.

### [li::http_backend](https://github.com/matt-42/lithium/tree/master/libraries/http_backend)

A coroutine based, asynchronous, low latency and high throughput HTTP Server library.

### [li::symbol](https://github.com/matt-42/lithium/tree/master/libraries/symbol)

The root of the lithium project. Used by all other libraries.
You probably will not use this library directly but
you can check the code if you want to know what is inside the s:: namespace.

# Need some help ?

Post a github issue if you need help with Lithium. And there are no dummy questions !

# Getting Started (or setting up automatic symbol generation)

The Lithium paradigm relies on compile time symbols (in the `s::` namespace) to bring introspection
into C++. If you use symbols, you can either declare them manually, or use `li_symbol_generator` to generate
these definitions automatically.

To get `li_symbol_generator`, you need to compile and install lithium locally:

```sh
git clone https://github.com/matt-42/lithium.git
cd lithium;
mkdir build; cd build;
# Global install:
cmake ..; make -j4 install;
# Local install (replace $HOME/local with your prefered location)
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/local; make -j4 install;
```

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
Lithium is header only. You can `#include` lithium headers and start coding. 

# Installation

https://github.com/matt-42/lithium/tree/master/INSTALL.md


# Supported compilers:
    - Linux: G++ 9.2, Clang++ 9.0
    - Macos, Windows: Not supported

# Support the project

If you find this project helpful, buy me a coffee!
https://www.paypal.me/matthieugarrigues
