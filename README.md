The Lithium C++ Libraries
========================

Lithium provides a set of free C++17 multi-platform libraries including:

### [li::metamap](https://github.com/matt-42/lithium/tree/master/libraries/metamap)

A zero-cost compile time key/value map.

### [li::json](https://github.com/matt-42/lithium/tree/master/libraries/json)

A JSON serializer/deserializer designed for
ease of use and performances.

### [li::sql](https://github.com/matt-42/lithium/tree/master/libraries/sql)

A set of tools to interact with SQL databases.

### [li::http_client](https://github.com/matt-42/lithium/tree/master/libraries/http_client)

An http_client built around the libcurl library.

### [li::http_backend](https://github.com/matt-42/lithium/tree/master/libraries/http_backend)

A library that ease the writing of web HTTP APIs.

### [li::symbol](https://github.com/matt-42/lithium/tree/master/libraries/symbol)

The root of the lithium project. Used by all other libraries.
You probably will not use this library directly but
you can check the code if you want to know what is inside the s:: namespace.


## Installation / Supported compilers


You can either use the single header version of the libraries:
https://github.com/matt-42/lithium/tree/master/single_headers

Or install all the libraries at once:

```c++
git clone https://github.com/matt-42/lithium.git
cd lithium;
mkdir build; cd build;
cmake .. -DCMAKE_INSTALL_PREFIX=_your_prefix_; make -j4 install;
```

**Important note:** In the single header versions, the namespace `li` is renamed as `li_{library_name}`.

### Supported compilers:
    - Linux: G++ 9.2, Clang++ 9.0
    - Macos: Clang 11
    - Windows: MSVC 19

## Donate

If you find this project helpful, please consider donating:
https://www.paypal.me/matthieugarrigues
