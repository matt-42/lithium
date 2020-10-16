
<p align="center"><img src="https://github.com/matt-42/lithium/raw/master/lithium_logo.png" alt="Lithium Logo. Designed by Yvan Darmet" title="The Lithium C++ libraries - Logo designed by Yvan Darmet" width=400 /></p>

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/matthieugarrigues)
![Travis](https://travis-ci.com/matt-42/lithium.svg?branch=master) ![platform](https://img.shields.io/badge/platform-Linux%20%7C%20MacOS-yellow) ![licence](https://img.shields.io/badge/licence-MIT-blue)

Lithium's goal is to ease the development of C++ high performance HTTP
APIs. As much effort is put into simplifying it's use as in optimizing its performances.
According to the [TechEmpower
benchmark](https://tfb-status.techempower.com), it is one of the
fastest web framework available. It is available on MacOS and Linux. Windows support is comming soon.

It is a set of independent header only C++ libraries, each one of them can be used
independently. Each library solves one problem and can be used
separately.

### [Installation/Getting Started](https://github.com/matt-42/lithium/tree/master/INSTALL.md)

# Libraries

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


# Supported compilers:
    - Linux: G++ 9.2, Clang++ 9.0
    - MacOS: Apple clang version 12.0.0 
    - Windows: Not supported

# Support the project

If you find this project helpful, give a star to lithium or buy me a coffee!
https://www.paypal.me/matthieugarrigues

# Project using Lithium

If you are using lithium and would like to name your project here, please submit a PR.

    - ffead-cpp
 
# Thanks

Big thanks to Yvan Darmet for the logo :) .
