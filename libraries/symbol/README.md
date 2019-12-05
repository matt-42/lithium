
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

Automatic symbol generation
===================

Declaring all symbols you are using is quite tedious.

The `li_symbol_generator` tool is provided to generate symbols automatically. 
To install it, build and install the lithum project.

```
$ li_symbol_generator
=================== Lithium symbol generator ===================

Usage: li_symbol_generator input_cpp_file1, ..., input_cpp_fileN
   Output on stdout the definitions of all the symbols used in the input files.

Usage: li_symbol_generator project_root
   For each folder under project root write a symbols.hh file containing the
   declarations of all symbols used in C++ source and header of this same directory.
```

You can also include a cmake rule to build all symbols.hh files automatically:

```cmake
add_custom_target(
    symbols_generation
    COMMAND li_symbol_generator ${CMAKE_CURRENT_SOURCE_DIR}/_root_or_your_code_with_symbols_)

# Now, you can use the li_add_executable function instead of add_executable:
function(li_add_executable target_name)
  add_executable(${target_name} ${ARGN})
  add_dependencies(${target_name} symbols_generation)
endfunction(li_add_executable)

# Or manually add the symbols_generation dependency to your target. 
add_dependencies(_your_target_name_ symbols_generation)
```

All you need now is to `#include "symbols.hh"` in your C++ code.
