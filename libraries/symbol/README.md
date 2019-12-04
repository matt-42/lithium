
li::symbol
=================================

This library implements the basics of symbol based
programming. Symbols are a new C++ paradigm that allow you to simply
implement introspection, serialization, named parameters, and other
things.

A symbol is defined with a macro function :

```c++
#ifndef LI_SYMBOL_my_symbol
#define LI_SYMBOL_my_symbol
  LI_SYMBOL(my_symbol)
#endif

#ifndef LI_SYMBOL_my_symbol2
#define LI_SYMBOL_my_symbol2
  LI_SYMBOL(my_symbol2)
#endif
```

A tool is provided to generate symbols automatically:
```
li_symbol_generator f1.cc f2.cc ... > symbol_declarations.hh
```

All symbols are placed in the s:: namespace and can be used with the followings :

```c++
// Named Variable declaration.
auto v = li::make_variable(s::my_symbol, 42);
assert(v.my_symbol == 42);

// Get the string associated to a symbol.
assert(!strcmp(li::symbol_string(s::my_symbol), "my_symbol"));

// Member access.
assert(li::symbol_member_access(v, s::my_symbol) == 42);  

// Method call
struct {
  int my_symbol(int a) { return x + a; }
  int x;
} obj{40};

assert(li::symbol_method_call(obj, s::my_symbol, 2) == 42);

// Introspection on objects.
assert(li::has_member(obj, s::my_symbol))
assert(!li::has_member(obj, s::my_symbol2))
```
