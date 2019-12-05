The Lithium C++ Libraries
========================

Lithium provides a set of free C++17 multi-platform libraries including:

### li::metamap

A zero-cost compile time key/value map.

### li::json

A JSON serializer/deserializer designed for
ease of use and performances.

### li::sql

A set of tools to interact with SQL databases.

### li::http_client

An http_client built around the libcurl library.

### li::http_backend

A library that ease the writing of web HTTP APIs.

### li::symbol

The root of the lithium project.


## Installation

You can either use the single header version of the libraries:
https://github.com/matt-42/lithium/tree/master/single_headers

Or install all the libraries at once:

```c++
git clone https://github.com/matt-42/lithium.git
cd lithium;
mkdir build; cd build;
cmake .. -DCMAKE_INSTALL_PREFIX=_your_prefix_; make -j4 install;
```

## Donate

If you find this project helpful, please consider donating:
https://www.paypal.me/matthieugarrigues
