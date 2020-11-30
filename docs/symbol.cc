
// __documentation_starts_here__

/*
symbol
=================================

## Introduction

This library implements the basics of symbol based
programming. Symbols allow you to simply
implement introspection, serialization, named parameters, and other
things.

A symbol is defined with a macro function :

*/
#ifndef LI_SYMBOL_my_symbol
#define LI_SYMBOL_my_symbol
  LI_SYMBOL(my_symbol)
#endif

#ifndef LI_SYMBOL_my_symbol2
#define LI_SYMBOL_my_symbol2
  LI_SYMBOL(my_symbol2)
#endif
/*
All symbols are placed in the `s::` namespace and offer the following features :
*/

// Named Variable declaration.
auto v = li::make_variable(s::my_symbol, 42);
assert(v.my_symbol == 42);

// Get the string associated to a symbol.
assert(!strcmp(li::symbol_string(s::my_symbol), "my_symbol"));

struct {
  int my_symbol(int a) { return x + a; }
  int x;
} obj{40};

// Member access.
assert(li::symbol_member_access(obj, s::x) == 40);  

// Method call
assert(li::symbol_method_call(obj, s::my_symbol, 2) == 42);

// Introspection on objects.
assert(li::has_member(obj, s::my_symbol))
assert(!li::has_member(obj, s::my_symbol2))
/*

## Automatic symbol generation

Manually defining symbols is a bit cumbersome, so we implemented a tool that can generate them automatically.
Check [here](/getting-started/automatic-symbols-generation) for more info.

*/