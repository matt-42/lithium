# Introduction

## Motivations

Pre-2010 C++ was a pretty low-level language and zero-runtime-cost abstractions where only possible
using complex template meta-programming tricks. The C++ standard committee addressed those issues by
publishing new C++ standard, making C++ programs much more simpler and fun to write.
These recent language evolutions has created inside the language a C++ subset that is easy to grasp 
for beginners and less error prone than C++ itself: using RAII instead of manual memory management, using
`constexpr` instead of template meta-programming, etc... 


Could this subset of C++ lead to a web framework that is as easy to grasp than
javascript alternatives while keeping the performances of C++ ?


The goal of `Lithium` is to prove that this is feasible.

The design of the libraries is focused on **simplicity**:
  - Simple things should be simple to write for the end user.
  - Inheritance is rarely used
  - The end user should not have to instantiate complex templates
  - Function taking many parameters should use named parameters

## Supported compilers
  - Linux: G++ 9.2, Clang++ 9.0
  - MacOS: Apple clang version 12.0.0 
  - Windows: Not supported. Waiting for a bugfix in the MSVC C++ compiler.

## Benchmarks

Lithium HTTP server and SQL drivers are benchmarked againts 200+ other framesworks at [TechEmpower](https://tfb-status.techempower.com/).
As of November 2020, it is ranked first.

## Issues / Feature requests

You are welcome to ask questions or to ask for feature requests by posting a [github issue](https://github.com/matt-42/lithium/issues).

## Support the project

If you find this project helpful and want to support it, give a github star to lithium or [buy me a coffee](https://github.com/sponsors/matt-42)
