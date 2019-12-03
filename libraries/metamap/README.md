li::metamap
===============================

```li::metamap``` is an immutable zero-cost key value map. All
operations on metamaps are run by the compiler and have a O(1)
runtime cost. This greatly helps to build high performance
applications while keeping the flexibility of maps.
Compile time has also been reduced thanks to a zero-compile-time cost
key retrieve and the heavy use of parameter pack expansion.

Note: This is a work in progress.


Dependencies
==============

[li::symbol](https://github.com/iodcpp/symbol)


Tutorial
==============

Let's first define some [symbols](https://github.com/iodcpp/symbol). They will be
used as map keys.

```c++
LI_SYMBOL(a)
LI_SYMBOL(b)
```

A map is a set of key value pairs:

```c++
// Create a map
auto m = li::metamap(s::a = 1, s::b = 2);

// Retrieve map values via direct member access.
// Zero cost neither at runtime nor compile time.
assert(m.a == 1);
// Or via operator[].
assert(m[s::a] == 1);
```

Concatenation of two maps. Values of m1 are given the priority in case of dupplicate keys.

```c++
auto m3 = li::cat(m1, m2);
```

Build the map containing keys present in m1 and m2, taking values from m1.

```c++
auto m4 = li::intersection(m1, m2);
```

Build the map containing keys present in m1 but not in m2, taking values from m1.

```c++
auto m5 = li::substract(m1, m2);
```

Map a function on all key value pairs:

```c++
li::map(m1, [] (auto k, auto v) { std::cout << li::symbol_string(k) << "=" << v << std::endl; });
```
