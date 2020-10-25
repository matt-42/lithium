# Introduction

## Motivations

Pre-2010 C++ was a pretty low-level language and zero-runtime-cost abstraction where only possible
using complex template meta-programming tricks. The C++ standard commitee addressed thoses issues by
publishing new C++ standard, making C++ programs much more simpler and fun to write.
This recent language evolutions has created inside the language a C++ subset that is easy to grasp 
for begginers and less error prone than C++ itself: using RAII insead of manual memory management, using
constexpr instead of template metaprogramming, etc... 

Could this subset of C++ lead to a web framework that is as easy to grasp than
javascript alternatives while keeping the performances of C++ ?

The goal of `Lithium` is to prove that this is feasible.

## Supported compilers
  - Linux: G++ 9.2, Clang++ 9.0
  - MacOS: Apple clang version 12.0.0 
  - Windows: Not supported. Waiting for a bugfix in the MSVC C++ compiler.
